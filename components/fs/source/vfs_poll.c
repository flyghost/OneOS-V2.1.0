/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        vfs_poll.c
 *
 * @brief       This file implements the general poll operation.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <sys/errno.h>
#include <string.h>
#include <poll.h>
#include <os_assert.h>
#include <os_clock.h>
#include <os_sem.h>
#include <vfs_poll.h>
#include "vfs_private.h"

static int vfs_poll_op_init(struct vfs_pollfd *poll_fd)
{
    int ret;
    struct vfs_file *fp;
    os_uint32_t revents;

    ret = 0;
    fp = fd_to_fp(poll_fd->fd);
    if (fp)
    {
        if (FT_DEVICE == fp->type)
        {
            if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->poll)
            {
                ret = fp->mnt_point->ops->poll(fp, poll_fd, OS_TRUE);
            }
            else
            {
                ret = -ENOSYS;
            }
        }
        else if (FT_REGULAR == fp->type)
        {
            /* For regular file, always readable and writeble. */
            revents = (poll_fd->events & (POLLIN | POLLOUT));
            if (revents != 0)
            {
                vfs_poll_notify(poll_fd, revents);
            }
        }
        else
        {
            ret = -EINVAL;
        }
    }
    else
    {
        ret = -EINVAL;
    }

    return ret;
}


static int vfs_poll_op_deinit(struct vfs_pollfd *poll_fd)
{
    int ret;
    struct vfs_file *fp;

    ret = 0;
    if (poll_fd->fd >= 0)
    {
        fp = fd_to_fp(poll_fd->fd);
        if (fp && (FT_DEVICE == fp->type))
        {
            if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->poll)
            {
                ret = fp->mnt_point->ops->poll(fp, poll_fd, OS_FALSE);
            }
            else
            {
                ret = -ENOSYS;
            }
        }
    }
    else
    {
        ret = -EBADF;
    }

    return ret;
}

static int vfs_poll_init(struct vfs_pollfd *fds, unsigned int nfds, os_sem_t *sem)
{
    os_uint32_t i;
    os_uint32_t j;
    os_uint32_t poll_cnt;
    int ret;

    ret = 0;
    poll_cnt = 0;
    for (i = 0; i < nfds; i++)
    {
        fds[i].sem = sem;
        fds[i].revents = 0;

        if ((fds[i].fd >= VFS_FD_OFFSET) && fds[i].events)
        {
            poll_cnt++;
            ret = vfs_poll_op_init(&fds[i]);
        }

        if (ret < 0)
        {
            for (j = 0; j < i; j++)
            {
                if ((fds[j].fd >= VFS_FD_OFFSET) && fds[j].events)
                {
                    vfs_poll_op_deinit(&fds[j]);
                }
            }
            break;
        }
    }

    if (0 == poll_cnt)
    {
        ret = -EINVAL;
    }

    return ret;
}

static int vfs_poll_deinit(struct vfs_pollfd *fds, unsigned int nfds, unsigned int *avalible_nfd)
{
    int result;
    int ret;
    os_uint32_t i;

    ret = 0;
    for (i = 0; i < nfds; i++)
    {
        if ((fds[i].fd >= VFS_FD_OFFSET) && fds[i].events)
        {
            result = vfs_poll_op_deinit(&fds[i]);
            if (result >= 0)
            {
                if (0 != fds[i].revents)
                {
                  (*avalible_nfd)++;
                }
            }
            else
            {
                ret = result;
            }
        }

        fds[i].sem = NULL;
    }

    return ret;
}

int vfs_poll(struct vfs_pollfd *fds, unsigned int nfds, int timeout)
{
    int ret;
    int poll_result;
    os_err_t wait_result;
    os_sem_t poll_sem;
    os_tick_t timeout_tick;
    os_uint32_t avalible_nfd;

    OS_ASSERT(fds);
    OS_ASSERT(nfds > 0);

    ret = -1;
    memset(&poll_sem, 0, sizeof(os_sem_t));
    os_sem_init(&poll_sem, OS_NULL, 0, nfds);

    poll_result = vfs_poll_init(fds, nfds, &poll_sem);
    if (poll_result >= 0)
    {
        if (timeout > 0)
        {
            timeout_tick = os_tick_from_ms(timeout);
        }
        else if (timeout == 0)
        {
            timeout_tick = OS_NO_WAIT;
        }
        else
        {
            timeout_tick = OS_WAIT_FOREVER;
        }
        wait_result = os_sem_wait(&poll_sem, timeout_tick);

        avalible_nfd = 0;
        poll_result = vfs_poll_deinit(fds, nfds, &avalible_nfd);

        if (OS_EOK == wait_result)
        {
            if (poll_result >= 0)
            {
                ret = avalible_nfd;
            }
            else
            {
                VFS_SET_ERRNO(poll_result);
                ret = -1;
            }
        }
        else if (OS_ETIMEOUT == wait_result)
        {
            ret = 0;
        }
        else
        {
            VFS_SET_ERRNO(-EBUSY);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(poll_result);
    }

    os_sem_deinit(&poll_sem);

    return ret;
}

os_err_t vfs_poll_notify(struct vfs_pollfd* poll_fd, os_uint16_t revents)
{
    os_err_t ret;

    OS_ASSERT(poll_fd);
    OS_ASSERT(poll_fd->sem);
    OS_ASSERT(revents != 0);

    poll_fd->revents = revents;
    ret = os_sem_post(poll_fd->sem);

    return ret;
}


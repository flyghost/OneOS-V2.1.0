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
 * @file        vfs_poll.h
 *
 * @brief       Header file for poll operation.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_POLL_H__
#define __VFS_POLL_H__

#include <oneos_config.h>
#include <poll.h>
#include <os_types.h>
#include <os_list.h>
#include <os_sem.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vfs_pollfd
{
    int             fd;      /* File descriptor */
    os_uint16_t     events;  /* Requested events */
    os_uint16_t     revents; /* Returned events */

    os_sem_t       *sem;     /* Semaphore used to post returned events */
    os_list_node_t  node;    /* Used by dirver. */
};

#ifdef OS_USING_IO_MULTIPLEXING
extern int vfs_poll(struct vfs_pollfd *fds, unsigned int nfds, int timeout);
extern os_err_t vfs_poll_notify(struct vfs_pollfd *poll_fd, os_uint16_t revents);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VFS_POLL_H__ */


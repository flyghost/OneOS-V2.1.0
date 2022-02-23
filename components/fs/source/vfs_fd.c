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
 * @file        vfs_fd.c
 *
 * @brief       This file management the file descriptor.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_memory.h>
#include <os_assert.h>
#include <dlog.h>
#include <vfs.h>
#include "vfs_private.h"
#include <os_mutex.h>

#define FD_TO_FDTABID(fd)       (fd - VFS_FD_OFFSET)
#define FDTABID_TO_FD(id)       (id + VFS_FD_OFFSET)

struct fd_entry   {
    unsigned short  ref_cnt;    /* Descriptor reference count */
    struct vfs_file file;
};

static struct fd_entry fdtable[VFS_FD_MAX];
os_mutex_t vfs_lock;

int fd_alloc(void)
{
    int id;
    int fd;

    fd = -1;

    VFS_LOCK();

    for (id = 0; id < VFS_FD_MAX; id++)
    {
        if (fdtable[id].ref_cnt == 0)
        {
            fdtable[id].ref_cnt++;
            break;
        }
    }

    VFS_UNLOCK();

    if (id < VFS_FD_MAX)
    {
        fd = FDTABID_TO_FD(id);
    }
    else
    {
        LOG_E(VFS_TAG, "fd table is not enough, you may increase VFS_FD_MAX");
    }

    return fd;
}

void fd_ref_inc(int fd)
{
    int id;

    id = FD_TO_FDTABID(fd);
    OS_ASSERT((id >= 0) && (id < VFS_FD_MAX));

    VFS_LOCK();

    fdtable[id].ref_cnt++;

    VFS_UNLOCK();
}

void fd_ref_dec(int fd)
{
    int id;

    id = FD_TO_FDTABID(fd);
    OS_ASSERT((id >= 0) && (id < VFS_FD_MAX));

    VFS_LOCK();

    OS_ASSERT(fdtable[id].ref_cnt > 0);
    fdtable[id].ref_cnt--;
    if (0 == fdtable[id].ref_cnt)
    {
        if (fdtable[id].file.path)
        {
            os_free((void *)fdtable[id].file.path);
        }
        memset(&fdtable[id].file, 0, sizeof(struct vfs_file));
    }

    VFS_UNLOCK();
}

void fd_free(int fd)
{
    fd_ref_dec(fd);
}

struct vfs_file *fd_to_fp(int fd)
{
    int id;
    struct vfs_file *fp;

    fp = OS_NULL;

    id = FD_TO_FDTABID(fd);
    if ((id >= 0) && (id < VFS_FD_MAX))
    {
        VFS_LOCK();

        if (fdtable[id].ref_cnt > 0)
        {
            fp = &fdtable[id].file;
        }
        else
        {
            LOG_E(VFS_TAG, "fd:%d not use now.", fd);
        }

        VFS_UNLOCK();
    }
    else
    {
        LOG_E(VFS_TAG, "fd:%d is invalid.", fd);
    }

    return fp;
}

struct vfs_dir *dp_alloc(void)
{
    int id;
    struct vfs_dir *dp;

    dp = os_malloc(sizeof(struct vfs_dir));
    if (dp)
    {
        memset(dp, 0, sizeof(struct vfs_dir));

        VFS_LOCK();

        for (id = 0; id < VFS_FD_MAX; id++)
        {
            if (0 == fdtable[id].ref_cnt)
            {
                fdtable[id].ref_cnt++;
                dp->fp = &fdtable[id].file;
                break;
            }
        }

        VFS_UNLOCK();

        if (!dp->fp)
        {
            os_free(dp);
            dp = OS_NULL;
            LOG_E(VFS_TAG, "fd table is not enough, you may increase VFS_FD_MAX");
        }
    }

    return dp;
}

void dp_ref_inc(struct vfs_dir *dp)
{
    struct fd_entry *fd_entry;

    fd_entry = os_container_of(dp->fp, struct fd_entry, file);

    VFS_LOCK();
    fd_entry->ref_cnt++;
    VFS_UNLOCK();
}

void dp_ref_dec(struct vfs_dir *dp)
{
    struct fd_entry *fd_entry;

    fd_entry = os_container_of(dp->fp, struct fd_entry, file);

    VFS_LOCK();

    OS_ASSERT(fd_entry->ref_cnt > 0);
    fd_entry->ref_cnt--;
    if (0 == fd_entry->ref_cnt)
    {
        if (dp->fp->path)
        {
            os_free((void *)dp->fp->path);
        }
        memset(dp->fp, 0, sizeof(struct vfs_file));
        os_free(dp);
    }

    VFS_UNLOCK();
}

void dp_free(struct vfs_dir *dp)
{
    dp_ref_dec(dp);
}

os_bool_t dp_check_valid(struct vfs_dir *dp)
{
    struct fd_entry *fd_entry;
    os_bool_t ret;

    ret = OS_FALSE;

    if (dp)
    {
        if (dp->fp)
        {
            fd_entry = os_container_of(dp->fp, struct fd_entry, file);
            if ((fd_entry >= &fdtable[0]) && (fd_entry <= &fdtable[VFS_FD_MAX - 1]))
            {
                VFS_LOCK();

                if (fd_entry->ref_cnt > 0)
                {
                    ret = OS_TRUE;
                }

                VFS_UNLOCK();
            }
        }

        if (OS_FALSE == ret)
        {
            LOG_E(VFS_TAG, "Invalid directory stream");
        }
    }
    else
    {
        LOG_E(VFS_TAG, "directory stream is NULL pointer?");
    }

    return ret;
}

void fd_table_init(void)
{
    memset(fdtable, 0, sizeof(fdtable));
}

#ifdef OS_USING_SHELL
#include <shell.h>

static char *fd_type_str[FT_TYPEMAX] = {
    "INVALID",
    "FILE   ",
    "DIR    ",
    "DEV    ",
};

os_err_t sh_fdshow(os_int32_t argc, char **argv)
{
    unsigned int i;
    struct vfs_file *fp;
    os_err_t result;

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    result = OS_EOK;

    os_kprintf("  fd    type    mnt_path    path\r\n");
    os_kprintf("  --   ------   --------    ------\r\n");

    VFS_LOCK();

    for (i = 0; i < VFS_FD_MAX; i++)
    {
        fp = &fdtable[i].file;
        if (fp->mnt_point)
        {
            if(fp->type < FT_TYPEMAX)
            {
                os_kprintf("  %2d    %s    %s    %s\r\n", 
                       FDTABID_TO_FD(i), fd_type_str[fp->type], fp->mnt_point->mnt_path, fp->path);
            }
            else
            {
                os_kprintf("  %2d    %s    %s    %s\r\n", 
                       FDTABID_TO_FD(i), "ERROR ?", fp->mnt_point->mnt_path, fp->path);
                result = OS_ERROR;
            }
        }
    }

    VFS_UNLOCK();

    return result;
}
SH_CMD_EXPORT(show_fd, sh_fdshow, "show fd information");
#endif


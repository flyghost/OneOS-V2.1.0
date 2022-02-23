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
 * @file        vfs_posix.c
 *
 * @brief       This file implements file or direcotory function compliance with POSIX standard.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_util.h>
#include <os_task.h>
#include <os_memory.h>
#include <fcntl.h>
#include <vfs.h>
#include <sys/errno.h>
#include "vfs_private.h"
#include <vfs_posix.h>

int vfs_open(const char *path, int flags, ...)
{
    int   fd;
    int   ret;
    char *abs_path;
    struct vfs_file *fp;

    fd = -1;
    fp = OS_NULL;

    if (path)
    {
        fd = fd_alloc();
        if (fd >= 0)
        {
            fp = fd_to_fp(fd);
            if (!fp)
            {
                fd_free(fd);
                VFS_SET_ERRNO(-EBADF);
                fd = -1;
            }
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
            fd = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    if (fd >= 0)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            fd_ref_inc(fd);
            ret = do_openfile(fp, abs_path, flags);
            fd_ref_dec(fd);

            if (ret < 0)
            {
                fd_free(fd);
                VFS_SET_ERRNO(ret);
                fd = -1;
            }
            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            fd_free(fd);
            VFS_SET_ERRNO(-ENOMEM);
            fd = -1;
        }
    }

    return fd;
}

int vfs_close(int fd)
{
    int ret;
    struct vfs_file *fp;

    ret = -1;

    fp = fd_to_fp(fd);
    if (fp)
    {
        fd_ref_inc(fd);
        ret = do_closefile(fp);
        fd_ref_dec(fd);

        /* No matter close success or fail, both free fd. */
        fd_free(fd);
        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
        }
        ret = (ret < 0) ? (-1) : 0;
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

int vfs_read(int fd, void *buf, size_t len)
{
    int ret;
    struct vfs_file *fp;

    ret = -1;

    fp = fd_to_fp(fd);
    if (fp)
    {
        fd_ref_inc(fd);
        ret = do_readfile(fp, buf, len);
        fd_ref_dec(fd);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

int vfs_write(int fd, const void *buf, size_t len)
{
    int ret;
    struct vfs_file *fp;

    ret = -1;

    fp = fd_to_fp(fd);
    if (fp)
    {
        fd_ref_inc(fd);
        ret = do_writefile(fp, buf, len);
        fd_ref_dec(fd);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

off_t vfs_lseek(int fd, off_t offset, int whence)
{
    int ret;
    struct vfs_file *fp;

    ret = -1;

    fp = fd_to_fp(fd);
    if (fp)
    {
        fd_ref_inc(fd);
        ret = do_lseekfile(fp, offset, whence);
        fd_ref_dec(fd);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

int vfs_fsync(int fd)
{
    int ret;
    struct vfs_file *fp;

    ret = -1;

    fp = fd_to_fp(fd);
    if (fp)
    {
        fd_ref_inc(fd);
        ret = do_syncfile(fp);
        fd_ref_dec(fd);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
        }
        ret = (ret < 0) ? (-1) : 0;
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

int vfs_fcntl(int fd, int cmd, ...)
{
    int     ret;
    va_list ap;
    void   *arg;
    struct vfs_file *fp;

    ret = -1;

    fp = fd_to_fp(fd);
    if (fp)
    {
        va_start(ap, cmd);
        arg = va_arg(ap, void *);
        va_end(ap);

        fd_ref_inc(fd);
        ret = do_fcntl(fp, cmd, arg);
        fd_ref_dec(fd);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

int vfs_ioctl(int fd, unsigned long request, ...)
{
    int     ret;
    va_list ap;
    void   *arg;
    struct vfs_file *fp;

    ret = -1;

    fp = fd_to_fp(fd);
    if (fp)
    {
        va_start(ap, request);
        arg = va_arg(ap, void *);
        va_end(ap);

        fd_ref_inc(fd);
        ret = do_ioctl(fp, request, arg);
        fd_ref_dec(fd);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

int vfs_mkdir(const char *path, mode_t mode)
{
    int   ret;
    char *abs_path;
    struct vfs_dir *dp;

    ret = -1;

    if (path)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            dp = dp_alloc();
            if (dp)
            {
                dp_ref_inc(dp);
                ret = do_opendir(dp, abs_path, O_CREAT);
                if (ret >= 0)
                {
                    ret = do_closedir(dp);
                }
                dp_ref_dec(dp);

                dp_free(dp);
                if (ret < 0)
                {
                    VFS_SET_ERRNO(ret);
                }
                ret = (ret < 0) ? (-1) : 0;
            }
            else
            {
                VFS_SET_ERRNO(-ENOMEM);
            }
            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}

int vfs_rmdir(const char *path)
{
    int   ret;
    char *abs_path;

    ret = -1;

    if (path)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            ret = do_unlink(abs_path);

            if (ret < 0)
            {
                VFS_SET_ERRNO(ret);
            }
            ret = (ret < 0) ? (-1) : 0;
            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}

FS_DIR *vfs_opendir(const char *path)
{
    int   ret;
    char *abs_path;
    struct vfs_dir *dp;

    dp = OS_NULL;

    if (path)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            dp = dp_alloc();
            if (dp)
            {
                dp_ref_inc(dp);
                ret = do_opendir(dp, abs_path, 0);
                dp_ref_dec(dp);

                if (ret < 0)
                {
                    dp_free(dp);
                    dp = OS_NULL;
                    VFS_SET_ERRNO(ret);
                }
            }
            else
            {
                VFS_SET_ERRNO(-ENOMEM);
            }
            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return dp;
}

int vfs_closedir(FS_DIR *dp)
{
    int ret;

    ret = -1;

    if (OS_TRUE == dp_check_valid(dp))
    {
        dp_ref_inc(dp);
        ret = do_closedir(dp);
        dp_ref_dec(dp);

        /* No matter close success or fail, both free dp. */
        dp_free(dp);
        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
        }
        ret = (ret < 0) ? (-1) : 0;
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

/*
    In POSIX.1-2008, readdir is not thread-safe. In the GNU C Library implementation, it is safe to call
    readdir concurrently on different dirstreams, but multiple threads accessing the same dirstream 
    result in undefined behavior. readdir_r is a fully thread-safe alternative, but suffers from poor 
    portability. It is recommended that you use readdir, with externallocking if multiple 
    threads access the same dirstream.
*/
struct dirent *vfs_readdir(FS_DIR *dp)
{
    int ret;
    struct dirent *dentry;

    dentry = OS_NULL;

    if (OS_TRUE == dp_check_valid(dp))
    {
        dp_ref_inc(dp);
        ret = do_readdir(dp, &dp->entry);
        dp_ref_dec(dp);

        if (ret > 0)
        {
            dentry = &dp->entry;
        }
        else if (ret == 0)
        {
            /* end of the directory stream, NULL is returned and errno is not changed. */
        }
        else
        {
            VFS_SET_ERRNO(ret);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return dentry;
}

long vfs_telldir(FS_DIR *dp)
{
    int ret;

    ret = -1;

    if (OS_TRUE == dp_check_valid(dp))
    {
        dp_ref_inc(dp);
        ret = do_telldir(dp);
        dp_ref_dec(dp);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }

    return ret;
}

void vfs_seekdir(FS_DIR *dp, off_t offset)
{
    int ret;

    if (OS_TRUE == dp_check_valid(dp))
    {
        dp_ref_inc(dp);
        ret = do_seekdir(dp, offset);
        dp_ref_dec(dp);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }
}

void vfs_rewinddir(FS_DIR *dp)
{
    int ret;

    if (OS_TRUE == dp_check_valid(dp))
    {
        dp_ref_inc(dp);
        ret = do_seekdir(dp, 0);
        dp_ref_dec(dp);

        if (ret < 0)
        {
            VFS_SET_ERRNO(ret);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EBADF);
    }
}

int vfs_unlink(const char *path)
{
    int   ret;
    char *abs_path;

    ret = -1;

    if (path)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            ret = do_unlink(abs_path);

            if (ret < 0)
            {
                VFS_SET_ERRNO(ret);
            }
            ret = (ret < 0) ? (-1) : 0;
            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}

int vfs_rename(const char *oldpath, const char *newpath)
{
    int   ret;
    char *old_abs_path;
    char *new_abs_path;

    ret = -1;

    if (oldpath && newpath)
    {
        old_abs_path = vfs_create_absolute_path(OS_NULL, oldpath);
        new_abs_path = vfs_create_absolute_path(OS_NULL, newpath);

        if (old_abs_path && new_abs_path)
        {
            ret = do_rename(old_abs_path, new_abs_path);

            if (ret < 0)
            {
                VFS_SET_ERRNO(ret);
            }
            ret = (ret < 0) ? (-1) : 0;
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
        if (old_abs_path)
        {
            vfs_destroy_absolute_path(old_abs_path);
        }
        if (new_abs_path)
        {
            vfs_destroy_absolute_path(new_abs_path);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}


int vfs_stat(const char *path, struct stat *buf)
{
    int   ret;
    char *abs_path;

    ret = -1;

    if (path && buf)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            ret = do_stat(abs_path, buf);

            if (ret < 0)
            {
                VFS_SET_ERRNO(ret);
            }
            ret = (ret < 0) ? (-1) : 0;
            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}

int vfs_fstat(int fd, struct stat *buf)
{
    int ret;
    char *abs_path;
    struct vfs_file *fp;

    ret = -1;

    if (buf)
    {
        fp = fd_to_fp(fd);
        if (fp && fp->mnt_point)
        {
            abs_path = vfs_create_absolute_path(fp->mnt_point->mnt_path, fp->path);
            if (abs_path)
            {
                ret = do_stat(abs_path, buf);

                if (ret < 0)
                {
                    VFS_SET_ERRNO(ret);
                }
                ret = (ret < 0) ? (-1) : 0;
                vfs_destroy_absolute_path(abs_path);
            }
            else
            {
                VFS_SET_ERRNO(-ENOMEM);
            }
        }
        else
        {
            VFS_SET_ERRNO(-EBADF);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}

int vfs_chdir(const char *path)
{
    int     ret;
    char   *abs_path;
    struct vfs_dir *dp;

    ret = -1;

    if (path)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            dp = dp_alloc();
            if (dp)
            {
                dp_ref_inc(dp);
                ret = do_opendir(dp, abs_path, 0);
                if (ret >= 0)
                {
                    ret = do_closedir(dp);
                }
                dp_ref_dec(dp);

                dp_free(dp);

                if (ret >= 0)
                {
                    working_dir_set(abs_path);
                }
                else
                {
                    VFS_SET_ERRNO(ret);
                }

                ret = (ret < 0) ? (-1) : 0;
            }
            else
            {
                VFS_SET_ERRNO(-ENOMEM);
            }

            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}

char *vfs_getcwd(char *buf, size_t size)
{
    if (buf)
    {
        working_dir_get(buf, size);
    }

    return buf;
}

int vfs_access(const char *path, int amode)
{
    int ret;
    struct stat st;

    ret = vfs_stat(path, &st);

    return ret;
}

int vfs_statfs(const char *path, struct statfs *buf)
{
    int     ret;
    char   *abs_path;
    struct stat stat_buf;

    ret = -1;

    if (path && buf)
    {
        abs_path = vfs_create_absolute_path(OS_NULL, path);
        if (abs_path)
        {
            ret = do_stat(abs_path, &stat_buf);
            if (ret < 0)
            {
                VFS_SET_ERRNO(ret);
            }
            else
            {
                ret = do_statfs(abs_path, buf);
                if (ret < 0)
                {
                    VFS_SET_ERRNO(ret);
                }
            }

            ret = (ret < 0) ? (-1) : 0;
            vfs_destroy_absolute_path(abs_path);
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
    }

    return ret;
}


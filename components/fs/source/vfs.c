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
 * @file        vfs.c
 *
 * @brief       This file adapter for the vitural filesystem and actual filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_mutex.h>
#include <os_stddef.h>
#include <string.h>
#include <os_assert.h>
#include <device.h>
#include <os_memory.h>
#include <sys/errno.h>
#include <vfs.h>
#include "vfs_private.h"
#include <fcntl.h>

int do_openfile(struct vfs_file *fp, const char *path, int flags)
{
    struct vfs_mountpoint *mnt_point;
    const char *rel_mnt_path;
    int ret;

    ret = -EINVAL;

    if (fp && path)
    {
        mnt_point = vfs_mount_point_find_and_ref(path);

        if (mnt_point)
        {
            if ((mnt_point->ops) && (mnt_point->ops->open))
            {
                rel_mnt_path = vfs_create_rel_mnt_path(mnt_point->mnt_path, path);

                if (rel_mnt_path)
                {
                    fp->mnt_point = mnt_point;
                    fp->path = rel_mnt_path;
                    fp->flags = flags;
                    if (0 == strcmp(mnt_point->mnt_path, DEVFS_PATH))
                    {
                        fp->type = FT_DEVICE;
                    }
                    else
                    {
                        fp->type = FT_REGULAR;
                    }
                    ret = mnt_point->ops->open(fp);
                }
                else
                {
                    ret = -ENOMEM;
                }
            }
            else
            {
                ret = -ENOENT;
            }

            if (ret < 0)
            {
                vfs_mount_point_deref(mnt_point);
            }
        }
        else
        {
            ret = -ENOENT;
        }
    }

    return ret;
}

int do_closefile(struct vfs_file *fp)
{
    int ret;

    ret = -EINVAL;

    if (fp && (FT_DIRECTORY != fp->type))
    {
        if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->close)
        {
            ret = fp->mnt_point->ops->close(fp);

            /* No matter close success or fail, both dereference. */
            vfs_mount_point_deref(fp->mnt_point);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_readfile(struct vfs_file *fp, void *buf, size_t len)
{
    int ret;

    ret = -EINVAL;

    if (fp && buf && (FT_DIRECTORY != fp->type))
    {
        if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->read)
        {
            ret = fp->mnt_point->ops->read(fp, buf, len);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_writefile(struct vfs_file *fp, const void *buf, size_t len)
{
    int ret;

    ret = -EINVAL;

    if (fp && buf && (FT_DIRECTORY != fp->type))
    {
        if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->write)
        {
            ret = fp->mnt_point->ops->write(fp, buf, len);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_lseekfile(struct vfs_file *fp, off_t offset, int whence)
{
    int ret;

    ret = -EINVAL;

    if (fp && (FT_DIRECTORY != fp->type))
    {
        if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->lseek)
        {
            ret = fp->mnt_point->ops->lseek(fp, offset, whence);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_syncfile(struct vfs_file *fp)
{
    int ret;

    ret = -EINVAL;

    if (fp && (FT_DIRECTORY != fp->type))
    {
        if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->sync)
        {
            ret = fp->mnt_point->ops->sync(fp);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_fcntl(struct vfs_file *fp, int cmd, void *args)
{
    int ret;
    int flags;
    int mask;

    ret = 0;

    if (fp)
    {
        switch (cmd)
        {
        case F_GETFL:
            ret = fp->flags;
            break;

        case F_SETFL:
            flags = (int)args;
            mask  = O_NONBLOCK; /* Now only support set or clear O_NONBLOCK. */
            flags &= mask;
            fp->flags &= ~mask;
            fp->flags |= flags;
            break;

        default:
            ret = -ENOSYS;
            break;
        }
    }
    else
    {
        ret = -EINVAL;
    }

    return ret;
}

int do_ioctl(struct vfs_file *fp, unsigned long request, void *args)
{
    int ret;

    ret = -EINVAL;

    if (fp && (FT_DIRECTORY != fp->type))
    {
        if (fp->mnt_point && fp->mnt_point->ops && fp->mnt_point->ops->ioctl)
        {
            ret = fp->mnt_point->ops->ioctl(fp, request, args);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_opendir(struct vfs_dir *dp, const char *path, int flags)
{
    struct vfs_mountpoint *mnt_point;
    const char *rel_mnt_path;
    int ret;

    ret = -EINVAL;

    if (dp && dp->fp && path)
    {
        mnt_point = vfs_mount_point_find_and_ref(path);

        if (mnt_point)
        {
            if ((mnt_point->ops) && (mnt_point->ops->opendir))
            {
                rel_mnt_path = vfs_create_rel_mnt_path(mnt_point->mnt_path, path);

                if (rel_mnt_path)
                {
                    dp->fp->mnt_point = mnt_point;
                    dp->fp->path = rel_mnt_path;
                    dp->fp->flags = flags;
                    dp->fp->type = FT_DIRECTORY;
                    ret = mnt_point->ops->opendir(dp);
                }
                else
                {
                    ret = -ENOMEM;
                }
            }
            else
            {
                ret = -ENOENT;
            }

            if (ret < 0)
            {
                vfs_mount_point_deref(mnt_point);
            }
        }
        else
        {
            ret = -ENOENT;
        }
    }

    return ret;
}

int do_closedir(struct vfs_dir *dp)
{
    int ret;

    ret = -EINVAL;

    if (dp && dp->fp && (FT_DIRECTORY == dp->fp->type))
    {
        if (dp->fp->mnt_point && dp->fp->mnt_point->ops && dp->fp->mnt_point->ops->closedir)
        {
            ret = dp->fp->mnt_point->ops->closedir(dp);

            /* No matter close success or fail, both dereference. */
            vfs_mount_point_deref(dp->fp->mnt_point);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_readdir(struct vfs_dir *dp, struct dirent *entry)
{
    int ret;

    ret = -EINVAL;

    if (dp && entry && dp->fp && (FT_DIRECTORY == dp->fp->type))
    {
        if (dp->fp->mnt_point && dp->fp->mnt_point->ops && dp->fp->mnt_point->ops->readdir)
        {
            ret = dp->fp->mnt_point->ops->readdir(dp, entry);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}


int do_seekdir(struct vfs_dir *dp, off_t offset)
{
    int ret;

    ret = -EINVAL;

    if (dp && dp->fp && (FT_DIRECTORY == dp->fp->type))
    {
        if (dp->fp->mnt_point && dp->fp->mnt_point->ops && dp->fp->mnt_point->ops->seekdir)
        {
            ret = dp->fp->mnt_point->ops->seekdir(dp, offset);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

long do_telldir(struct vfs_dir *dp)
{
    int ret;

    ret = -EINVAL;

    if (dp && dp->fp && (FT_DIRECTORY == dp->fp->type))
    {
        if (dp->fp->mnt_point && dp->fp->mnt_point->ops && dp->fp->mnt_point->ops->telldir)
        {
            ret = dp->fp->mnt_point->ops->telldir(dp);
        }
        else
        {
            ret = -EBADF;
        }
    }

    return ret;
}

int do_rename(const char *oldpath, const char *newpath)
{
    struct vfs_mountpoint *old_mnt_point;
    struct vfs_mountpoint *new_mnt_point;
    const char *oldpath_rel_mnt;
    const char *newpath_rel_mnt;
    int ret;

    ret = -EINVAL;
    old_mnt_point = OS_NULL;
    new_mnt_point = OS_NULL;

    if (oldpath && newpath)
    {
        old_mnt_point = vfs_mount_point_find_and_ref(oldpath);
        new_mnt_point = vfs_mount_point_find_and_ref(newpath);

        if ((old_mnt_point) && (new_mnt_point == old_mnt_point))
        {
            ret = 0;
        }
        else
        {
            ret = -ENOENT;
        }
    }

    if (0 == ret)
    {
        if (old_mnt_point->ops && old_mnt_point->ops->rename)
        {
            oldpath_rel_mnt = vfs_get_rel_mnt_path(old_mnt_point->mnt_path, oldpath);
            newpath_rel_mnt = vfs_get_rel_mnt_path(new_mnt_point->mnt_path, newpath);
            if (oldpath_rel_mnt && newpath_rel_mnt)
            {
                ret = old_mnt_point->ops->rename(old_mnt_point, oldpath_rel_mnt, newpath_rel_mnt);
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
    }

    if (old_mnt_point)
    {
        vfs_mount_point_deref(old_mnt_point);
    }
    if (new_mnt_point)
    {
        vfs_mount_point_deref(new_mnt_point);
    }

    return ret;
}

int do_unlink(const char *path)
{
    struct vfs_mountpoint *mnt_point;
    const char *rel_mnt_path;
    int ret;

    ret = -EINVAL;

    if (path)
    {
        mnt_point = vfs_mount_point_find_and_ref(path);
        if (mnt_point)
        {
            if ((mnt_point->ops) && (mnt_point->ops->unlink))
            {
                rel_mnt_path = vfs_get_rel_mnt_path(mnt_point->mnt_path, path);
                if (rel_mnt_path)
                {
                    ret = mnt_point->ops->unlink(mnt_point, rel_mnt_path);
                }
                else
                {
                    ret = -EINVAL;
                }
            }
            else
            {
                ret = -ENOENT;
            }
            vfs_mount_point_deref(mnt_point);
        }
        else
        {
            ret = -ENOENT;
        }
    }

    return ret;
}

int do_stat(const char *path, struct stat *buf)
{
    struct vfs_mountpoint *mnt_point;
    const char *rel_mnt_path;
    int ret;

    ret = -EINVAL;

    if (path)
    {
        mnt_point = vfs_mount_point_find_and_ref(path);
        if (mnt_point)
        {
            if ((mnt_point->ops) && (mnt_point->ops->stat))
            {
                rel_mnt_path = vfs_get_rel_mnt_path(mnt_point->mnt_path, path);
                if (rel_mnt_path)
                {
                    ret = mnt_point->ops->stat(mnt_point, rel_mnt_path, buf);
                }
                else
                {
                    ret = -EINVAL;
                }
            }
            else
            {
                ret = -ENOENT;
            }
            vfs_mount_point_deref(mnt_point);
        }
        else
        {
            ret = -ENOENT;
        }
    }

    return ret;
}

int do_statfs(const char *path, struct statfs *buf)
{
    struct vfs_mountpoint *mnt_point;
    int ret;

    ret = -EINVAL;

    if (path)
    {
        mnt_point = vfs_mount_point_find_and_ref(path);
        if (mnt_point)
        {
            if ((mnt_point->ops) && (mnt_point->ops->statfs))
            {
                ret = mnt_point->ops->statfs(mnt_point, buf);
            }
            else
            {
                ret = -ENOENT;
            }
            vfs_mount_point_deref(mnt_point);
        }
        else
        {
            ret = -ENOENT;
        }
    }

    return ret;
}


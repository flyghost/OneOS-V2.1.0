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
 * @file        vfs_fs.c
 *
 * @brief       This file implements the basic operation of filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <oneos_config.h>
#include <os_memory.h>
#include <os_util.h>
#include <os_assert.h>
#include <os_spinlock.h>
#include <dlog.h>
#include <vfs.h>
#include <fcntl.h>
#include <sys/errno.h>
#include "vfs_private.h"

#ifdef OS_USING_VFS_DEVFS
#include <vfs_devfs.h>
#endif

#define MNT_PT_INITED       (0x5A5A)
#define VFS_MKFS_MAX        VFS_MOUNTPOINT_MAX

enum vfs_dev_ref_stat
{
    DEV_REF_NONE,
    DEV_REF_MOUNT,
    DEV_REF_MKFS,
    DEV_REF_NO_SPACE
};

static OS_DEFINE_SPINLOCK(gs_vfs_init_lock);
static unsigned short gs_vfs_init_flag = 0;
static const struct vfs_filesystem_ops *vfs_ops_table[VFS_FILESYSTEM_TYPES_MAX];
static struct vfs_mountpoint mnt_point_table[VFS_MOUNTPOINT_MAX];
static void *mkfs_dev_table[VFS_MKFS_MAX];

static const struct vfs_filesystem_ops *_vfs_fs_ops_get(const char *fs_name)
{
    unsigned short i;
    const struct vfs_filesystem_ops *fs_ops;

    fs_ops = OS_NULL;

    VFS_LOCK();

    for (i = 0; i < VFS_FILESYSTEM_TYPES_MAX; i++)
    {
        if ((vfs_ops_table[i]) && (strcmp(vfs_ops_table[i]->fs_name, fs_name) == 0))
        {
            fs_ops = vfs_ops_table[i];
            break;
        }
    }

    VFS_UNLOCK();

    return fs_ops;
}

static enum vfs_dev_ref_stat _vfs_mkfs_dev_ref(void *dev)
{
    unsigned short i;
    enum vfs_dev_ref_stat status;

    status = DEV_REF_NONE;

    VFS_LOCK();

    /* Check whether this device has been mounted. */
    for (i = 0; i < VFS_MOUNTPOINT_MAX; i++)
    {
        if (mnt_point_table[i].dev == dev)
        {
           status = DEV_REF_MOUNT;
           break;
        }
    }

    /* Check whether this device is doing mkfs. */
    if (DEV_REF_NONE == status)
    {
        for (i = 0; i < VFS_MKFS_MAX; i++)
        {
            if (mkfs_dev_table[i] == dev)
            {
                status = DEV_REF_MKFS;
                break;
            }
        }
    }

    /* Reference this device. */
    if (DEV_REF_NONE == status)
    {
        for (i = 0; i < VFS_MKFS_MAX; i++)
        {
            if (!mkfs_dev_table[i])
            {
                mkfs_dev_table[i] = dev;
                break;
            }
        }

        if (i >= VFS_MKFS_MAX)
        {
            status = DEV_REF_NO_SPACE;
        }
    }

    VFS_UNLOCK();

    return status;
}

static void _vfs_mkfs_dev_deref(void *dev)
{
    unsigned short i;

    VFS_LOCK();

    for (i = 0; i < VFS_MKFS_MAX; i++)
    {
        if (mkfs_dev_table[i] == dev)
        {
            mkfs_dev_table[i] = OS_NULL;
            break;
        }
    }

    VFS_UNLOCK();
}

static char *_vfs_mntpath_get(const char *path)
{
    char *mnt_path;

    mnt_path = vfs_create_absolute_path(OS_NULL, path);

    if (mnt_path)
    {
        /* If not root dir and /dev, should check whether the directory exist. */
        if ((0 != strcmp(mnt_path, "/")) && (0 != strcmp(mnt_path, DEVFS_PATH)))
        {
            struct vfs_dir *dp;
            int ret;

            ret = -1;
            dp = dp_alloc();
            if (dp)
            {
                dp_ref_inc(dp);
                ret = do_opendir(dp, mnt_path, 0);
                if (ret >= 0)
                {
                    ret = do_closedir(dp);
                }
                dp_ref_dec(dp);

                dp_free(dp);
            }

            if (ret < 0)
            {
                vfs_destroy_absolute_path(mnt_path);
                mnt_path = OS_NULL;
            }
        }
    }

    return mnt_path;
}

static struct vfs_mountpoint *_vfs_mount_point_add(void *dev, char *mnt_path, const struct vfs_filesystem_ops *fs_ops)
{
    unsigned short i;
    os_bool_t use_flag;
    struct vfs_mountpoint *mnt_point;

    use_flag  = OS_FALSE;
    mnt_point = OS_NULL;

    VFS_LOCK();

    for (i = 0; i < VFS_MKFS_MAX; i++)
    {
        if (dev && (mkfs_dev_table[i] == dev))
        {
            LOG_E(VFS_TAG, "ERROR. This device is doing mkfs, please try later.");
            use_flag = OS_TRUE;
            break;
        }
    }

    if (OS_FALSE == use_flag)
    {
        for (i = 0; i < VFS_MOUNTPOINT_MAX; i++)
        {
            if (mnt_point_table[i].ops)
            {
               /* Foreach item in mnt_point_table, if path already been mounted, return NULL. */
                if ((strcmp(mnt_point_table[i].mnt_path, mnt_path) == 0)
                    || (dev && (mnt_point_table[i].dev == dev)))
                {
                   LOG_E(VFS_TAG, "ERROR. Path or device has been mounted before.");
                   mnt_point = OS_NULL;
                   break;
                }
            }
            else
            {
               /* Use the first empty item. */
                if (!mnt_point)
                {
                    mnt_point = &mnt_point_table[i];
                }
            }
        }

        if (mnt_point)
        {
            mnt_point->mnt_path = mnt_path;
            mnt_point->ops = fs_ops;
            mnt_point->dev = dev;
        }
    }

    VFS_UNLOCK();

    return mnt_point;
}

static void _vfs_mount_point_set_init(struct vfs_mountpoint *mnt_point)
{
    VFS_LOCK();

    mnt_point->is_inited = MNT_PT_INITED;

    VFS_UNLOCK();
}

static void _vfs_mount_point_del(struct vfs_mountpoint *mnt_point, char *mnt_path)
{
    if (mnt_point)
    {
        VFS_LOCK();

        memset(mnt_point, 0, sizeof(struct vfs_mountpoint));

        VFS_UNLOCK();
    }

    if (mnt_path)
    {
        vfs_destroy_absolute_path(mnt_path);
    }
}

static struct vfs_mountpoint *_vfs_mount_point_get_from_mntpath(const char* mnt_path)
{
    unsigned short i;
    struct vfs_mountpoint *mnt_point;

    mnt_point = OS_NULL;

    VFS_LOCK();

    for (i = 0; i < VFS_MOUNTPOINT_MAX; i++)
    {
        if ((mnt_point_table[i].mnt_path) && (0 == strcmp(mnt_point_table[i].mnt_path, mnt_path)))
        {
            mnt_point = &mnt_point_table[i];
            mnt_point->ref_cnt++;
            break;
        }
    }

    VFS_UNLOCK();

    return mnt_point;
}

static int _vfs_mount_point_unmount(struct vfs_mountpoint *mnt_point)
{
    int ret;
    struct vfs_mountpoint mnt_point_tmp;

    ret = -1;

    VFS_LOCK();

    if (1 == mnt_point->ref_cnt)
    {
        /* Back up this mnt_point info to allow to call specify filesystem's unmount.*/
        memcpy(&mnt_point_tmp, mnt_point, sizeof(struct vfs_mountpoint));
        /* Delete this mnt_point to prevent other task access this mnt_point when we do unmout. */
        memset(mnt_point, 0, sizeof(struct vfs_mountpoint));

        ret = 0;
    }
    else
    {
        mnt_point->ref_cnt--;
        LOG_E(VFS_TAG, "Can't unmount now. This mount point was refereced, ref_cnt:%d.", mnt_point->ref_cnt);
        LOG_E(VFS_TAG, "You may use cmd: fd_show to see whether referenced by opened file/dir.");
        ret = (-EBUSY);
    }

    VFS_UNLOCK();

    if (0 == ret)
    {
        ret = mnt_point_tmp.ops->unmount(&mnt_point_tmp);
        if (ret >= 0)
        {
            vfs_destroy_absolute_path(mnt_point_tmp.mnt_path);
        }
        else    /* If unfortunately unmount fail, we should restore it back to mnt_point_table. */
        {
            _vfs_mount_point_add(mnt_point_tmp.dev, mnt_point_tmp.mnt_path, mnt_point_tmp.ops);
        }
    }

    return ret;
}

struct vfs_mountpoint *vfs_mount_point_find_and_ref(const char* abspath)
{
    struct vfs_mountpoint *mnt_point;
    unsigned short path_len;
    unsigned short mnt_path_len;
    unsigned short already_match_len;
    unsigned short i;

    mnt_point = OS_NULL;
    already_match_len = 0;

    if (abspath)
    {
        path_len = strlen(abspath);

        VFS_LOCK();

        for (i = 0; i < VFS_MOUNTPOINT_MAX; i++)
        {
            if ((!mnt_point_table[i].mnt_path) || (!mnt_point_table[i].ops) || (mnt_point_table[i].is_inited != MNT_PT_INITED))
            {
                continue;
            }

            mnt_path_len = strlen(mnt_point_table[i].mnt_path);
            if ((mnt_path_len < already_match_len) || (mnt_path_len > path_len))
            {
                continue;
            }

            /* check whether path have directory separator '/' at the the mount_path end.*/
            if ((mnt_path_len > 1) && (abspath[mnt_path_len] != '/') && (abspath[mnt_path_len] != '\0'))
            {
                continue;
            }

            if (strncmp(mnt_point_table[i].mnt_path, abspath, mnt_path_len) == 0)
            {
                mnt_point = &mnt_point_table[i];
                if (mnt_path_len == path_len)
                {
                    /* Find the best match mnt_path, break. */
                    break;
                }
                else/* mnt_path_len < path_len */
                {
                    /* Find a match mnt_path, but maybe not the longest match, 
                        record it, and then continue to search longger match.*/
                    already_match_len = mnt_path_len;
                }
            }
        }

        if (mnt_point)
        {
            mnt_point->ref_cnt++;
        }

        VFS_UNLOCK();
    }

    return mnt_point;
}

void vfs_mount_point_deref(struct vfs_mountpoint *mnt_point)
{
    VFS_LOCK();

    OS_ASSERT(mnt_point->ref_cnt > 0);
    mnt_point->ref_cnt--;

    VFS_UNLOCK();
}

int vfs_register(const struct vfs_filesystem_ops *fs_ops)
{
    unsigned short i;
    int ret;

    ret = -1;

    if (fs_ops)
    {
        VFS_LOCK();

        for (i = 0; i < VFS_FILESYSTEM_TYPES_MAX; i++)
        {
            if (vfs_ops_table[i])
            {
                if (strcmp(vfs_ops_table[i]->fs_name, fs_ops->fs_name) == 0)
                {
                    LOG_E(VFS_TAG, "Filesystem %s has already been registerd !", fs_ops->fs_name);
                    break;
                }
            }
            else
            {
                vfs_ops_table[i] = fs_ops;
                ret = 0;
                break;
            }
        }

        VFS_UNLOCK();

        if (VFS_FILESYSTEM_TYPES_MAX == i)
        {
            LOG_E(VFS_TAG, "Filesytem ops table is not enough, you may increase VFS_FILESYSTEM_TYPES_MAX");
        }
    }
    else
    {
        LOG_E(VFS_TAG, "Invaid fs_ops");
    }

    return ret;
}

int vfs_mount(const char *dev_name, const char *path, const char *fs_name, unsigned long mountflag, const void *data)
{
    const struct vfs_filesystem_ops *fs_ops;
    struct vfs_mountpoint *mnt_point;
    char *mnt_path;
    void *dev;
    int ret;

    fs_ops    = OS_NULL;
    mnt_point = OS_NULL;
    mnt_path  = OS_NULL;
    dev       = OS_NULL;
    ret       = 0;

    if ((!path) || (!fs_name))
    {
        LOG_E(VFS_TAG, "Invalid path or fs_name !");
        VFS_SET_ERRNO(-EINVAL);
        ret = -1;
    }

    /* For some fs, dev_name may not mandatory. But if need dev_name, we should check device exist. */
    if (dev_name)
    {
        dev = (void *)os_device_find(dev_name);
        if (!dev)
        {
            VFS_SET_ERRNO(-ENODEV);
            ret = -1;
        }
    }

    /* Check whether the path exist.*/
    if (0 == ret)
    {
        mnt_path = _vfs_mntpath_get(path);
        if (!mnt_path)
        {
            VFS_SET_ERRNO(-ENOTDIR);
            ret = -1;
        }
    }

    /* Find the fs ops. */
    if (0 == ret)
    {
        fs_ops = _vfs_fs_ops_get(fs_name);
        if ((!fs_ops) || (!fs_ops->mount))
        {
            VFS_SET_ERRNO(-ENOSYS);
            ret = -1;
        }
    }

    /* Get an entry to save the mount point info. */
    if (0 == ret)
    {
        mnt_point = _vfs_mount_point_add(dev, mnt_path, fs_ops);
        if (!mnt_point)
        {
            VFS_SET_ERRNO(-ENOSPC);
            ret = -1;
        }
    }

    /* Mount the specific filesystem. */
    if (0 == ret)
    {
        ret = fs_ops->mount(mnt_point, mountflag, data);
    }

    if (ret >= 0)
    {
        _vfs_mount_point_set_init(mnt_point);
        LOG_I(VFS_TAG, "Mount %s to %s", fs_name, path);
    }
    else
    {
        VFS_SET_ERRNO(ret);
        _vfs_mount_point_del(mnt_point, mnt_path);
    }
    ret = (ret < 0) ? (-1) : 0;

    return ret;
}

int vfs_unmount(const char *path)
{
    int ret;
    char *mnt_path;
    struct vfs_mountpoint *mnt_point;

    ret = -1;

    mnt_path = vfs_create_absolute_path(OS_NULL, path);
    if (mnt_path)
    {
        mnt_point = _vfs_mount_point_get_from_mntpath(mnt_path);
        if (mnt_point && mnt_point->ops && mnt_point->ops->unmount)
        {
            ret = _vfs_mount_point_unmount(mnt_point);
            if (ret < 0)
            {
                VFS_SET_ERRNO(ret);
            }
            ret = (ret < 0) ? (-1) : 0;
        }
        else
        {
            VFS_SET_ERRNO(-ENOENT);
        }
        vfs_destroy_absolute_path(mnt_path);
    }
    else
    {
        VFS_SET_ERRNO(-ENOENT);
    }

    return ret;
}

int vfs_mkfs(const char *fs_name, const char *dev_name)
{
    int ret;
    void *dev;
    const struct vfs_filesystem_ops *fs_ops;
    enum vfs_dev_ref_stat status;

    ret = -1;
    dev = OS_NULL;

    if (fs_name && dev_name)
    {
        dev = (void *)os_device_find(dev_name);
    }

    if (dev)
    {
        status = _vfs_mkfs_dev_ref(dev);
        if (DEV_REF_MOUNT == status)
        {
            LOG_E(VFS_TAG, "This device has been mounted, you should unmount it before mkfs");
            VFS_SET_ERRNO(-EBUSY);
        }
        else if (DEV_REF_MKFS == status)
        {
            LOG_E(VFS_TAG, "This device is already doing mkfs, you may try it later");
            VFS_SET_ERRNO(-EBUSY);
        }
        else if (DEV_REF_NO_SPACE == status)
        {
            LOG_E(VFS_TAG, "Too many device is doing mkfs, you may try it later");
            VFS_SET_ERRNO(-EBUSY);
        }
        else
        {
            fs_ops = _vfs_fs_ops_get(fs_name);
            if (fs_ops && fs_ops->mkfs)
            {
                ret = fs_ops->mkfs(dev);
                if (ret < 0)
                {
                    VFS_SET_ERRNO(ret);
                }
                ret = (ret < 0) ? (-1) : 0;
            }
            else
            {
                LOG_E(VFS_TAG, "The file system (%s) mkfs function was not found", fs_name);
                VFS_SET_ERRNO(-ENOSYS);
                ret = -1;
            }
            _vfs_mkfs_dev_deref(dev);
        }
    }
    else
    {
        LOG_E(VFS_TAG, "Device (%s) was not found", dev_name);
        VFS_SET_ERRNO(-ENODEV);
    }

    return ret;
}

int vfs_init(void)
{
    int ret;

    os_spin_lock(&gs_vfs_init_lock);

    if (0 == gs_vfs_init_flag)
    {
        gs_vfs_init_flag = 1;
        os_spin_unlock(&gs_vfs_init_lock);

        VFS_LOCK_INIT();

        memset((void *)vfs_ops_table, 0, sizeof(vfs_ops_table));
        memset((void *)mnt_point_table, 0, sizeof(mnt_point_table));
        memset((void *)mkfs_dev_table, 0, sizeof(mkfs_dev_table));
        working_dir_init();
        fd_table_init();

#ifdef OS_USING_VFS_DEVFS
        if (0 == vfs_devfs_init())
        {
            (void)vfs_mount(OS_NULL, DEVFS_PATH, "dev", 0, OS_NULL);
        }
#endif

        ret = 0;
     }
     else
     {
        LOG_E(VFS_TAG, "The VFS has already been inited. ");
        ret = -1;
     }

    return ret;
}
OS_POSTCORE_INIT(vfs_init, OS_INIT_SUBLEVEL_HIGH);


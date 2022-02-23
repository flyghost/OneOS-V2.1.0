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
 * @file        vfs_devfs.c
 *
 * @brief       This file implements the device filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-26   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_errno.h>
#include <sys/errno.h>
#include <os_memory.h>
#include <os_mutex.h>
#include <os_list.h>
#include <os_spinlock.h>
#include <dlog.h>
#include <vfs.h>
#include <vfs_fs.h>
#include <vfs_devfs.h>

#define DEVFS_TAG       "DEVFS"

struct devfs_fd
{
    struct devfs_device *   fs_dev;
    /* TODO, for device file, should we save pos info here  ? */
    off_t                   pos;    /* File position or dir entry position. */
    os_spinlock_t           lock;
};

struct devfs_device {
    os_uint16_t             ref_cnt;
    os_list_node_t          dev_node;
    char                    name[OS_NAME_MAX + 1];
    struct dev_file_ops    *ops;
    void                   *private;
};
static os_list_node_t gs_devfs_dev_list_head = OS_LIST_INIT(gs_devfs_dev_list_head);

static os_mutex_t devfs_lock;

#define DEVFS_LOCK_INIT()   os_mutex_init(&devfs_lock, "devfslock", OS_FALSE);
#define DEVFS_LOCK()        os_mutex_lock(&devfs_lock, OS_WAIT_FOREVER)
#define DEVFS_UNLOCK()      os_mutex_unlock(&devfs_lock);

static int vfs_devfs_open(struct vfs_file* fp)
{
    int ret;
    struct devfs_device *fs_dev;
    struct devfs_fd *dev_fd;
    const char *name;

    ret = -ENOENT;
    if (fp && fp->path)
    {
        name = &fp->path[1];     /* path[0] is '/', skip it. */
        DEVFS_LOCK();
        os_list_for_each_entry(fs_dev, &gs_devfs_dev_list_head, struct devfs_device, dev_node)
        {
            if ((strcmp(name, fs_dev->name) == 0) && fs_dev->ops && fs_dev->ops->open)
            {
                fs_dev->ref_cnt++;
                ret = 0;
                break;
            }
        }
        DEVFS_UNLOCK();
    }

    if (0 == ret)
    {
        dev_fd = os_malloc(sizeof(struct devfs_fd));
        if (dev_fd)
        {
            dev_fd->pos = 0;/*Do not know the file size, no matter flags is O_TRUNC or O_APPEND, set pos to zero. */
            dev_fd->fs_dev = fs_dev;
            os_spin_lock_init(&dev_fd->lock);

            fp->desc = (void *)dev_fd;
            fp->private = fs_dev->private;
            ret = fs_dev->ops->open(fp);
            if (0 != ret)
            {
                DEVFS_LOCK();
                fs_dev->ref_cnt--;
                DEVFS_UNLOCK();
                os_free(dev_fd);
                ret = -EIO;
            }
        }
        else
        {
            ret = -ENOMEM;
        }
    }

    return ret;
}

static int vfs_devfs_close(struct vfs_file* fp)
{
    int ret;
    struct devfs_device *fs_dev;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (fp && fp->desc)
    {
        dev_fd = (struct devfs_fd *)fp->desc;
        fs_dev = dev_fd->fs_dev;
        if (fs_dev)
        {
            if (fs_dev->ops && fs_dev->ops->close)
            {
                ret = fs_dev->ops->close(fp);
                if (0 == ret)
                {
                    DEVFS_LOCK();
                    fs_dev->ref_cnt--;
                    DEVFS_UNLOCK();
                }
                else
                {
                    ret = -EIO;
                }

                /* no matter successful or fail, free.*/
                os_free(dev_fd);
            }
            else
            {
                ret = -ENOSYS;
            }
        }
        else
        {
            ret = -EINVAL;
        }
    }

    return ret;
}

static int vfs_devfs_read(struct vfs_file *fp, void *buf, size_t count)
{
    int ret;
    struct devfs_device *fs_dev;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (fp && fp->desc && buf)
    {
        dev_fd = (struct devfs_fd *)fp->desc;
        fs_dev = dev_fd->fs_dev;
        if (fs_dev)
        {
            if (fs_dev->ops && fs_dev->ops->read)
            {
                ret = fs_dev->ops->read(fp, dev_fd->pos, buf, count);
                if (ret > 0)
                {
                    os_spin_lock(&dev_fd->lock);
                    dev_fd->pos += ret;
                    os_spin_unlock(&dev_fd->lock);
                }
            }
            else
            {
                ret = -ENOSYS;
            }
        }
        else
        {
            ret = -EINVAL;
        }
    }

    return ret;
}

static int vfs_devfs_write(struct vfs_file *fp, const void *buf, size_t count)
{
    int ret;
    struct devfs_device *fs_dev;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (fp && fp->desc && buf)
    {
        dev_fd = (struct devfs_fd *)fp->desc;
        fs_dev = dev_fd->fs_dev;
        if (fs_dev)
        {
            if (fs_dev->ops && fs_dev->ops->write)
            {
                ret = fs_dev->ops->write(fp, dev_fd->pos, buf, count);
                if (ret > 0)
                {
                    os_spin_lock(&dev_fd->lock);
                    dev_fd->pos += ret;
                    os_spin_unlock(&dev_fd->lock);
                }
            }
            else
            {
                ret = -ENOSYS;
            }
        }
        else
        {
            ret = -EINVAL;
        }
    }

    return ret;
}

static int vfs_devfs_lseek(struct vfs_file *fp, off_t offset, int whence)
{
    int ret;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (fp && fp->desc)
    {
        dev_fd = (struct devfs_fd *)fp->desc;
        os_spin_lock(&dev_fd->lock);
        switch (whence)
        {
        case SEEK_SET:
            dev_fd->pos = offset;
            ret = dev_fd->pos;
            break;
        case SEEK_CUR:
            dev_fd->pos += offset;
            ret = dev_fd->pos;
            break;
        case SEEK_END:
            ret = -EINVAL;      /* TODO ? do not know the size, not support SEEK_END. */
            break;
        default:
            ret = -EINVAL;
            break;
        }
        os_spin_unlock(&dev_fd->lock);
    }

    return ret;
}


static int vfs_devfs_ioctl(struct vfs_file *fp, unsigned long cmd, void *args)
{
    int ret;
    struct devfs_device *fs_dev;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (fp && fp->desc)
    {
        dev_fd = (struct devfs_fd *)fp->desc;
        fs_dev = dev_fd->fs_dev;
        if (fs_dev)
        {
            if (fs_dev->ops && fs_dev->ops->ioctl)
            {
                ret = fs_dev->ops->ioctl(fp, cmd, args);
                if (0 != ret)
                {
                    ret = -EIO;
                }
            }
            else
            {
                ret = -ENOSYS;
            }
        }
        else
        {
            ret = -EINVAL;
        }
    }

    return ret;
}

#ifdef OS_USING_IO_MULTIPLEXING
static int vfs_devfs_poll(struct vfs_file *fp, struct vfs_pollfd *poll_fd, os_bool_t poll_setup)
{
    int ret;
    struct devfs_device *fs_dev;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (fp && fp->desc)
    {
        dev_fd = (struct devfs_fd *)fp->desc;
        fs_dev = dev_fd->fs_dev;
        if (fs_dev)
        {
            if (fs_dev->ops && fs_dev->ops->poll)
            {
                ret = fs_dev->ops->poll(fp, poll_fd, poll_setup);
                if (ret < 0)
                {
                    ret = -EIO;
                }
            }
            else
            {
                ret = -ENOSYS;
            }
        }
        else
        {
            ret = -EINVAL;
        }
    }

    return ret;
}
#endif

static int vfs_devfs_opendir(struct vfs_dir *dp)
{
    int ret;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (dp && dp->fp)
    {
        if (strcmp(dp->fp->path, "/") == 0)     /* If devfs's directory. */
        {
            dev_fd = os_malloc(sizeof(struct devfs_fd));
            if (dev_fd)
            {
                dev_fd->pos = 0;
                dev_fd->fs_dev = OS_NULL;
                os_spin_lock_init(&dev_fd->lock);

                dp->fp->desc = (void *)dev_fd;
                dp->fp->private = OS_NULL;
                dp->fp->type = FT_DIRECTORY;
                ret = 0;
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
    }

    return ret;
}

static int vfs_devfs_closedir(struct vfs_dir *dp)
{
    int ret;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (dp && dp->fp && dp->fp->desc)
    {
        dev_fd = (struct devfs_fd *)dp->fp->desc;
        os_free(dev_fd);
        ret = 0;
    }

    return ret;
}

static int vfs_devfs_readdir(struct vfs_dir *dp, struct dirent *dentry)
{
    struct devfs_fd *dev_fd;
    struct devfs_device *fs_dev;
    os_uint32_t index;
    int read_cnt;
    int entry_pos;

    index = 0;
    read_cnt = 0;
    if (dp && dp->fp && dp->fp->desc)
    {
        dev_fd = (struct devfs_fd *)dp->fp->desc;
        DEVFS_LOCK();
        entry_pos = dev_fd->pos;
        os_list_for_each_entry(fs_dev, &gs_devfs_dev_list_head, struct devfs_device, dev_node)
        {
            if (index >= entry_pos)
            {
                dentry->d_type = DT_CHR;
                strncpy(dentry->d_name, fs_dev->name, OS_NAME_MAX);
                entry_pos++;
                read_cnt++;
                break;
            }
            index++;
        }
        DEVFS_UNLOCK();

        os_spin_lock(&dev_fd->lock);
        dev_fd->pos = entry_pos;
        os_spin_unlock(&dev_fd->lock);
    }

    return read_cnt;
}

static int vfs_devfs_seekdir(struct vfs_dir *dp, off_t offset)
{
    int ret;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (dp && dp->fp && dp->fp->desc)
    {
        dev_fd = (struct devfs_fd *)dp->fp->desc;
        os_spin_lock(&dev_fd->lock);
        dev_fd->pos = offset;
        os_spin_unlock(&dev_fd->lock);
        ret = offset;
    }

    return ret;
}

static long vfs_devfs_telldir(struct vfs_dir *dp)
{
    int ret;
    struct devfs_fd *dev_fd;

    ret = -EINVAL;
    if (dp && dp->fp && dp->fp->desc)
    {
        dev_fd = (struct devfs_fd *)dp->fp->desc;
        ret = dev_fd->pos;
    }

    return (long)ret;
}

static int vfs_devfs_stat(struct vfs_mountpoint *mnt_point, const char *path, struct stat *buf)
{
    int ret = 0;
    const char *name;
    os_uint16_t type;
    struct devfs_device *fs_dev;

    ret = -ENOENT;
    if (path)
    {
        if (strcmp(path, "/") == 0)     /* If devfs's directory. */
        {
            type = FT_DIRECTORY;
            ret = 0;
        }
        else
        {
            name = &path[1];
            DEVFS_LOCK();
            os_list_for_each_entry(fs_dev, &gs_devfs_dev_list_head, struct devfs_device, dev_node)
            {
                if (strcmp(name, fs_dev->name) == 0)
                {
                    type = FT_DEVICE;
                    ret = 0;
                    break;
                }
            }
            DEVFS_UNLOCK();
        }
    }

    if (0 == ret)
    {
        buf->st_mode = S_IFCHR | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH;
        if (FT_DIRECTORY == type)
        {
            buf->st_mode &= ~S_IFCHR;
            buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        }
        buf->st_size = 0;
        buf->st_mtime = 0;
    }

    return ret;
}

static int vfs_devfs_mount(struct vfs_mountpoint *mnt_point, unsigned long mountflag, const void *data)
{
    int ret;
    static int init_cnt = 0;

    if (init_cnt == 0)
    {
        init_cnt++;
        ret = 0;
    }
    else             /* Only allow to be mounted once, And not allow be unmounted. */
    {
        ret = -1;
    }

    return ret;
}

static const struct vfs_filesystem_ops _devfs =
{
    "dev",

    vfs_devfs_open,
    vfs_devfs_close,
    vfs_devfs_read,
    vfs_devfs_write,
    vfs_devfs_lseek,
    OS_NULL,                /* Not support sync. */
    vfs_devfs_ioctl,
#ifdef OS_USING_IO_MULTIPLEXING
    vfs_devfs_poll,
#endif

    vfs_devfs_opendir,
    vfs_devfs_closedir,
    vfs_devfs_readdir,
    vfs_devfs_seekdir,
    vfs_devfs_telldir,

    OS_NULL,                /* Not support rename. */
    OS_NULL,                /* Not support unlink. */
    vfs_devfs_stat,

    OS_NULL,                /* Not support mkfs. */
    OS_NULL,                /* Not support statfs. */
    vfs_devfs_mount,
    OS_NULL,                /* Not support unmount. */
};

os_err_t devfs_register_device(const char *dev_name, struct dev_file_ops *ops, void *private)
{
    os_err_t ret;
    struct devfs_device *fs_dev;
    struct devfs_device *fs_dev_new;

    ret = OS_EINVAL;
    if (dev_name && ops)
    {
        fs_dev_new = os_malloc(sizeof(struct devfs_device));
        if (fs_dev_new)
        {
            memset(fs_dev_new, 0, sizeof(struct devfs_device));
            strncpy(fs_dev_new->name, dev_name, OS_NAME_MAX);
            fs_dev_new->ops = ops;
            fs_dev_new->private = private;

            ret = OS_EOK;
            DEVFS_LOCK();
            os_list_for_each_entry(fs_dev, &gs_devfs_dev_list_head, struct devfs_device, dev_node)
            {
                if (strcmp(dev_name, fs_dev->name) == 0)
                {
                    LOG_E(DEVFS_TAG, "The device:%s has already been registered", dev_name);
                    ret = OS_EBUSY;
                    break;
                }
            }
            if (OS_EOK == ret)
            {
                os_list_add(&gs_devfs_dev_list_head, &fs_dev_new->dev_node);
            }
            DEVFS_UNLOCK();

            if (OS_EOK != ret)
            {
                os_free(fs_dev_new);
            }
        }
        else
        {
            ret = OS_ENOMEM;
        }
    }

    return ret;
}

os_err_t devfs_unregister_device(const char *dev_name)
{
    os_err_t ret;
    struct devfs_device *fs_dev;
    struct devfs_device *fs_dev_tmp;

    ret = OS_EINVAL;
    if (dev_name)
    {
        DEVFS_LOCK();
        os_list_for_each_entry_safe(fs_dev, fs_dev_tmp, &gs_devfs_dev_list_head, struct devfs_device, dev_node)
        {
            if (strcmp(dev_name, fs_dev->name) == 0)
            {
                if (fs_dev->ref_cnt == 0)
                {
                    os_list_del(&fs_dev->dev_node);
                    ret = OS_EOK;
                }
                else
                {
                    LOG_E(DEVFS_TAG, "The device file:%s should be closed before unregister, ref_cnt:%d", dev_name, fs_dev->ref_cnt);
                    ret = OS_EBUSY;
                }
                break;
            }
        }
        DEVFS_UNLOCK();

        if (OS_EOK == ret)
        {
            os_free(fs_dev);
        }
    }

    return ret;
}

int vfs_devfs_init(void)
{
    /* Register device file system. */
    DEVFS_LOCK_INIT();
    return vfs_register(&_devfs);
}


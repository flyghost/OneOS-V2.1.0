/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        vfs_cutefs.c
 *
 * @brief       This file implement the RAM filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-10   OneOS Team      First version, now only support run on ramdisk.
 ***********************************************************************************************************************
 */
#include <string.h>
#include <os_memory.h>
#include <os_stddef.h>
#include <dlog.h>
#include <vfs_fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/errno.h>
#include <cutefs/vfs_cutefs.h>
#include "cutefs.h"
#include "cutefs_block.h"

/**
 ***********************************************************************************************************************
 * @brief           Mount cute filesystem.
 *
 * @param[in,out]   mnt_point       The pointer of VFS object.
 * @param[in]       mountflag       The read/write flag, not used now.
 * @param[in]       data            private data, not used now.
 *
 * @return          The mount result.
 * @retval          0               Mount successfully.
 * @retval          -EIO            Mount failed, return the error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_mount(struct vfs_mountpoint *mnt_point, unsigned long mountflag, const void *data)
{
    struct cutefs *cfs = OS_NULL;

    if (!mnt_point)
    {
        return -EIO;
    }

    if (os_device_open((os_device_t *)mnt_point->dev) == OS_EOK)
    {
        cfs = cutefs_mount((os_device_t *)mnt_point->dev);
    }

    if (!cfs)
    {
        return -EIO;
    }
    mnt_point->fs_instance = cfs;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Unmount cute filesystem.
 *
 * @param[in,out]   mnt_point       The pointer of VFS object.
 *
 * @return          The unmount result.
 * @retval          0               Unmount successfully.
 * @retval          -EIO            Unmount failed.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_unmount(struct vfs_mountpoint *mnt_point)
{
    struct cutefs *cfs;

    if ((!mnt_point) || (!mnt_point->fs_instance))
    {
        return -EIO;
    }

    cfs = mnt_point->fs_instance;
    if (cutefs_unmount(cfs) < 0)
    {
        return -EIO;
    }
    os_device_close((os_device_t *)mnt_point->dev);
    mnt_point->fs_instance = OS_NULL;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the cute filesystem stat.
 *
 * @param[in]       mnt_point       The pointer of VFS object.
 * @param[out]      buf             The pointer of buf to save the cute filesystem stat.
 *
 * @return          The operation result.
 * @retval          0               Get stat successfully.
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_statfs(struct vfs_mountpoint *mnt_point, struct statfs *buf)
{
    struct cutefs *cfs;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!buf))
    {
        return -EINVAL;
    }
    cfs = (struct cutefs *)mnt_point->fs_instance;
    cutefs_statfs(cfs, buf);

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Open file in cute filesystem.
 *
 * @param[in,out]   fp              The file pointer.
 *
 * @return          The open result.
 * @retval          0               Open file successfully.
 * @retval          Other           Open failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_open(struct vfs_file *fp)
{
    struct cutefs *cfs;
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    int result;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance))
    {
        return -EINVAL;
    }

    mnt_point = (struct vfs_mountpoint *)fp->mnt_point;
    cfs = (struct cutefs *)mnt_point->fs_instance;
    cfs_fd = os_malloc(sizeof(struct cutefs_fd));
    if (!cfs_fd)
    {
        return -ENOMEM;
    }
    memset(cfs_fd, 0, sizeof(struct cutefs_fd));
    result = cutefs_open(cfs, cfs_fd, (char *)fp->path, fp->flags);
    if (result < 0)
    {
        os_free(cfs_fd);
        return result;
    }

    fp->desc = cfs_fd;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Close file in cute filesystem.
 *
 * @param[in,out]   fp              The file pointer.
 *
 * @return          The close result.
 * @retval          0               File close successfully.
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_close(struct vfs_file *fp)
{
    struct cutefs *cfs;
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc))
    {
        return -EINVAL;
    }

    mnt_point = (struct vfs_mountpoint *)fp->mnt_point;
    cfs = (struct cutefs *)mnt_point->fs_instance;
    cfs_fd = fp->desc;
    if (cutefs_close(cfs, cfs_fd) < 0)
    {
        return -EINVAL;
    }
    os_free(cfs_fd);
    fp->desc = NULL;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Read file.
 *
 * @param[in,out]   fp              The file pointer.
 * @param[out]      buf             The pointer of buffer to save read content.
 * @param[in]       len             The expected read size.
 *
 * @return          The actual read size.
 * @retval          positive int    The actual read size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_read(struct vfs_file *fp, void *buf, size_t size)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    struct cutefs *cfs;
    int read_size = 0;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc) || (!buf))
    {
        return -EINVAL;
    }

    mnt_point = fp->mnt_point;
    cfs = mnt_point->fs_instance;
    cfs_fd = (struct cutefs_fd *)fp->desc;
    if (size > 0)
    {
        read_size = cutefs_read(cfs, cfs_fd, buf, size);
    }

    return read_size;
}

/**
 ***********************************************************************************************************************
 * @brief           Write file.
 *
 * @param[in,out]   fp              The file pointer.
 * @param[in]       buf             The pointer of data to write.
 * @param[in]       len             The expected write size.
 *
 * @return          The actual write size.
 * @retval          positive int    The actual write size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_write(struct vfs_file *fp, const void *buf, size_t size)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    struct cutefs *cfs;
    int write_size = 0;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc) || (!buf))
    {
        return -EINVAL;
    }

    mnt_point = fp->mnt_point;
    cfs = mnt_point->fs_instance;
    cfs_fd = (struct cutefs_fd *)fp->desc;
    if (size > 0)
    {
        write_size = cutefs_write(cfs, cfs_fd, buf, size);
    }

    return write_size;
}

/**
 ***********************************************************************************************************************
 * @brief           Reposition read/write file offset.
 *
 * @param[in,out]   fp              The file pointer.
 * @param[in]       offset          The new offset in file.
 *
 * @return          The lseek result.
 * @retval          int             The new position in file.  
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_lseek(struct vfs_file *fp, off_t offset, int whence)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    struct cutefs *cfs;
    off_t pos = 0;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc) || (fp->type != FT_REGULAR))
    {
        return -EINVAL;
    }

    mnt_point = fp->mnt_point;
    cfs = mnt_point->fs_instance;
    cfs_fd = (struct cutefs_fd *)fp->desc;

	switch (whence)
	{
	case SEEK_SET:
		pos = offset;
		break;
	case SEEK_CUR:
		pos = cfs_fd->pos + offset;
		break;
	case SEEK_END:
		pos = cfs_fd->size + offset;
		break;
	default:
		return -EINVAL;
	}

	if (pos < 0) {
		return -EINVAL;
	}

    pos = cutefs_seek(cfs, cfs_fd, pos);

    return pos;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete dir entry from cute filesystem.
 *
 * @param[in]       mnt_point       The pointer of VFS object.
 * @param[in]       path            The path (the file name) to be delete.
 *
 * @return          The delete result.
 * @retval          0               Delete successfully.
 * @retval          others          Delete fail, return err code.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_unlink(struct vfs_mountpoint *mnt_point, const char *path)
{
    struct cutefs *cfs;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!path))
    {
        return -EINVAL;
    }

    cfs = (struct cutefs *)mnt_point->fs_instance;
    return cutefs_unlink(cfs, path);
}

/**
 ***********************************************************************************************************************
 * @brief           Get the file status.
 *
 * @param[in]       mnt_point       The pointer of VFS object.
 * @param[in]       path            The file path.
 * @param[out]      st              The pointer to save file status.
 *
 * @return          The operation result.
 * @retval          0               Get status successfully.
 * @retval          -ENOENT         Not found the file entry. 
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_stat(struct vfs_mountpoint *mnt_point, const char *path, struct stat *st)
{
    struct cutefs *cfs;
    struct cutefs_fileinfo info;
    int result;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!path) || (!st))
    {
        return -EINVAL;
    }
    cfs = (struct cutefs *)mnt_point->fs_instance;
    result = cutefs_stat(cfs, path, &info);
    if (result < 0)
    {
        return -ENOENT;
    }

    //st->st_dev = 0;
    st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                  S_IWUSR | S_IWGRP | S_IWOTH;
    if (info.type == FT_DIRECTORY)
    {
        st->st_mode &= ~S_IFREG;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    }
    st->st_size = info.size;
    st->st_mtime = 0;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Rename file/directory in cute filesystem.
 *
 * @param[in]       mnt_point       The pointer of VFS object.
 * @param[in]       oldpath         The old file/directory name.
 * @param[in]       newpath         The new file/directory name.
 *
 * @return          The rename result.
 * @retval          0               Rename successfully.
 * @retval          -EEXIST         The new file/directory name already exist.
 * @retval          -ENOENT         The old file/directory entry not found.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_rename(struct vfs_mountpoint *mnt_point, const char *oldpath, const char *newpath)
{
    struct cutefs *cfs;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!oldpath) || (!newpath))
    {
        return -EINVAL;
    }
    cfs = (struct cutefs *)mnt_point->fs_instance;

    return cutefs_rename(cfs, oldpath, newpath);
}

int vfs_cutefs_opendir(struct vfs_dir *dp)
{
    struct cutefs *cfs;
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    int result;
    os_uint32_t flags = 0;

    if ((!dp) || (!dp->fp) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        return -EINVAL;
    }

    mnt_point = (struct vfs_mountpoint *)dp->fp->mnt_point;
    cfs = (struct cutefs *)mnt_point->fs_instance;
    cfs_fd = os_malloc(sizeof(struct cutefs_fd));
    if (!cfs_fd)
    {
        return -ENOMEM;
    }
    if (dp->fp->type == FT_DIRECTORY)
    {
        flags = (dp->fp->flags | O_DIRECTORY);
    }
    result = cutefs_open(cfs, cfs_fd, (char *)dp->fp->path, flags);
    if (result < 0)
    {
        os_free(cfs_fd);
        return result;
    }

    dp->fp->desc = cfs_fd;

    return result;
}
int vfs_cutefs_closedir(struct vfs_dir *dp)
{
    struct cutefs *cfs;
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance) || (!dp->fp->desc))
    {
        return -EINVAL;
    }

    mnt_point = (struct vfs_mountpoint *)dp->fp->mnt_point;
    cfs = (struct cutefs *)mnt_point->fs_instance;
    cfs_fd = dp->fp->desc;
    if (cutefs_close(cfs, cfs_fd) < 0)
    {
        return -EINVAL;
    }
    os_free(cfs_fd);
    dp->fp->desc = NULL;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Get dir entry.
 *
 * @param[in,out]   dp              The file descriptor of dir path.
 * @param[out]      dent            The pointer to save dir entry.
 *
 * @return          The read size.
 * @retval          int             The actual read size of dir entry.
 * @retval          -EINVAL         The parameter error.
 ***********************************************************************************************************************
 */
static int vfs_cutefs_readdir(struct vfs_dir *dp, struct dirent *dent)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    struct cutefs *cfs;
    int ret;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance) || (!dent))
    {
        return -EINVAL;
    }

    mnt_point = dp->fp->mnt_point;
    cfs = mnt_point->fs_instance;
    cfs_fd = (struct cutefs_fd *)dp->fp->desc;
    ret = cutefs_readdir(cfs, cfs_fd, dent);

    return ret;
}

int vfs_cutefs_seekdir(struct vfs_dir *dp, off_t offset)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    struct cutefs *cfs;
    off_t pos = 0;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        return -EINVAL;
    }

    mnt_point = dp->fp->mnt_point;
    cfs = mnt_point->fs_instance;
    cfs_fd = (struct cutefs_fd *)dp->fp->desc;

    pos = cutefs_seek(cfs, cfs_fd, offset);

    return pos;
}

long vfs_cutefs_telldir(struct vfs_dir *dp)
{
    struct cutefs_fd *cfs_fd;
    struct vfs_mountpoint *mnt_point;
    struct cutefs *cfs;
    off_t pos = 0;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        return -EINVAL;
    }
    mnt_point = dp->fp->mnt_point;
    cfs = mnt_point->fs_instance;
    cfs_fd = (struct cutefs_fd *)dp->fp->desc;

    pos = cutefs_tell(cfs, cfs_fd);
    return pos;
}

static const struct vfs_filesystem_ops _cutefs =
{
    "cute",

    vfs_cutefs_open,
    vfs_cutefs_close,
    vfs_cutefs_read,
    vfs_cutefs_write,
    vfs_cutefs_lseek,
    OS_NULL,                /* Not support sync. */
    OS_NULL,                /* Not support ioctl. */
#ifdef OS_USING_IO_MULTIPLEXING
    OS_NULL,                /* Not support poll. */
#endif

    vfs_cutefs_opendir,
    vfs_cutefs_closedir,
    vfs_cutefs_readdir,
    vfs_cutefs_seekdir,
    vfs_cutefs_telldir,

    vfs_cutefs_rename,
    vfs_cutefs_unlink,
    vfs_cutefs_stat,

    NULL,                   /* Not support mkfs. */
    vfs_cutefs_statfs,
    vfs_cutefs_mount,
    vfs_cutefs_unmount,
};

/**
 ***********************************************************************************************************************
 * @brief           Register cutefs operation structure to VFS.
 *
 * @param[in,out]   None.
 *
 * @return          The register result. 
 * @retval          0               Register successfully.
 * @retval          -1              Register failed.
 ***********************************************************************************************************************
 */
int vfs_cutefs_init(void)
{
    /* Register cute file system. */
    return vfs_register(&_cutefs);
}
OS_CMPOENT_INIT(vfs_cutefs_init, OS_INIT_SUBLEVEL_LOW);


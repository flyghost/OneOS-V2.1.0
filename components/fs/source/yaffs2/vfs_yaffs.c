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
 * @file        vfs_yaffs.c
 *
 * @brief       This file is adapter for vfs and yaffs2.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <vfs_fs.h>
#include <vfs.h>
#include <nand.h>
#include <os_memory.h>
#include "yaffs/yaffs_guts.h"
#include "yaffs/direct/yaffsfs.h"

#include "yportenv.h"
#include <string.h>



static int vfs_yaffs_open(struct vfs_file *file)
{
    struct vfs_mountpoint *fs;
    struct yaffs_dev *yfs_dev;
    int fd;
    
    if ((!file) || (!file->mnt_point))
    {
        return -EINVAL;
    }
    fs = (struct vfs_mountpoint *)file->mnt_point;
    yfs_dev = (struct yaffs_dev *)fs->fs_instance;
    

    fd = yaffs_open_reldev(yfs_dev, file->path, file->flags, S_IRUSR | S_IWUSR);
	
    if (fd < 0)
    {
       
        return yaffsfs_GetLastError();
    }
	
    file->desc = (void *)fd;
    
    if (file->flags & O_APPEND)
    { 
        yaffs_lseek(fd, 0, SEEK_END); 
    }
    
    return 0;
}

static int vfs_yaffs_close(struct vfs_file *file)
{
    if ((!file) || ((file->type != FT_REGULAR) && (file->type != FT_DIRECTORY)))
    {
        return -EINVAL;
    }

    if (((int)(file->desc)) < 0)
    {
        return -EINVAL;
    }

    if (yaffs_close((int)(file->desc)) < 0)
    {
        return yaffsfs_GetLastError();
    }
   
    file->desc = NULL;
    
    return 0;
}

static int vfs_yaffs_read(struct vfs_file *file, void *buf, size_t len)
{
    int fd;
    int read_size;

    if ((!file) || (((int)(file->desc)) < 0) || (!buf))
    {
        return -EINVAL;
    }
    
    fd = (int)(file->desc);
    read_size = yaffs_read(fd, buf, len);
	
    if (read_size < 0)
    {
        return yaffsfs_GetLastError();
    }

    return read_size;
}

static int vfs_yaffs_write(struct vfs_file *file, const void *buf, size_t len)
{
    int fd;
    int write_size;

    if ((!file) || (((int)(file->desc)) < 0) || (!buf))
    {
        return -EINVAL;
    }
    fd = (int)(file->desc);

    write_size = yaffs_write(fd, buf, len);
    if (write_size < 0)
    {
        return yaffsfs_GetLastError();
    }
    
    return write_size;
}

static int vfs_yaffs_lseek(struct vfs_file *file, os_off_t offset, int whence)
{
    int fd;
    int result = -1;
    
    if (!file)
    {
        return -EINVAL;
    }

    fd = (int)(file->desc);
    
    if (fd < 0)
    {
        return -EINVAL;
    }
    
    result = yaffs_lseek(fd, offset, whence);
    
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }
    
    return result;
}


static int vfs_yaffs_flush(struct vfs_file *file)
{
    int fd;
    int result;

    if ((!file) || (((int)(file->desc)) < 0))
    {
        return -EINVAL;
    }
    fd = (int)(file->desc);

    result = yaffs_flush(fd);
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    return 0;
}

static int vfs_yaffs_opendir(struct vfs_dir *dp)
{
	struct yaffs_dev *yfs_dev;
	struct yaffs_fd *yfs_fd;
	int result;
	
	if ((!dp) || (!dp->fp) || (!dp->fp->mnt_point))
	{
		return -EINVAL;
	}
	
	yfs_dev = (struct yaffs_dev *)dp->fp->mnt_point->fs_instance;
    yfs_fd = (struct yaffs_fd *)os_malloc(sizeof(struct yaffs_fd));
    if (!yfs_fd)
    {
        return -ENOMEM;
    }
    memset(yfs_fd, 0, sizeof(struct yaffs_fd));

	if (dp->fp->flags & O_CREAT)
	{
		result = yaffs_mkdir_reldev(yfs_dev, dp->fp->path, 0x777);
		if (result < 0)
		{
			return yaffsfs_GetLastError();
		}
	}

	yfs_fd->dirent = yaffs_opendir_reldev(yfs_dev, dp->fp->path);
	if (!yfs_fd->dirent)
	{
        os_free(yfs_fd);
		return yaffsfs_GetLastError();
	}
	dp->fp->desc = yfs_fd;

	return 0;
}

static int vfs_yaffs_closedir(struct vfs_dir *dp)
{
	struct yaffs_fd *yfs_fd;
	
	if ((!dp)|| (!dp->fp) || ((dp->fp->type != FT_REGULAR) && (dp->fp->type != FT_DIRECTORY)))
	{
		return -EINVAL;
	}

	if (!dp->fp->desc)
	{
		return -EINVAL;
	}

    yfs_fd = dp->fp->desc;
    
	if (yaffs_closedir(yfs_fd->dirent) < 0)
	{
	    os_free(yfs_fd);
	    dp->fp->desc = NULL;
		return yaffsfs_GetLastError();
	}
	
    os_free(yfs_fd);
	dp->fp->desc = NULL;
    
    return 0;
}

static int vfs_yaffs_readdir(struct vfs_dir *dp, struct dirent *dirp)
{
    struct dirent* d;
    struct yaffs_fd *yfs_fd;
    struct yaffs_dirent * yaffs_dp;
	
    if ((!dp->fp) || (!dp->fp->desc) || (!dirp) || (dp->fp->type != FT_DIRECTORY))
    {
        return -EINVAL;
    }
    
    yfs_fd = dp->fp->desc;
   
    d = dirp;

    yaffs_dp = yaffs_readdir(yfs_fd->dirent);
    if (!yaffs_dp)
    {
        return yaffsfs_GetLastError();
    }

    if (yaffs_dp->d_type == YAFFS_DT_DIR)
    {
        d->d_type = DT_DIR;
    }
    else if (yaffs_dp->d_type == YAFFS_DT_REG)
    {
        d->d_type = DT_REG;
    }
    else
    {
        d->d_type = DT_UNKNOWN;
    }

    strncpy(d->d_name, yaffs_dp->d_name, strlen(yaffs_dp->d_name) + 1);

    yfs_fd->pos += sizeof(struct dirent);
   
    return sizeof(struct dirent);
}

static int vfs_yaffs_seekdir(struct vfs_dir *dp, off_t offset)
{
	struct yaffs_fd *yfs_fd;
	struct yaffs_dirent * yaffs_d;
	int result = -1;
	int entry_num;

	if (!dp->fp)
	{
		return -EINVAL;
	}

    yfs_fd = dp->fp->desc;
    
	if (!yfs_fd)
	{
		return -EINVAL;
	}

    yaffs_rewinddir(yfs_fd->dirent);
    yfs_fd->pos = 0;
    entry_num = offset / sizeof(struct dirent);
    
    while (entry_num > 0)
    {
        yaffs_d = yaffs_readdir(yfs_fd->dirent);
        if (!yaffs_d)
        {
            return yaffsfs_GetLastError();
        }
        entry_num--;
        yfs_fd->pos += sizeof(struct dirent);
    }
    result = yfs_fd->pos;
        
	return result;
}

static long vfs_yaffs_telldir(struct vfs_dir *dp)
{
    struct yaffs_fd *yfs_fd;

    if (!dp->fp)
	{
		return -EINVAL;
	}

    yfs_fd = dp->fp->desc;
    
	if (!yfs_fd)
	{
		return -EINVAL;
	}
    
	return yfs_fd->pos;
}

static int vfs_yaffs_rename(struct vfs_mountpoint *fs, const char *oldpath, const char *newpath)
{
    int result;
    struct yaffs_dev *yfs_dev;
    struct yaffs_stat s;

    if ((!fs) || (!oldpath) || (!newpath))
    {
        return -EINVAL;
    }
    
    yfs_dev = (struct yaffs_dev *)fs->fs_instance;

    if (strcmp(oldpath, newpath) == 0)
    {
        /* If oldpath same as newpath, and path exist, return success. */
        if (yaffs_stat_reldev(yfs_dev, oldpath, &s) == 0)
        {
            return 0;
        }
    }

    result = yaffs_rename_reldev(yfs_dev, oldpath, newpath);
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    return 0;
}

static int vfs_yaffs_unlink(struct vfs_mountpoint *fs, const char *path)
{
    int result;
    struct yaffs_stat s;
    struct yaffs_dev *yfs_dev;

    if ((!fs) || (!fs->fs_instance) || (!path))
    {
        return -EINVAL;
    }
    yfs_dev = (struct yaffs_dev *)fs->fs_instance;

    if (yaffs_stat_reldev(yfs_dev, path, &s) < 0)
    {
        return yaffsfs_GetLastError();
    }

    switch (s.st_mode & S_IFMT)
    {
    case S_IFREG:
        result = yaffs_unlink_reldev(yfs_dev, path);
        break;
    case S_IFDIR:
        result = yaffs_rmdir_reldev(yfs_dev, path);
        break;
    default:
        return -1;
    }
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    return 0;
}

static int vfs_yaffs_stat(struct vfs_mountpoint *fs, const char *path, struct stat *st)
{
    int result;
    struct yaffs_stat s;
    struct yaffs_dev *yfs_dev;

    if ((!fs) || (!path) || (!st))
    {
        return -EINVAL;
    }
    yfs_dev = (struct yaffs_dev *)fs->fs_instance;

    result = yaffs_stat_reldev(yfs_dev, path, &s);
    if (result < 0)
    {
        return yaffsfs_GetLastError();
    }

    st->st_dev = 0;
    st->st_mode = s.st_mode;
    st->st_size = s.st_size;
    st->st_mtime = s.yst_mtime;

    return 0;
}


static int vfs_yaffs_mkfs(void* dev_id)
{
    if (!dev_id)
    {
        return -1;
    }

    return yaffs_format(device_name((os_device_t *)dev_id), 0, 0, 0);
}

static int vfs_yaffs_statfs(struct vfs_mountpoint *fs, struct statfs *buf)
{
    struct yaffs_dev *yfs_dev;
    struct yaffs_param *yfs_param;
    Y_LOFF_T freespace;

    if ((!fs) || (!fs->fs_instance) || (!buf))
    {
        return -EINVAL;
    }
    yfs_dev = (struct yaffs_dev *)fs->fs_instance;
    freespace = yaffs_freespace_reldev(yfs_dev);
    if (freespace < 0)
    {
        return -EINVAL;
    }
    yfs_param = &yfs_dev->param;
    buf->f_bsize = yfs_param->total_bytes_per_chunk * yfs_param->chunks_per_block;
    buf->f_blocks = yfs_param->end_block + 1;
    buf->f_bfree = freespace /buf->f_bsize;

    return 0;
}

static int vfs_yaffs_mount(struct vfs_mountpoint *fs, unsigned long rwflag, const void *data)
{
    os_device_t *os_dev;
    struct yaffs_dev *yfs_dev;

    if ((!fs) || (!fs->dev))
    {
        return -1;
    }

    os_dev = (os_device_t*)fs->dev;
    if (!os_dev)
    {
        return -1;
    }

    yfs_dev = yaffs_getdev(device_name(os_dev));
    if (!yfs_dev)
    {
        return -1;
    }
    if (yaffs_mount(device_name(os_dev)) < 0)
    {
        return yaffsfs_GetLastError();
    }
    fs->fs_instance = yfs_dev;

    return 0;
}

static int vfs_yaffs_unmount(struct vfs_mountpoint *fs)
{
    os_device_t *os_dev;

    if ((!fs) || (!fs->dev))
    {
        return -1;
    }

    os_dev = (os_device_t*)fs->dev;
    if (!os_dev)
    {
        return -1;
    }

    if (yaffs_unmount(device_name(os_dev)) < 0)
    {
        return yaffsfs_GetLastError();
    }
    fs->fs_instance = OS_NULL;

    return 0;
}


static const struct vfs_filesystem_ops _fsops =
{
    "yaffs",
    
	/* Operations for file */
	vfs_yaffs_open,
	vfs_yaffs_close,
	vfs_yaffs_read,
	vfs_yaffs_write,
	vfs_yaffs_lseek,
	vfs_yaffs_flush,
	OS_NULL,                              /* Not support ioctl. */
	OS_NULL,                              /* Not support poll. */

	/* Operations for dirctory */
	vfs_yaffs_opendir,
	vfs_yaffs_closedir,
	vfs_yaffs_readdir,
	vfs_yaffs_seekdir,
	vfs_yaffs_telldir,

	/* Operations for file or dircotry */
	vfs_yaffs_rename,
	vfs_yaffs_unlink,
	vfs_yaffs_stat,

	/* Operations for file or dircotry */
	vfs_yaffs_mkfs,                                 
	vfs_yaffs_statfs,
    vfs_yaffs_mount,
    vfs_yaffs_unmount,

};

/**
 ***********************************************************************************************************************
 * @brief           Register YAFFS operation structure to VFS.
 *
 * @param[in,out]   None.
 *
 * @return          The register result. 
 * @retval          0               Register successfully.
 * @retval          -1              Register failed.
 ***********************************************************************************************************************
 */
int yaffs_init(void)
{
	/* Register yaffs file system */
    return vfs_register(&_fsops);
}
OS_CMPOENT_INIT(yaffs_init,OS_INIT_SUBLEVEL_MIDDLE);


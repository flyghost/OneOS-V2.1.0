/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        vfs_nfs.c
 *
 * @brief       Adapter file between nfs and vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      Rafactor nfs code
 ***********************************************************************************************************************
 */


#include <stdio.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <dlog.h>
#include <vfs_fs.h>
#include <vfs.h>
#include <rpc/rpc.h>
#include <sys/fcntl.h>
#include "nfs.h"

#define NFS_TAG       "NFS"

#define HOLE_BUF_SIZE       1024

/**
 ***********************************************************************************************************************
 * @brief           Mount nfs filesystem.
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
static int vfs_nfs_mount(struct vfs_mountpoint *mnt_point, unsigned long mountflag, const void *data)
{
    struct nfs_filesystem *nfs = OS_NULL;

    if (!mnt_point)
    {
        return -EIO;
    }

    nfs = nfs_mount((char *)data);
	
    if (!nfs)
    {
        return -EIO;
    }
    mnt_point->fs_instance = nfs;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Unmount nfs filesystem.
 *
 * @param[in,out]   mnt_point       The pointer of VFS object.
 *
 * @return          The unmount result.
 * @retval          0               Unmount successfully.
 * @retval          -EIO            Unmount failed.
 ***********************************************************************************************************************
 */
static int vfs_nfs_unmount(struct vfs_mountpoint *mnt_point)
{
    struct nfs_filesystem *nfs;

    if ((!mnt_point) || (!mnt_point->fs_instance))
    {
        return -EIO;
    }

    nfs = mnt_point->fs_instance;
    if (nfs_unmount(nfs) < 0)
    {
        return -EIO;
    }
	
    mnt_point->fs_instance = OS_NULL;

    return 0;
}

int vfs_nfs_ioctl(struct vfs_file *fp, unsigned long cmd, void *args)
{
    return -ENOSYS;
}

/**
 ***********************************************************************************************************************
 * @brief           Read file.
 *
 * @param[in,out]   fp              The file pointer.
 * @param[out]      buf             The pointer of buffer to save read content.
 * @param[in]       size             The expected read size.
 *
 * @return          The actual read size.
 * @retval          positive int    The actual read size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_nfs_read(struct vfs_file *fp, void *buf, size_t size)
{
    struct nfs_fd *n_fd;
    struct vfs_mountpoint *mnt_point;
    struct nfs_filesystem *nfs;
    int read_size = 0;
    int file_size = 0;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc) || (!buf))
    {
        return -EINVAL;
    }

    if (fp->flags & O_WRONLY)
    {
        return -EINVAL;
    }
	
    if(size <= 0)
    {
        return 0;
    }

    mnt_point = fp->mnt_point;
    nfs = mnt_point->fs_instance;
    n_fd = (struct nfs_fd *)fp->desc;

    file_size = nfs_get_filesize(nfs, fp->path);
    if (n_fd->offset > file_size)
    {
        return -EINVAL;
    }
    size = (size > (file_size - n_fd->offset)) ? (file_size - n_fd->offset) : size;
    read_size = nfs_read(nfs, n_fd, buf, n_fd->offset, size);
	

    return read_size;
}

/**
 ***********************************************************************************************************************
 * @brief           Write file.
 *
 * @param[in,out]   fp              The file pointer.
 * @param[in]       buf             The pointer of data to write.
 * @param[in]       size             The expected write size.
 *
 * @return          The actual write size.
 * @retval          positive int    The actual write size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_nfs_write(struct vfs_file *fp, const void *buf, size_t size)
{
    struct nfs_fd *n_fd;
    struct vfs_mountpoint *mnt_point;
    struct nfs_filesystem *nfs;
    int write_size = 0;
    char *buf_hole;
    int size_hole;
    int size_temp;
    off_t pos;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc) || (!buf))
    {
        return -EINVAL;
    }
    if (fp->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }
    if ((!(fp->flags & O_RDWR)) && (!(fp->flags & O_WRONLY)))
    {
        return -EINVAL;
    }
    if(size <= 0)
    {
        return 0;
    }

    mnt_point = fp->mnt_point;
    nfs = mnt_point->fs_instance;
    n_fd = (struct nfs_fd *)fp->desc;

    /* If hole exist, fill hole to null value */
    if (n_fd->offset > n_fd->size)
    {
        size_hole = n_fd->offset - n_fd->size;
        pos = n_fd->size;
        size_temp = (size_hole > HOLE_BUF_SIZE) ? HOLE_BUF_SIZE : size_hole;
        buf_hole = os_malloc(size_temp);
        if (!buf_hole)
        {
            return -ENOMEM;
        }
        memset(buf_hole, 0, size_temp);
        while(size_hole > 0)
        {
            write_size = (size_hole > size_temp) ? size_temp : size_hole;
            write_size = nfs_write(nfs, n_fd, buf_hole, pos, write_size);
            if (write_size > 0)
            {
                size_hole -= write_size;
                pos += write_size;
            }
            else
            {
                os_free(buf_hole);
                return write_size;
            }
        }
        os_free(buf_hole);
    }

    /*Write actual data */
    write_size = nfs_write(nfs, n_fd, buf, n_fd->offset, size);

    return write_size;
}


/**
 ***********************************************************************************************************************
 * @brief           Reposition read/write file offset.
 *
 * @param[in,out]   fp              The file pointer.
 * @param[in]       offset          The new offset in file.
 * @param[in]       whence          SEEK_SET/SEEK_CUR/SEEK_END
 *
 * @return          The lseek result.
 * @retval          int             The new position in file.  
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_nfs_lseek(struct vfs_file *fp, off_t offset, int whence)
{
    struct nfs_fd *n_fd;
    off_t pos = 0;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc) || (fp->type != FT_REGULAR))
    {
        return -EINVAL;
    }

    n_fd = (struct nfs_fd *)fp->desc;

	switch (whence)
	{
	case SEEK_SET:
		pos = offset;
		break;
	case SEEK_CUR:
		pos = n_fd->offset + offset;
		break;
	case SEEK_END:
		pos = n_fd->size + offset;
		break;
	default:
		return -EINVAL;
	}

	if (pos < 0) {
		return -EINVAL;
	}


    nfs_seekfile((struct nfs_fd *)fp->desc, pos);

    return pos;
}

/**
 ***********************************************************************************************************************
 * @brief           Close file in nfs filesystem.
 *
 * @param[in,out]   fp              The file pointer.
 *
 * @return          The close result.
 * @retval          0               File close successfully.
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_nfs_close(struct vfs_file *fp)
{
    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance) || (!fp->desc))
    {
        return -EINVAL;
    }

    nfs_closefile((struct nfs_fd *)fp->desc);

    fp->desc = NULL;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Open file in nfs filesystem.
 *
 * @param[in,out]   fp              The file pointer.
 *
 * @return          The open result.
 * @retval          0               Open file successfully.
 * @retval          Other           Open failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_nfs_open(struct vfs_file *fp)
{
    struct nfs_filesystem *nfs;
    struct vfs_mountpoint *mnt_point;
    struct nfs_fd *n_fd;
    struct fattr3 *info;
    int result;

    if ((!fp) || (!fp->mnt_point) || (!fp->mnt_point->fs_instance))
    {
        return -EINVAL;
    }

    mnt_point = (struct vfs_mountpoint *)fp->mnt_point;
    nfs = (struct nfs_filesystem *)mnt_point->fs_instance;

    
    if (fp->flags & O_CREAT)
    {
        /* If O_EXCL specified in conjuncition with O_CREAT, and pathname already exists, then fails with EEXIST*/
        if (fp->flags & O_EXCL)
        {
            info = os_malloc(sizeof(struct fattr3));
            if (!info)
            {
                return -ENOMEM;
            }
			memset(info,0,sizeof(struct fattr3));
            result = nfs_stat(nfs, fp->path, info);
            if ((result == 0) && (info->type == NFS3REG) || (info->type == NFS3DIR))
            {
                os_free(info);
                return -EEXIST;
            }
            os_free(info);
        }
        if (nfs_mkfile(nfs, fp->path, 0664) < 0)
        {
            return -ENOENT;
        }
    }

    /*If the file already exists and the access mode allows writing, it will be truncated to length 0. */
    if ((fp->flags & O_TRUNC) && ((fp->flags & O_RDWR) || (fp->flags & O_WRONLY)))
    {
        info = os_malloc(sizeof(struct fattr3));
        if (!info)
        {
            return -ENOMEM;
        }
		memset(info,0,sizeof(struct fattr3));
        result = nfs_stat(nfs, fp->path, info);
        if ((result == 0) && (info->size > 0))
        {
            if (nfs_unlinkfile(nfs, fp->path) < 0)
            {
                os_free(info);
                return -EINVAL;
            }
            if (nfs_mkfile(nfs, fp->path, 0664) < 0)
            {
                os_free(info);
                return -ENOENT;
            }
        }
        os_free(info);
    }

    n_fd = nfs_openfile(nfs, fp->path, fp->flags);
    if (!n_fd)
    {
        return -ENOENT;
    }

    fp->desc = n_fd;

    return 0;
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
static int vfs_nfs_stat(struct vfs_mountpoint *mnt_point, const char *path, struct stat *st)
{
    struct nfs_filesystem *nfs;
    struct fattr3 *info;
    int result;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!path) || (!st))
    {
        return -EINVAL;
    }
    nfs = (struct nfs_filesystem *)mnt_point->fs_instance;

    info = os_malloc(sizeof(struct fattr3));
    if (!info)
    {
        return -ENOMEM;
    }
	memset(info,0,sizeof(struct fattr3));
    result = nfs_stat(nfs, path, info);
    if (result < 0)
    {
        os_free(info);
        return -ENOENT;
    }

    //st->st_dev = 0;
    st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                  S_IWUSR | S_IWGRP | S_IWOTH;
    if (info->type == NFS3DIR)
    {
        st->st_mode &= ~S_IFREG;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    }
    st->st_size = info->size;
    st->st_mtime = info->mtime.seconds;
    os_free(info);

    return 0;
}


/**
 ***********************************************************************************************************************
 * @brief           Get the nfs filesystem stat.
 *
 * @param[in]       mnt_point       The pointer of VFS object.
 * @param[out]      buf             The pointer of buf to save the cute filesystem stat.
 *
 * @return          The operation result.
 * @retval          0               Get stat successfully.
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_nfs_statfs(struct vfs_mountpoint *mnt_point, struct statfs *buf)
{
    struct nfs_filesystem *nfs;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!buf))
    {
        return -EINVAL;
    }
    nfs = (struct nfs_filesystem *)mnt_point->fs_instance;

    return nfs_statfs(nfs, buf);
}


/**
 ***********************************************************************************************************************
 * @brief           Delete dir entry from nfs filesystem.
 *
 * @param[in]       mnt_point       The pointer of VFS object.
 * @param[in]       path            The path (the file name) to be delete.
 *
 * @return          The delete result.
 * @retval          0               Delete successfully.
 * @retval          others          Delete fail, return err code.
 ***********************************************************************************************************************
 */
static int vfs_nfs_unlink(struct vfs_mountpoint *mnt_point, const char *path)
{
    struct nfs_filesystem *nfs;
    int result = 0;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!path))
    {
        return -EINVAL;
    }

    nfs = (struct nfs_filesystem *)mnt_point->fs_instance;

    if (nfs_get_type(nfs, path) == FT_REGULAR)
    {
        result = nfs_unlinkfile(nfs, path);
    }
    else
    {
        result = nfs_unlinkdir(nfs, path);
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Rename file/directory in nfs filesystem.
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
static int vfs_nfs_rename(struct vfs_mountpoint *mnt_point, const char *oldpath, const char *newpath)
{
    struct nfs_filesystem *nfs;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!oldpath) || (!newpath))
    {
        return -EINVAL;
    }
    nfs = (struct nfs_filesystem *)mnt_point->fs_instance;

    return nfs_rename(nfs, oldpath, newpath);
}

/**
 ***********************************************************************************************************************
 * @brief           open dir .
 *
 * @param[in,out]   dp              The descriptor of dir path.
 *
 * @return          The open result.
 * @retval          0               Open dir successfully.
 * @retval          Other           Open failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_nfs_opendir(struct vfs_dir *dp)
{
    struct nfs_filesystem *nfs;
    struct vfs_mountpoint *mnt_point;
    struct nfs_dir *dir;

    if ((!dp) || (!dp->fp) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        return -EINVAL;
    }

    mnt_point = (struct vfs_mountpoint *)dp->fp->mnt_point;
    nfs = (struct nfs_filesystem *)mnt_point->fs_instance;

    if (dp->fp->flags & O_CREAT)
    {
        if (nfs_mkdir(nfs, dp->fp->path, 0755) < 0)
        {
            return -ENOENT;
        }
    }

    dir = nfs_opendir(nfs, dp->fp->path);
    if (!dir)
    {
        return -ENOENT;
    }
    dp->fp->desc = dir;
    
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           close dir .
 *
 * @param[in,out]   dp              The descriptor of dir path.
 *
 * @return          The close result.
 * @retval          0               close dir successfully.
 * @retval          Other           close failed, return error code.
 ***********************************************************************************************************************
 */
static int vfs_nfs_closedir(struct vfs_dir *dp)
{
    if ((!dp) || (!dp->fp) || (!dp->fp->desc))
    {
        return -EINVAL;
    }

    nfs_closedir((struct nfs_dir *)dp->fp->desc);

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
static int vfs_nfs_readdir(struct vfs_dir *dp, struct dirent *dent)
{
    struct nfs_dir *dir;
    os_uint32_t index;
    struct dirent *d;
    struct vfs_mountpoint *mnt_point;
    struct nfs_filesystem *nfs;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance) || (!dent))
    {
        return -EINVAL;
    }

    mnt_point  = ((struct vfs_mountpoint *)(dp->fp->mnt_point));
    nfs = (struct nfs_filesystem *)(mnt_point->fs_instance);
    dir = (struct nfs_dir *)(dp->fp->desc);

    index = 0;
    d = dent;
    while (1)
    {
        if (nfs_readdir(nfs, dir, d) < 0)
        {
            break;
        }
        if ((strcmp(d->d_name, ".") == 0) || (strcmp(d->d_name, "..") == 0))
        {
            continue;
        }

        index ++;
        if (index > 0)
        {
            break;
        }
    }

    return index;
}

/**
 ***********************************************************************************************************************
 * @brief           Reposition read/write dir offset.
 *
 * @param[in,out]   fp              The dir pointer.
 * @param[in]       offset          The new offset in dir.
 *
 * @return          The seekdir result.
 * @retval          int             The new position in dir.  
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static int vfs_nfs_seekdir(struct vfs_dir *dp, off_t offset)
{
    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (dp->fp->type != FT_DIRECTORY))
    {
        return -EINVAL;
    }

    nfs_seekdir((struct nfs_dir *)dp->fp->desc, offset);

    return offset;
}

/**
 ***********************************************************************************************************************
 * @brief           tell dir offset.
 *
 * @param[in,out]   fp              The dir pointer.
 *
 * @return          The dir offset.
 * @retval          long            offset of dir.  
 * @retval          -EINVAL         Invalid parameter.
 ***********************************************************************************************************************
 */
static long vfs_nfs_telldir(struct vfs_dir *dp)
{
    off_t pos = 0;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc))
    {
        return -EINVAL;
    }

    pos = nfs_telldir((struct nfs_dir *)dp->fp->desc);
    return pos;
}


static const struct vfs_filesystem_ops vfs_nfs =
{
    "nfs",

    vfs_nfs_open,
    vfs_nfs_close,
    vfs_nfs_read,
    vfs_nfs_write,
    vfs_nfs_lseek,
    NULL, /* sync */    
    vfs_nfs_ioctl,
#ifdef OS_USING_IO_MULTIPLEXING
    NULL, /* poll */
#endif

    vfs_nfs_opendir,
    vfs_nfs_closedir,
    vfs_nfs_readdir,
    vfs_nfs_seekdir,
    vfs_nfs_telldir,

    vfs_nfs_rename,
    vfs_nfs_unlink,
    vfs_nfs_stat,
    
    NULL, /* mkfs */
    vfs_nfs_statfs, /* statfs */	
    vfs_nfs_mount,
    vfs_nfs_unmount,

};


/**
 ***********************************************************************************************************************
 * @brief           Register nfs operation structure to VFS.
 *
 * @param[in,out]   None.
 *
 * @return          The register result. 
 * @retval          0               Register successfully.
 * @retval          -1              Register failed.
 ***********************************************************************************************************************
 */
int nfs_init(void)
{
    /* register nfs file system */
    return vfs_register(&vfs_nfs);
}
OS_CMPOENT_INIT(nfs_init, OS_INIT_SUBLEVEL_MIDDLE);


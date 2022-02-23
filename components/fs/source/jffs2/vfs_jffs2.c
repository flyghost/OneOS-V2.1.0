/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
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
 * @file        vfs_jffs2.c
 *
 * @brief       This file is adapter for Vitual filesystem and jffs2 filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-29   OneOS Team      Modify some code to compliant with the posix syntax and optimize some feature.
 ***********************************************************************************************************************
 */
 
#include <vfs.h>
#include <device.h>
#include "cyg/infra/cyg_type.h"
#include "cyg/fileio/fileio.h"
#include "port/codes.h"
#include "port/fcntl.h"
#undef mode_t

#include <vfs_fs.h>
#include <vfs.h>
#include "vfs_jffs2.h"
#include "jffs2_config.h"
#include "porting.h"
#include <string.h>


#if DEVICE_PART_MAX > 1
    #error "support only one jffs2 partition on a flash device!"
#endif

struct device_part
{
    struct cyg_mtab_entry *mte;
    struct os_device      *dev;
};
static struct device_part device_partition[DEVICE_PART_MAX] = {0};
static struct os_mutex jffs2_lock;

#define jffs2_mount         jffs2_fste.mount
#define jffs2_umount        jffs2_fste.umount
#define jffs2_open          jffs2_fste.open
#define jffs2_file_unlink   jffs2_fste.unlink
#define jffs2_mkdir         jffs2_fste.mkdir
#define jffs2_rmdir         jffs2_fste.rmdir
#define jffs2_rename        jffs2_fste.rename
#define jffs2_link          jffs2_fste.link
#define jffs2_opendir       jffs2_fste.opendir
#define jffs2_chdir         jffs2_fste.chdir
#define jffs2_ops_stat      jffs2_fste.stat
#define jffs2_getinfo       jffs2_fste.getinfo
#define jffs2_setinfo       jffs2_fste.setinfo

#define jffs2_file_read     jffs2_fileops.fo_read
#define jffs2_file_write    jffs2_fileops.fo_write
#define jffs2_file_lseek    jffs2_fileops.fo_lseek
#define jffs2_file_ioctl    jffs2_fileops.fo_ioctl
#define jffs2_file_select   jffs2_fileops.fo_select
#define jffs2_file_fsync    jffs2_fileops.fo_fsync
#define jffs2_file_colse    jffs2_fileops.fo_close
#define jffs2_file_fstat    jffs2_fileops.fo_fstat
#define jffs2_file_getinfo  jffs2_fileops.fo_getinfo
#define jffs2_file_setinfo  jffs2_fileops.fo_setinfo

#define jffs2_dir_read      jffs2_dirops.fo_read
//#define jffs2_dir_write   jffs2_dirops.fo_write
#define jffs2_dir_lseek     jffs2_dirops.fo_lseek
//#define jffs2_dir_ioctl   jffs2_dirops.fo_ioctl
#define jffs2_dir_select    jffs2_dirops.fo_select
//#define jffs2_dir_fsync   jffs2_dirops.fo_fsync
#define jffs2_dir_colse     jffs2_dirops.fo_close
//#define jffs2_dir_fstat   jffs2_dirops.fo_fstat
//#define jffs2_dir_getinfo jffs2_dirops.fo_getinfo
//#define jffs2_dir_setinfo jffs2_dirops.fo_setinfo

extern int jffs2_flash_erase_all(os_device_t *dev_id);

static int jffs2_result_to_vfs(int result)
{
    if (result < 0)
    {
        return  result;
    }
    
    if (result > 0)
    {
        return -result;
    }

    return 0;
}

static int vfs_jffs2_mount(struct vfs_mountpoint* mnt_point, unsigned long mountflag, const void *data)
{
    unsigned index;
    int      result;
    struct cyg_mtab_entry *mte;

    if (!mnt_point && mnt_point->dev)
    {
        return -EINVAL;
    }
    if (os_device_open((os_device_t *)mnt_point->dev) != OS_EOK)
    {
        return -EINVAL;
    }

    /* Find a empty entry in partition table */
    for (index = 0; index < DEVICE_PART_MAX; index++)
    {
        if (device_partition[index].dev == OS_NULL)
        {
            break;
        }
    }

    if (index == DEVICE_PART_MAX)
    {
        return -ENOSPC;
    }

    mte = os_malloc(sizeof(struct cyg_mtab_entry));
    if (mte == OS_NULL)
    {
        return -ENOMEM;
    }

    mte->name = mnt_point->mnt_path;
    mte->fsname = "jffs2";
    mte->devname = NULL;
    mte->data = (CYG_ADDRWORD)mnt_point->dev;

    device_partition[index].dev = (os_device_t *)mnt_point->dev;

    /* After jffs2_mount, mte->data will not be dev_id anymore */
    result = jffs2_mount(NULL, mte);
    if (result != 0)
    {
        device_partition[index].dev = NULL;
        return jffs2_result_to_vfs(result);
    }

    device_partition[index].mte = mte;
    return 0;
}

static int _find_fs(struct cyg_mtab_entry **mte, os_device_t *dev_id)
{
    unsigned index;

    /* Find device index */
    for (index = 0; index < DEVICE_PART_MAX; index++)
    {
        if (device_partition[index].dev == dev_id)
        {
            *mte = device_partition[index].mte;
            return 0;
        }
    }

    os_kprintf("error, could not found the fs!");
    return -1;
}

static int vfs_jffs2_unmount(struct vfs_mountpoint *mnt_point)
{
    int      result;
    unsigned index;

    if (!mnt_point)
    {
        return -EIO;
    }

    /* Find device index, then umount it */
    for (index = 0; index < DEVICE_PART_MAX; index++)
    {
        if (device_partition[index].dev == (os_device_t *)mnt_point->dev)
        {
            result = jffs2_umount(device_partition[index].mte);
            if (result) 
            {
                return jffs2_result_to_vfs(result);
            }

            os_free(device_partition[index].mte);
            device_partition[index].dev = NULL;
            device_partition[index].mte = NULL;
            return OS_EOK;
        }
    }

    return -ENOENT;
}

static int vfs_jffs2_mkfs(void *dev_id)
{
    unsigned index;

    OS_ASSERT(dev_id != OS_NULL);
    
    /* Find device index, if dev exsit,return error */
    for (index = 0; index < DEVICE_PART_MAX; index++)
    {
        if (device_partition[index].dev == (os_device_t *)dev_id)
        {
            return -EPERM;
        }
    }

    /* erase all blocks on this partition */
    return jffs2_flash_erase_all((os_device_t *)dev_id);
}

static int vfs_jffs2_statfs(struct vfs_mountpoint *mnt, struct statfs *buf)
{
    struct cyg_mtab_entry *mte;
    struct jffs2_fs_info info;
    int result;

    result = _find_fs(&mte, mnt->dev);
    if (result)
    {
        return -ENOENT;
    }

    OS_ASSERT(mte->data != 0);

    jffs2_get_info_from_sb((void *)mte->data, &info);
    buf->f_bsize = info.sector_size;
    buf->f_blocks = info.nr_blocks;
    buf->f_bfree = info.free_size / info.sector_size;

    return 0;
}

static const char jffs2_root_path[] = ".";

static int vfs_jffs2_open(struct vfs_file *fp)
{
    int result;
    int oflag;
    int mode;
    int pos;
    int size;
    const char *name;
    cyg_file   *jffs2_file;
    struct vfs_mountpoint *mnt_point;
    struct cyg_mtab_entry *mte;

    oflag = fp->flags;
    mnt_point = fp->mnt_point;
    OS_ASSERT(mnt_point != OS_NULL);

    jffs2_file = os_malloc(sizeof(cyg_file));
    if (jffs2_file == OS_NULL)
    {
        return -ENOMEM;
    }

    /* Escape  the '/' provided by vfs code */
    name = fp->path;
    if ((name[0] == '/') && (name[1] == 0))
    {
        name = jffs2_root_path;
    }
    else 
    {
        /* name[0] still will be '/' */
        name++;
    }

    result = _find_fs(&mte, (os_device_t *)mnt_point->dev);
    if (result)
    {
        os_free(jffs2_file);
        return -ENOENT;
    }

    /* Set mount table */
    jffs2_file->f_mte = mte;
    
    /* Handle regular file operations */
    mode = JFFS2_O_RDONLY;
    if (oflag & O_WRONLY)
    {
        mode |= JFFS2_O_WRONLY;
    }
    if (oflag & O_RDWR)
    {
        mode |= JFFS2_O_RDWR;
    }
    
    /* Opens the file, if it is existing. If not, a new file is created. */
    if (oflag & O_CREAT)
    {
        mode |= JFFS2_O_CREAT;
    }
    /* Creates a new file. If the file is existing, it is truncated and overwritten. */
    if (oflag & O_TRUNC)
    {
        mode |= JFFS2_O_TRUNC;
    }
    /* Creates a new file. The function fails if the file is already existing. */
    if (oflag & O_EXCL)
    {
        mode |= JFFS2_O_EXCL;
    }

    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_open(mte, 0, name, mode, jffs2_file);
    if (result != 0)
    {
        os_mutex_unlock(&jffs2_lock);
        os_free(jffs2_file);
        return jffs2_result_to_vfs(result);
    }

    /* Save this pointer, it will be used when calling read(), write(),
    flush(), lessk(), and will be os_free when calling close()*/
    fp->desc = jffs2_file;
    pos = jffs2_file->f_offset;
    size = 0;
    jffs2_file_lseek(jffs2_file, (off_t *)(&size), SEEK_END);
    jffs2_file->f_offset = (off_t)pos;
    os_mutex_unlock(&jffs2_lock);

    if (oflag & O_APPEND)
    {
        pos = size;
        jffs2_file->f_offset = size;
    }

    return 0;
}

static int vfs_jffs2_opendir(struct vfs_dir *dp)
{
    int result;
    int oflag;
    const char *name;
    cyg_file   *jffs2_file;
    struct vfs_mountpoint *mnt_point;
    struct cyg_mtab_entry *mte;

    struct vfs_file *fp = dp->fp;
    oflag = fp->flags;
    mnt_point = fp->mnt_point;
    OS_ASSERT(mnt_point != OS_NULL);

    jffs2_file = os_malloc(sizeof(cyg_file));
    if (jffs2_file == OS_NULL)
    {
        return -ENOMEM;
    }

    /* Escape  the '/' provided by vfs code */
    name = fp->path;
    if ((name[0] == '/') && (name[1] == 0))
    {
        name = jffs2_root_path;
    }
    else
    {
        /* name[0] still will be '/' */
        name++;
    }

    result = _find_fs(&mte, (os_device_t *)mnt_point->dev);
    if (result)
    {
        os_free(jffs2_file);
        return -ENOENT;
    }

    /* Set mount table */
    jffs2_file->f_mte = mte;

    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);

     /* Create a dir*/
    if (oflag & O_CREAT)
    {
        result = jffs2_mkdir(mte, mte->root, name);
        if (result)
        {
            os_mutex_unlock(&jffs2_lock);
            os_free(jffs2_file);
            return jffs2_result_to_vfs(result);
        }
    }

    /* Open dir */
    result = jffs2_opendir(mte, mte->root, name, jffs2_file);
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        os_free(jffs2_file);
        return jffs2_result_to_vfs(result);
    }
#ifdef  CONFIG_JFFS2_NO_RELATIVEDIR
    jffs2_file->f_offset = 2;
#endif
    /* Save this pointer, it will be used by vfs_jffs2_getdents*/
    fp->desc = jffs2_file;
    return 0;
}

static int vfs_jffs2_close(struct vfs_file *fp)
{
    int       result;
    cyg_file *jffs2_file;

    OS_ASSERT(fp->desc != NULL);
    jffs2_file = (cyg_file *)(fp->desc);

    /* Handle regular file  */
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_file_colse(jffs2_file);
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }

    /* Release memory */
    os_free(jffs2_file);
    return 0;
}

static int vfs_jffs2_closedir(struct vfs_dir *dp)
{
    int       result;
    cyg_file *jffs2_file;

    OS_ASSERT(dp->fp->desc != NULL);
    jffs2_file = (cyg_file *)(dp->fp->desc);

    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_dir_colse(jffs2_file);
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }

    os_free(jffs2_file);
    return 0;
}

static int vfs_jffs2_ioctl(struct vfs_file *fp, unsigned long cmd, void *args)
{
    return -ENOSYS;
}

static int vfs_jffs2_read(struct vfs_file *fp, void *buf, size_t len)
{
    int       char_read;
    int       result;
    cyg_file *jffs2_file;
    struct CYG_UIO_TAG   uio_s;
    struct CYG_IOVEC_TAG iovec;

    OS_ASSERT(fp->desc != NULL);

    if (fp->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }
    
    if (fp->flags & O_WRONLY)
    {
        return -EINVAL;
    }
    
    jffs2_file = (cyg_file *)(fp->desc);
    uio_s.uio_iov = &iovec;
    uio_s.uio_iov->iov_base = buf;
    uio_s.uio_iov->iov_len = len;
    /* Support only 1 */
    uio_s.uio_iovcnt = 1;
    uio_s.uio_resid = uio_s.uio_iov->iov_len;

    /* The current offset */
    char_read = jffs2_file->f_offset; 
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_file_read(jffs2_file, &uio_s);
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }

    /* Update position */
    //file->pos = jffs2_file->f_offset;
    char_read = jffs2_file->f_offset - char_read;
    return char_read;
}

static int vfs_jffs2_readdir(struct vfs_dir *dp, struct dirent *dentry)
{
    struct dirent *d;
    cyg_file      *jffs2_file;
    struct CYG_UIO_TAG   uio_s;
    struct CYG_IOVEC_TAG iovec;
    struct dirent  jffs2_d;
#if !defined (CYGPKG_FS_JFFS2_RET_DIRENT_DTYPE)
    struct jffs2_stat s;
    cyg_mtab_entry   *mte;
    char             *fullname;
#endif
    int result;

    OS_ASSERT(dp->fp->desc != OS_NULL);
    jffs2_file = (cyg_file *)(dp->fp->desc);
    mte = jffs2_file->f_mte;

    memset(&jffs2_d, 0, sizeof(struct dirent));
    /* Set CYG_UIO_TAG uio_s */
    uio_s.uio_iov = &iovec;
    uio_s.uio_iov->iov_base = &jffs2_d;
    uio_s.uio_iov->iov_len = sizeof(struct dirent);
    /* Must be 1 */
    uio_s.uio_iovcnt = 1;
    uio_s.uio_offset = 0;
    uio_s.uio_resid = uio_s.uio_iov->iov_len;

    d = dentry;
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_dir_read(jffs2_file, &uio_s);

    os_mutex_unlock(&jffs2_lock);
    /* If met a error or all entry are read over, break while*/
    if (result || jffs2_d.d_name[0] == 0)
    {
        return jffs2_result_to_vfs(result);
    }

#if defined (CYGPKG_FS_JFFS2_RET_DIRENT_DTYPE)
    switch(jffs2_d.d_type & JFFS2_S_IFMT)
    {
    case JFFS2_S_IFREG:
        d->d_type = DT_REG;
        break;
    case JFFS2_S_IFDIR:
        d->d_type = DT_DIR;
        break;
    default:
        d->d_type = DT_UNKNOWN;
        break;
    }
#else
    fullname = os_malloc(FILE_PATH_MAX);
    if(fullname == OS_NULL)
    {
        return -ENOMEM;
    }

    /* Make a right entry */
    if ((dp->fp->path[0] == '/') )
    {
        if (dp->fp->path[1] == 0)
        {
            strcpy(fullname, jffs2_d.d_name);
        }
        else
        {
            os_snprintf(fullname, FILE_PATH_MAX, "%s/%s", dp->fp->path+1, jffs2_d.d_name);
        }
    }
    else
    {
        os_snprintf(fullname, FILE_PATH_MAX, "%s/%s", dp->fp->path, jffs2_d.d_name);
    }
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);

    result = jffs2_porting_stat(mte, mte->root, fullname, (void *)&s);

    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }

    os_free(fullname);
    /* Convert to vfs stat structure */
    switch(s.st_mode & JFFS2_S_IFMT)
    {
    case JFFS2_S_IFREG:
        d->d_type = DT_REG;
        break;
    case JFFS2_S_IFDIR:
        d->d_type = DT_DIR;
        break;
    default:
        d->d_type = DT_UNKNOWN;
        break;
    }
#endif
    /* Write the rest fields of struct dirent* dirp  */
    strncpy(d->d_name, jffs2_d.d_name, strlen(jffs2_d.d_name) + 1);

    if (result)
    {
        return jffs2_result_to_vfs(result);
    }

    return sizeof(struct dirent);
}

static int vfs_jffs2_seekdir(struct vfs_dir *dp, off_t offset)
{
    cyg_file      *jffs2_file;

    OS_ASSERT(dp->fp->desc != OS_NULL);
    if (offset < 0)
    {
        os_kprintf("error, offset can't be a negative value\n");
        return EINVAL;
    }

    jffs2_file = (cyg_file *)(dp->fp->desc);
    jffs2_file->f_offset = offset;

    return ENOERR;
}

static long vfs_jffs2_telldir(struct vfs_dir *dp)
{
    cyg_file      *jffs2_file;
    OS_ASSERT(dp->fp->desc != OS_NULL);

    jffs2_file = (cyg_file *)(dp->fp->desc);

    return jffs2_file->f_offset;
}

static int vfs_jffs2_write(struct vfs_file *fp, const void *buf, size_t len)
{
    int       char_write;
    int       result;
    cyg_file *jffs2_file;
    struct CYG_UIO_TAG   uio_s;
    struct CYG_IOVEC_TAG iovec;

    OS_ASSERT(fp->desc != NULL);

    if (fp->type == FT_DIRECTORY)
    {
        return -EISDIR;
    }
    
    if ((!(fp->flags & O_RDWR)) && (!(fp->flags & O_WRONLY)))
    {
        return -EINVAL;
    }
    
    jffs2_file = (cyg_file *)(fp->desc);
    uio_s.uio_iov = &iovec;
    uio_s.uio_iov->iov_base = (void *)buf;
    uio_s.uio_iov->iov_len = len;
    /* Must be 1 */
    uio_s.uio_iovcnt = 1; 
    uio_s.uio_resid = uio_s.uio_iov->iov_len;

    char_write = jffs2_file->f_offset;
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_file_write(jffs2_file, &uio_s);
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }

    /* Update position */
    char_write = jffs2_file->f_offset - char_write;
    return char_write;
}

static int vfs_jffs2_flush(struct vfs_file *fp)
{
    return -ENOSYS;
}

static int vfs_jffs2_lseek(struct vfs_file *fp, os_off_t offset, int whence)
{
    cyg_file *jffs2_file;
    int       result;

    OS_ASSERT(fp->desc != NULL);
    jffs2_file = (cyg_file *)(fp->desc);

#ifdef  CONFIG_JFFS2_NO_RELATIVEDIR
    if ((fp->flags & O_DIRECTORY) && (0 == offset))
    {
        jffs2_file->f_offset = 2;
        return 0;
    }
#endif

    /* Set offset as current offset */
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_file_lseek(jffs2_file, &offset, whence);
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }
    /* Update file position */
    return offset;
}

static int vfs_jffs2_unlink(struct vfs_mountpoint *mnt, const char *path)
{
    int result;
    struct jffs2_stat s;
    cyg_mtab_entry   *mte;

    result = _find_fs(&mte, mnt->dev);
    if (result)
    {
        return -ENOENT;
    }

    /* Skip starting '/' */
    if (path[0] == '/')
    {
        path++;
    }

    /* Judge file type, dir is to be delete by rmdir, others by unlink */
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_porting_stat(mte, mte->root, path, (void *)&s);
    if (result)
    {
        os_mutex_unlock(&jffs2_lock);
        return jffs2_result_to_vfs(result);
    }

    switch(s.st_mode & JFFS2_S_IFMT)
    {
    case JFFS2_S_IFREG:
        result = jffs2_file_unlink(mte, mte->root, path);
        break;
    case JFFS2_S_IFDIR:
        result = jffs2_rmdir(mte, mte->root, path);
        break;
    default:
        /* Unknown file type */
        os_mutex_unlock(&jffs2_lock);
        return -1;
    }
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }
    return 0;
}

static int vfs_jffs2_rename_check(struct vfs_mountpoint *mnt, const char *oldpath, const char *newpath)
{
    int result;
    cyg_mtab_entry   *mte;
    struct jffs2_stat old_state;
    struct jffs2_stat new_state;
    cyg_file         *jffs2_file;
    struct CYG_UIO_TAG   uio_s;
    struct CYG_IOVEC_TAG iovec;
    struct dirent  jffs2_d;

    result = _find_fs(&mte, mnt->dev);
    if (result)
    {
        return -EINVAL;
    }
    
    result = jffs2_porting_stat(mte, mte->root, oldpath, (void *)&old_state);
    if (result)
    {
        return -EINVAL;
    }

    /* Only check dir */
    if (JFFS2_S_IFDIR != (old_state.st_mode & JFFS2_S_IFMT))
    {
        return 0;
    }

    /* The new pathname shall not contain a path prefix that names old. */
    if (strlen(newpath) > strlen(oldpath))
    {
        if (strncmp(newpath, oldpath, strlen(oldpath)) == 0)
        {
            if ('/' == newpath[strlen(oldpath)])
            {
                return -EINVAL;
            }
        }
    }

    result = jffs2_porting_stat(mte, mte->root, newpath, (void *)&new_state);
    if (0 == result)
    { 
        /* If new pathname is a dirctory, check whether is empty */
        if (JFFS2_S_IFDIR == (new_state.st_mode & JFFS2_S_IFMT))
        {   
            jffs2_file = os_malloc(sizeof(cyg_file));
            if (jffs2_file == OS_NULL)
            {
                return -ENOMEM;
            }

            jffs2_file->f_mte = mte;
            
            result = jffs2_opendir(mte, mte->root, newpath, jffs2_file);
            if (result)
            {
                os_free(jffs2_file);
                return -EINVAL;
            }

#ifdef  CONFIG_JFFS2_NO_RELATIVEDIR
            jffs2_file->f_offset = 2;
#endif

            memset(&jffs2_d, 0, sizeof(struct dirent));
            /* Set CYG_UIO_TAG uio_s */
            uio_s.uio_iov = &iovec;
            uio_s.uio_iov->iov_base = &jffs2_d;
            uio_s.uio_iov->iov_len = sizeof(struct dirent);
            /* Must be 1 */
            uio_s.uio_iovcnt = 1; 
            uio_s.uio_offset = 0;
            uio_s.uio_resid = uio_s.uio_iov->iov_len;

            result = jffs2_dir_read(jffs2_file, &uio_s);

            jffs2_file_colse(jffs2_file);
            os_free(jffs2_file);

            if ((result) || (0 == jffs2_d.d_name[0]))
            {
                return 0;
            }
            else
            {
                return -EINVAL;
            }
        } 
    }

    return 0;
}

static int vfs_jffs2_rename(struct vfs_mountpoint *mnt_point, const char *oldpath, const char *newpath)
{
    int result;
    cyg_mtab_entry *mte;

    result = _find_fs(&mte, (os_device_t *)mnt_point->dev);
    if (result)
    {
        return -ENOENT;
    }

    if (*oldpath == '/')
    {
        oldpath += 1;
    }
    
    if (*newpath == '/')
    {
        newpath += 1;
    }
    
    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);

    result = vfs_jffs2_rename_check(mnt_point, oldpath, newpath);

    if (result)
    {
        os_mutex_unlock(&jffs2_lock);
        return jffs2_result_to_vfs(result);
    }

    result = jffs2_rename(mte, mte->root, oldpath, mte->root, newpath);
    os_mutex_unlock(&jffs2_lock);
    if (result)
    {
        return jffs2_result_to_vfs(result);
    }
    return 0;
}

static int vfs_jffs2_stat(struct vfs_mountpoint *mnt, const char *path, struct stat *st)
{
    int result;
    struct jffs2_stat s;
    cyg_mtab_entry   *mte;

    /* Deal the root path for jffs2 */
    if (path[0] == '/' && path[1] == '\0')
    {
        st->st_mode  = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH | 
                        S_IXUSR | S_IXGRP | S_IXOTH| S_IFDIR;
        st->st_dev  = 0;
        st->st_size  = 0;
        st->st_mtime = 0;
        return 0;
    }

    if (path[0] == '/')
    {
        path++;
    }

    result = _find_fs(&mte, mnt->dev);
    if (result)
    {
        return -ENOENT;
    }

    os_mutex_lock(&jffs2_lock, OS_WAIT_FOREVER);
    result = jffs2_porting_stat(mte, mte->root, path, (void *)&s);
    os_mutex_unlock(&jffs2_lock);

    if (result)
    {
        return jffs2_result_to_vfs(result);
    }

    /* Convert to vfs stat structure */
    switch(s.st_mode & JFFS2_S_IFMT)
    {
    case JFFS2_S_IFREG:
        st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                      S_IWUSR | S_IWGRP | S_IWOTH;
        break;

    case JFFS2_S_IFDIR:
        st->st_mode = S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
        break;

    default:
        st->st_mode = DT_UNKNOWN;
        break;
    }

    st->st_dev  = 0;
    st->st_size = s.st_size;
    st->st_mtime = s.st_mtime;

    return 0;
}

static const struct vfs_filesystem_ops _jffs2_fops =
{
    "jffs2",

    vfs_jffs2_open,
    vfs_jffs2_close,
    vfs_jffs2_read,
    vfs_jffs2_write,
    vfs_jffs2_lseek,
    vfs_jffs2_flush,
    vfs_jffs2_ioctl,
#ifdef OS_USING_IO_MULTIPLEXING
    OS_NULL,                /* Not support poll. */
#endif
    vfs_jffs2_opendir,
    vfs_jffs2_closedir,
    vfs_jffs2_readdir,
    vfs_jffs2_seekdir,
    vfs_jffs2_telldir,

    vfs_jffs2_rename,
    vfs_jffs2_unlink,
    vfs_jffs2_stat,

    vfs_jffs2_mkfs,
    vfs_jffs2_statfs,
    vfs_jffs2_mount,
    vfs_jffs2_unmount,
};

/**
 ***********************************************************************************************************************
 * @brief           This function do jffs2 file system initialization work. 
 *
 * @param[]         None
 *
 * @return          Return 0 on successful, -1 on failed.
 ***********************************************************************************************************************
 */
int vfs_jffs2_init(void)
{
    /* Register jffs2 file system */
    vfs_register(&_jffs2_fops);

    /* Initialize mutex */
    if (os_mutex_init(&jffs2_lock, "jffs2lock", OS_FALSE) != OS_EOK)
    {
        os_kprintf("init jffs2 lock mutex failed\r\n");
    }
    os_kprintf("init jffs2 lock mutex okay\r\n");
    return 0;
}
OS_CMPOENT_INIT(vfs_jffs2_init, OS_INIT_SUBLEVEL_MIDDLE);


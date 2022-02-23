#include <vfs.h>
#include <device.h>
#include <vfs_fs.h>
#include <fal/fal_part.h>
#include <os_errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlog.h>
#include <os_mutex.h>
#include "lfs.h"
#include "lfs_io.h"

#define LFS_FS_TAG "littlefs"

#define LFS_ENTER_OPEN_LIST_CRITICAL(part_info)        os_mutex_lock(&part_info->open_lock, OS_WAIT_FOREVER)
#define LFS_LEAVE_OPEN_LIST_CRITICAL(part_info)        os_mutex_unlock(&part_info->open_lock)

struct lfs_part_info
{
    lfs_device_info_t      dev_info;
    lfs_t                  lfs;
    struct lfs_config      config;
    os_list_node_t         open_entry_head;
    os_mutex_t             open_lock;
};

typedef struct lfs_part_info lfs_part_info_t;

struct lfs_open_entry
{
    os_list_node_t  list_node;
    char           *name;
};

typedef struct lfs_open_entry lfs_open_entry_t;

extern const struct lfs_device_ops g_lfs_dev_ops;

static int lfs_ret_to_vfs(int val)
{
    int err = 0;

    switch (val)
    {
    case LFS_ERR_OK:
        err = 0;
        break;

    case LFS_ERR_IO:
        err = -EIO;
        break;

    case LFS_ERR_CORRUPT:
        err = -EILSEQ;
        break;

    case LFS_ERR_NOENT:
        err = -ENOENT;
        break;

    case LFS_ERR_EXIST:
        err = -EEXIST;
        break;

    case LFS_ERR_NOTDIR:
        err = -ENOTDIR;
        break;

    case LFS_ERR_ISDIR:
        err = -EISDIR;
        break;

    case LFS_ERR_NOTEMPTY:
        err = -ENOTEMPTY;
        break;

    case LFS_ERR_BADF:
        err = -EBADF;
        break;

    case LFS_ERR_FBIG:
        err = -EFBIG;
        break;

    case LFS_ERR_INVAL:
        err = -EINVAL;
        break;

    case LFS_ERR_NOSPC:
        err = -ENOSPC;
        break;

    case LFS_ERR_NOMEM:
        err = -ENOMEM;
        break;

#ifdef LFS_ATTR
    case LFS_ERR_NOATTR:
        err = -ENODATA;
        break;
#endif

    case LFS_ERR_NAMETOOLONG:
        err = -ENAMETOOLONG;
        break;

    default:
        err = -1;
        break;
    }

    return err;
}

static int vfs_lfs_cfg_init(lfs_part_info_t *part_info, os_device_t *dev)
{
    fal_part_t *part = fal_part_find(dev->name);
//    lfs_size_t  lookahead_size;
    int         ret;

    if (OS_NULL == part)
    {
        ret = -ENODEV;
    }
    else
    {
        memset(&part_info->lfs, 0, sizeof(lfs_t));
        memset(&part_info->config, 0 ,sizeof(struct lfs_config));

        part_info->dev_info.dev    = (void *)part;

        part_info->config.context = &part_info->dev_info;
        part_info->config.read    = g_lfs_dev_ops.read;
        part_info->config.prog    = g_lfs_dev_ops.prog;
        part_info->config.erase   = g_lfs_dev_ops.erase;
        part_info->config.sync    = g_lfs_dev_ops.sync;
#ifdef LFS_THREADSAFE
        part_info->config.lock    = g_lfs_dev_ops.lock;
        part_info->config.unlock  = g_lfs_dev_ops.unlock;
#endif

        part_info->config.read_size        = part->flash->page_size;
        part_info->config.prog_size        = part->flash->page_size;
        part_info->config.block_size       = part->flash->block_size;
        part_info->config.block_count      = part->info->size / part->flash->block_size;
        part_info->config.block_cycles     = -1;
        part_info->config.cache_size       = part->flash->page_size;

        part_info->config.lookahead_size   = LFS_LOOKAHEAD_SIZE;
 
        part_info->config.read_buffer      = OS_NULL;
        part_info->config.prog_buffer      = OS_NULL;
        part_info->config.lookahead_buffer = OS_NULL;

        ret = 0;
    }

    return ret;
}

static lfs_part_info_t *vfs_lfs_part_info_init(os_device_t* dev)
{
    char             name[OS_NAME_MAX];
    lfs_part_info_t *part_info;
    lfs_part_info_t *ret;

    part_info = os_malloc(sizeof(lfs_part_info_t));

    if (!part_info)
    {
        ret = OS_NULL;
    }
    else
    {
        os_list_init(&part_info->open_entry_head);

        if (os_snprintf(name, sizeof(name), "lfs%p", part_info) < 0)
        {
            ret = OS_NULL;
            os_free(part_info);
        }
        else
        {
            if (OS_EOK == os_mutex_init(&part_info->open_lock, name, OS_FALSE))
            {
                if (OS_EOK == os_mutex_init(&part_info->dev_info.dev_lock, OS_NULL, OS_FALSE))
                {
                    ret = part_info;
                }
                else
                {
                    os_free(part_info);
                    ret = OS_NULL;
                }
            }
            else
            {
                os_free(part_info);
                ret = OS_NULL;
            }
        }
    }

    if (ret != OS_NULL)
    {
        if (vfs_lfs_cfg_init(part_info, dev) != 0)
        {
            os_free(part_info);
            ret = OS_NULL;
        }
    }

    return ret;
}

static int vfs_lfs_part_info_deinit(lfs_part_info_t *part_info)
{
#if 1
    os_list_node_t *head;
    lfs_open_entry_t *open_entry;
    int ret;

    LFS_ENTER_OPEN_LIST_CRITICAL(part_info);

    if (os_list_empty(&part_info->open_entry_head))
    {
        LFS_LEAVE_OPEN_LIST_CRITICAL(part_info);
        os_mutex_deinit(&part_info->open_lock);
        os_mutex_deinit(&part_info->dev_info.dev_lock);
        os_free(part_info);
        ret = 0;
    }
    else
    {
        head = &part_info->open_entry_head;

        os_list_for_each_entry(open_entry, head, lfs_open_entry_t, list_node)
        {
            LOG_W(LFS_FS_TAG, "This file/dir should be closed:%s", open_entry->name);
        }

        LFS_LEAVE_OPEN_LIST_CRITICAL(part_info);
        ret = -1;
    }

    return ret;
#else
    os_mutex_deinit(&part_info->lock);
    os_free(part_info);

    return 0;
#endif
}

static int vfs_lfs_add_entry(lfs_part_info_t *part_info, const char *path)
{
    os_list_node_t *head;
    lfs_open_entry_t *entry;
    lfs_open_entry_t *entry_temp;
    os_bool_t   foundflag = OS_FALSE;
    os_uint32_t len;
    int ret = 0; 

    head = &part_info->open_entry_head;
    len  = strlen(path);

    LFS_ENTER_OPEN_LIST_CRITICAL(part_info);

    os_list_for_each_entry_safe(entry, entry_temp, head, lfs_open_entry_t, list_node)
    {
        if (0 == strcmp(entry->name, path))
        {
            foundflag = OS_TRUE;
            break;
        }
    }

    if (OS_FALSE == foundflag)
    {
        entry = os_malloc(sizeof(lfs_open_entry_t));

        if (!entry)
        {
            ret = -1;
        }
        else
        {
            entry->name = os_malloc(len + 1);

            if (!entry->name)
            {
                ret = -1;
                os_free(entry);
            }
            else
            {
                strncpy(entry->name, path, strlen(path));
                entry->name[len] = '\0';
                os_list_add(&part_info->open_entry_head, &entry->list_node);
            }
        }
    }
    else
    {
        ret = -2;
    }

    LFS_LEAVE_OPEN_LIST_CRITICAL(part_info);

    return ret;
}

static int vfs_lfs_del_entry(lfs_part_info_t *part_info, const char *path)
{
    os_list_node_t *head;
    lfs_open_entry_t *entry;
    lfs_open_entry_t *entry_temp;
    int ret = -1; 

    head = &part_info->open_entry_head;
    LFS_ENTER_OPEN_LIST_CRITICAL(part_info);

    os_list_for_each_entry_safe(entry, entry_temp, head, lfs_open_entry_t, list_node)
    {
        if (0 == strcmp(entry->name, path))
        {
            os_list_del(&entry->list_node);
            os_free(entry->name);
            os_free(entry);
            ret = 0;
            break;
        }
    }

    LFS_LEAVE_OPEN_LIST_CRITICAL(part_info);

    return ret;
}

static os_bool_t vfs_lfs_find_open_entry(lfs_part_info_t *part_info, const char *path)
{
    os_list_node_t *head;
    lfs_open_entry_t *entry;
    os_bool_t ret = OS_FALSE; 

    head = &part_info->open_entry_head;

    LFS_ENTER_OPEN_LIST_CRITICAL(part_info);

    os_list_for_each_entry(entry, head, lfs_open_entry_t, list_node)
    {
        if (strcmp(entry->name, path) == 0)
        {
            ret = OS_TRUE;
            break;
        }
    }

    LFS_LEAVE_OPEN_LIST_CRITICAL(part_info);

    return ret;
}

static int vfs_lfs_mount(struct vfs_mountpoint *mnt_point, unsigned long mountflag, const void *data)
{
    int ret = 0;
    lfs_part_info_t *part_info;

    if (!mnt_point || !mnt_point->dev)
    {
        ret = -EINVAL;
    }
    else
    {
        if (OS_EOK == os_device_open((os_device_t *)mnt_point->dev))
        {
            part_info = vfs_lfs_part_info_init((os_device_t *)mnt_point->dev);

            if (OS_NULL == part_info)
            {
                ret = -ENOSPC;
                os_device_close((os_device_t *)mnt_point->dev);
            }
            else
            {
                ret = lfs_mount(&part_info->lfs, &part_info->config);

                if (ret != LFS_ERR_OK)
                {
                    os_device_close((os_device_t *)mnt_point->dev);
                    vfs_lfs_part_info_deinit(part_info);
                    ret = lfs_ret_to_vfs(ret);
                }
                else
                {
                    mnt_point->fs_instance  = (void *)part_info;
                    ret                     = 0;
                }
            }
        }
        else
        {
            ret = -EIO;
        }
    }

    return ret;
}

static int vfs_lfs_unmount(struct vfs_mountpoint *mnt_point)
{
    int ret = -ENOENT;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!mnt_point->dev))
    {
        ret = -EINVAL;
    }
    else
    {
        ret = lfs_unmount(&((lfs_part_info_t *)mnt_point->fs_instance)->lfs);

        if (ret) 
        {
            ret = lfs_ret_to_vfs(ret);
        }
        else
        {
            vfs_lfs_part_info_deinit((lfs_part_info_t *)mnt_point->fs_instance);
            os_device_close((os_device_t *)mnt_point->dev); 
            mnt_point->fs_instance = OS_NULL;
            ret = 0;
        }
    }

    return ret;
}

static int vfs_lfs_mkfs(void *dev)
{
#ifdef LFS_READONLY
    return -EPERM;
#else
    int               ret = 0;
    lfs_part_info_t  *part_info;

    if (!dev)
    {
        ret = -EINVAL;
    }
    else
    {
        if (OS_EOK == os_device_open((os_device_t *)dev))
        {
            part_info = vfs_lfs_part_info_init((os_device_t *)dev);

            if (part_info != OS_NULL)
            {
                ret = lfs_format(&part_info->lfs, &part_info->config);

                if (ret < 0)
                {
                    ret = lfs_ret_to_vfs(ret);
                }
                else
                {
                    ret = 0;
                }

                vfs_lfs_part_info_deinit(part_info);
            }
            else
            {
                ret = -EINVAL;
            }

            os_device_close((os_device_t *)dev);
        }
        else
        {
            ret = -EIO;
        }
    }

    return ret;
#endif
}

static int vfs_lfs_statfs(struct vfs_mountpoint *mnt_point, struct statfs *buf)
{
    lfs_part_info_t      *part_info;
    int                   ret;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!buf))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)mnt_point->fs_instance;

        buf->f_bsize  = part_info->config.block_size;

        buf->f_blocks = part_info->config.block_count;

        ret = lfs_fs_size(&part_info->lfs);

        if (ret < 0)
        {
            ret = lfs_ret_to_vfs(ret);
        }
        else
        {
            buf->f_bfree = buf->f_blocks - ret;
            ret = 0;
        }
    }

    return ret;
}

static const char lfs_root_path[] = ".";

static int vfs_lfs_open(struct vfs_file *file)
{
    uint32_t         oflag;
    int              lfs_flag;
    const char      *name;
    lfs_part_info_t *part_info;
    void            *data = OS_NULL;
    int              ret  = 0;

    if ((!file) || (!file->path) || (!file->mnt_point) || (!file->mnt_point->fs_instance))
    {
        ret = -EINVAL;
    }
    else
    {
        oflag = file->flags;

        /* Escape the '/' provided by vfs code */
        name = file->path;
        if ((name[0] == '/') && (name[1] == 0))
        {
            name = lfs_root_path;
        }
        else
        {
            /* name[0] still will be '/' */
            name++;
        }

        part_info = (lfs_part_info_t *)file->mnt_point->fs_instance;

#ifdef LFS_READONLY
        if (oflag & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC | O_EXCL | O_APPEND))
        {
            ret = -EPERM;
        }
        else
#endif
        {
            lfs_flag = 0;

            /* Handle regular file operations */
            if (oflag & O_WRONLY)
            {
                lfs_flag = LFS_O_WRONLY;
            }
            else if (oflag & O_RDWR)
            {
                lfs_flag = LFS_O_RDWR;
            }
            else
            {
                lfs_flag = LFS_O_RDONLY;
            }

            /* Opens the file, if it is existing. If not, a new file is created. */
            if (oflag & O_CREAT)
            {
                lfs_flag |= LFS_O_CREAT;
            }

            /* Creates a new file. If the file is existing, it is truncated and overwritten. */
            if (oflag & O_TRUNC)
            {
                lfs_flag |= LFS_O_TRUNC;
            }

            /* Creates a new file. The function fails if the file is already existing. */
            if (oflag & O_EXCL)
            {
                lfs_flag |= LFS_O_EXCL;
            }

            if (oflag & O_APPEND)
            {
                lfs_flag |= LFS_O_APPEND;
            }

            data = os_calloc(1, sizeof(lfs_file_t));

            if (OS_NULL == data)
            {
                ret = -ENOMEM;
            }
            else
            {
                ret = vfs_lfs_add_entry(part_info, file->path);
            
                if (0 == ret)
                {
                    ret = lfs_file_open(&part_info->lfs, (lfs_file_t *)data, name, lfs_flag);

                    if (ret != LFS_ERR_OK)
                    {
                        free(data);
                        vfs_lfs_del_entry(part_info, file->path);
                        ret = lfs_ret_to_vfs(ret);
                    }
                    else
                    {
                        /*
                         * Save this pointer, it will be used when calling read(), write(),
                         * flush(), lessk(), and will be free when calling close().
                         */
                        file->desc = data;
                    }
                }
                else
                {
                    if (-2 == ret)
                    {
                        LOG_E(LFS_FS_TAG, "The file has been opened:%s.You must close the file before opening it.", name);
                    }

                    free(data);
                    ret = -EINVAL;
                }
            }
        }
    }

    return ret;
}

static int vfs_lfs_close(struct vfs_file *file)
{
    int              ret = 0;
    lfs_part_info_t *part_info;

    if ((!file) || (!file->mnt_point) || (!file->mnt_point->fs_instance) || (!file->desc))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)file->mnt_point->fs_instance;

        /* Handle regular file  */
        ret = lfs_file_close(&part_info->lfs, (lfs_file_t *)file->desc);

        ret = lfs_ret_to_vfs(ret);

        free(file->desc);

        file->desc = OS_NULL;

        vfs_lfs_del_entry(part_info, file->path);
    }

    return ret;
}

static int vfs_lfs_read(struct vfs_file *file, void *buf, size_t size)
{
    lfs_ssize_t      read_cnt;
    lfs_file_t      *lfs_file;
    lfs_part_info_t *part_info;
    int              ret;

    if ((!file) || (!file->mnt_point) || (!file->mnt_point->fs_instance) || (!file->desc) || (!buf))
    {
        ret = -EINVAL;
    }
    else
    {
        if (FT_DIRECTORY == file->type)
        {
            ret = -EISDIR;
        }
        else
        {
            if (file->flags & O_WRONLY)
            {
                ret = -EINVAL;
            }
            else
            {
                part_info = (lfs_part_info_t *)file->mnt_point->fs_instance;
                lfs_file  = (lfs_file_t *)file->desc;

                read_cnt = lfs_file_read(&part_info->lfs, lfs_file, buf, (lfs_size_t)size);

                if (read_cnt < 0)
                {
                    ret = lfs_ret_to_vfs((int)read_cnt);
                }
                else
                {
                    ret = (int)read_cnt;
                }
            }
        }
    }

    return ret;
}

static int vfs_lfs_write(struct vfs_file *file, const void *buf, size_t size)
{
    lfs_ssize_t      write_cnt;
    lfs_file_t      *lfs_file;
    lfs_part_info_t *part_info;
    int              ret;

    if ((!file) || (!file->mnt_point) || (!file->mnt_point->fs_instance) || (!file->desc) || (!buf))
    {
        ret =  -EINVAL;
    }
    else
    {
        if ((!(file->flags & O_RDWR)) && (!(file->flags & O_WRONLY)))
        {
            ret = -EINVAL;
        }
        else
        {
            if (FT_DIRECTORY == file->type)
            {
                ret = -EISDIR;
            }
            else
            {
                part_info = (lfs_part_info_t *)file->mnt_point->fs_instance;
                lfs_file  = (lfs_file_t *)file->desc;

                write_cnt = lfs_file_write(&part_info->lfs, lfs_file, buf, (lfs_size_t)size);

                if (write_cnt < 0)
                {
                    ret = lfs_ret_to_vfs((int)write_cnt);
                }
                else
                {
                    ret = (int)write_cnt;
                }
            }
        }
    }

    return ret;
}

static int vfs_lfs_sync(struct vfs_file *file)
{
    int              ret;
    lfs_part_info_t *part_info; 
    lfs_file_t      *lfs_file;

    if ((!file) || (!file->mnt_point) || (!file->mnt_point->fs_instance) || (!file->desc))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)file->mnt_point->fs_instance;
        lfs_file  = (lfs_file_t *)file->desc;

        ret = lfs_file_sync(&part_info->lfs, lfs_file);

        if (ret < 0)
        {
            ret = lfs_ret_to_vfs(ret);
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}

static int vfs_lfs_lseek(struct vfs_file *file, off_t offset, int whence)
{
    lfs_part_info_t *part_info;
    lfs_soff_t       new_pos;
    lfs_file_t      *lfs_file;
    int              lfs_whence;
    int              ret;

    if ((!file) || (!file->mnt_point) || (!file->mnt_point->fs_instance) || (!file->desc))
    {
        ret = -EINVAL;
    }
    else
    {
        if (file->type != FT_REGULAR)
        {
            ret = -EINVAL;
        }
        else
        {
            part_info = (lfs_part_info_t *)file->mnt_point->fs_instance;
            lfs_file  = (lfs_file_t *)file->desc;

            ret = 0;

            switch (whence)
            {
            case SEEK_SET:
                lfs_whence = LFS_SEEK_SET;
                break;
            case SEEK_CUR:
                lfs_whence = LFS_SEEK_CUR;
                break;
            case SEEK_END:
                lfs_whence = LFS_SEEK_END;
                break;
            default:
                ret = -EINVAL;
                break;
            }

            if (0 == ret)
            {
                new_pos = lfs_file_seek(&part_info->lfs, lfs_file, (lfs_soff_t)offset, lfs_whence);

                if (new_pos < 0)
                {
                    ret = lfs_ret_to_vfs((int)new_pos);
                }
                else
                {
                    ret = (int)new_pos;
                }
            }
        }
    }

    return ret;
}

static int vfs_lfs_opendir(struct vfs_dir *dp)
{
    uint32_t         oflag;
    int              err;
    const char      *name;
    lfs_part_info_t *part_info;
    void            *data = OS_NULL;
    int              ret  = 0;

    if ((!dp) || (!dp->fp) || (!dp->fp->path) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        ret = -EINVAL;
    }
    else
    {
        oflag = dp->fp->flags;

        /* Escape  the '/' provided by vfs code */
        name = dp->fp->path;
        if ((name[0] == '/') && (name[1] == 0))
        {
            name = lfs_root_path;
        }
        else 
        {
            /* name[0] still will be '/' */
            name++;
        }

        part_info = (lfs_part_info_t *)dp->fp->mnt_point->fs_instance;

        data = os_calloc(1, sizeof(lfs_dir_t));

        if (OS_NULL == data)
        {
            ret = -ENOMEM;
        }
        else
        {
             /* Create a dir*/
            if (oflag & O_CREAT)
            {
#ifdef LFS_READONLY
                free(data);
                ret = -EPERM;
#else
                err = lfs_mkdir(&part_info->lfs, name);

                if (err < 0)
                {
                    free(data);
                    ret = lfs_ret_to_vfs(err);
                }
#endif
            }

            if (0 == ret)
            {
                err = vfs_lfs_add_entry(part_info, dp->fp->path);
                
                if (0 == err)
                {
                    /* Open dir */
                    err = lfs_dir_open(&part_info->lfs, (lfs_dir_t *)data, name);

                    if (err)
                    {
                        free(data);
                        vfs_lfs_del_entry(part_info, dp->fp->path);
                        ret = lfs_ret_to_vfs(err);
                    }
                    else
                    {
                        dp->fp->desc = data;
                    }
                }
                else
                {
                    if (-2 == err)
                    {
                        LOG_E(LFS_FS_TAG, "The directory has been opened:%s.You must close the directory before opening it.", name);
                    }

                    free(data);
                    ret = -EINVAL;
                }
            }
        }
    }

    return ret;
}

static int vfs_lfs_closedir(struct vfs_dir *dp)
{
    int              ret;
    int              err;
    lfs_part_info_t *part_info;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->path)
        || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)dp->fp->mnt_point->fs_instance;
            
        err = lfs_dir_close(&part_info->lfs, (lfs_dir_t *)dp->fp->desc);

        if (err)
        {
            ret = lfs_ret_to_vfs(err);
        }
        else
        {
            ret = 0;
        }

        free(dp->fp->desc);
        dp->fp->desc = OS_NULL;

        vfs_lfs_del_entry(part_info, dp->fp->path);
    }

    return ret;
}

static int vfs_lfs_seekdir(struct vfs_dir *dp, off_t offset)
{
    int              ret;
    lfs_part_info_t *part_info;
    lfs_dir_t       *lfs_dir;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        ret = -EINVAL;
    }
    else
    {

        part_info = (lfs_part_info_t *)dp->fp->mnt_point->fs_instance;
        lfs_dir   = (lfs_dir_t *)dp->fp->desc;

        ret = lfs_dir_seek(&part_info->lfs, lfs_dir, (lfs_off_t)offset);

        if (ret < 0)
        {
            ret = lfs_ret_to_vfs(ret);
        }
        else
        {
            ret = (int)lfs_dir->pos;
        }
    }

    return ret;
}

/* Return the size of struct dirent*/
static int vfs_lfs_readdir(struct vfs_dir *dp, struct dirent *dent)
{
    int              ret;
    struct lfs_info  info;
    lfs_part_info_t *part_info;
    lfs_dir_t       *lfs_dir;
    size_t           cpy_name_len;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance) || (!dent))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)dp->fp->mnt_point->fs_instance;
        lfs_dir   = (lfs_dir_t *)dp->fp->desc;

        memset(&info, 0, sizeof(struct lfs_info));

        ret = lfs_dir_read(&part_info->lfs, lfs_dir, &info);

        if (ret < 0)
        {
            /* Read error. */
            ret = lfs_ret_to_vfs(ret);
        }
        else if (0 == ret)
        {
            /* Reach end of the directory. */
            ;
        }
        else
        {
            /* Read success. */

            if (LFS_TYPE_REG == info.type)
            {
                dent->d_type = DT_REG;
            }
            else if (LFS_TYPE_DIR == info.type)
            {
                dent->d_type = DT_DIR; 
            }
            else
            {
                dent->d_type = DT_UNKNOWN;
            }

            /* Write the rest fields of struct dirent* dirp  */

            cpy_name_len = strlen(info.name);
            cpy_name_len = (cpy_name_len <= (sizeof(dent->d_name) - 1)) ? cpy_name_len : (sizeof(dent->d_name) - 1);

            strncpy(dent->d_name, info.name, cpy_name_len);
            dent->d_name[cpy_name_len] = '\0';

            ret = 1;
        }
    }

    return ret;
}

long vfs_lfs_telldir(struct vfs_dir *dp)
{
    lfs_part_info_t *part_info;
    lfs_dir_t       *lfs_dir;
    lfs_soff_t       ret;

    if ((!dp) || (!dp->fp) || (!dp->fp->desc) || (!dp->fp->mnt_point) || (!dp->fp->mnt_point->fs_instance))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)dp->fp->mnt_point->fs_instance;
        lfs_dir   = (lfs_dir_t *)dp->fp->desc;

        ret = lfs_dir_tell(&part_info->lfs, lfs_dir);

        if (ret < 0)
        {
            /* Read error. */
            ret = lfs_ret_to_vfs(ret);
        }
    }

    return (long)ret;
}

static int vfs_lfs_unlink(struct vfs_mountpoint *mnt_point, const char *path)
{
    int ret;

#ifdef LFS_READONLY
    (void)mnt_point;
    (void)path;
    ret = -EPERM;
#else
    lfs_part_info_t *part_info;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!path))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)mnt_point->fs_instance;
        
        if (OS_TRUE == vfs_lfs_find_open_entry(part_info, path))
        {
            LOG_W(LFS_FS_TAG, "This file/dir should be closed before delete:%s", path);
            ret = -EINVAL;
        }
        else
        {
            /* Skip starting '/' */
            if (path[0] == '/')
            {
                path++;
            }

            ret = lfs_remove(&part_info->lfs, path);

            if (ret < 0)
            {
                ret = lfs_ret_to_vfs(ret);
            }
            else
            {
                ret = 0;
            }
        }
    }
#endif

    return ret;
}

static int vfs_lfs_rename(struct vfs_mountpoint *mnt_point, const char *oldpath, const char *newpath)
{
    int ret;

#ifdef LFS_READONLY
    (void)fs;
    (void)oldpath;
    (void)newpath;
    ret = -EPERM;
#else
    lfs_part_info_t *part_info;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!oldpath) || (!newpath))
    {
        ret = -EINVAL;
    }
    else
    {
        part_info = (lfs_part_info_t *)mnt_point->fs_instance;

        if (OS_TRUE == vfs_lfs_find_open_entry(part_info, oldpath))
        {
            LOG_W(LFS_FS_TAG, "This file/dir should be closed:%s", oldpath);
            ret = -EINVAL;
        }
        else
        {
            if (OS_TRUE == vfs_lfs_find_open_entry(part_info, newpath))
            {
                LOG_W(LFS_FS_TAG, "This file/dir should be closed:%s", newpath);
                ret = -EINVAL;
            }
            else
            {
                if ('/' == *oldpath)
                {
                    oldpath += 1;
                }
                
                if ('/' == *newpath)
                {
                    newpath += 1;
                }

                ret = lfs_rename(&part_info->lfs, oldpath, newpath);

                if (ret < 0)
                {
                    ret = lfs_ret_to_vfs(ret);
                }
                else
                {
                    ret = 0;
                }
            }
        }
    }
#endif

    return ret;
}

static int vfs_lfs_stat(struct vfs_mountpoint *mnt_point, const char *path, struct stat *st)
{
    lfs_part_info_t *part_info;
    struct lfs_info  info;
    int              ret;

    if ((!mnt_point) || (!mnt_point->fs_instance) || (!path) || (!st))
    {
        ret = -EINVAL;
    }
    else
    {
        if (path[0] == '/')
        {
            path++;
        }
        
        part_info = (lfs_part_info_t *)mnt_point->fs_instance;

        ret = lfs_stat(&part_info->lfs, path, &info);

        if (ret < 0)
        {
            ret = lfs_ret_to_vfs(ret);
        }
        else
        {
            memset(st, 0, sizeof(struct stat));

            st->st_mode = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH;

            /* Convert to vfs stat structure */
            if (LFS_TYPE_REG == info.type)
            {
                st->st_mode |= S_IFREG;
                st->st_size = info.size;
            }
            else
            {
                /*Directory*/
                st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
            }

            ret = 0;
        }
    }

    return ret;
}

static const struct vfs_filesystem_ops _lfs_ops =
{
    "littlefs",

    vfs_lfs_open,
    vfs_lfs_close,
    vfs_lfs_read,
    vfs_lfs_write,
    vfs_lfs_lseek,
    vfs_lfs_sync,
    OS_NULL,               /* Not support ioctl. */
#ifdef OS_USING_IO_MULTIPLEXING
    OS_NULL,                /* Not support poll. */
#endif
    vfs_lfs_opendir,
    vfs_lfs_closedir,
    vfs_lfs_readdir,
    vfs_lfs_seekdir,
    vfs_lfs_telldir,

    vfs_lfs_rename,
    vfs_lfs_unlink,
    vfs_lfs_stat,

    vfs_lfs_mkfs,
    vfs_lfs_statfs,
    vfs_lfs_mount,
    vfs_lfs_unmount,
};


/**
 ***********************************************************************************************************************
 * @brief           This function do lfs file system initialization work. 
 *
 * @param[]         None
 *
 * @return          Return 0 on successful, -1 on failed.
 ***********************************************************************************************************************
 */
int vfs_lfs_init(void)
{
    /* Register lfs file system */
    vfs_register(&_lfs_ops);

    return 0;
}
OS_CMPOENT_INIT(vfs_lfs_init, OS_INIT_SUBLEVEL_LOW);


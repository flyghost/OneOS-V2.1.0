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
 * @file        cutefs.c
 *
 * @brief       This file implement the cute filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <string.h>
#include <os_util.h>
#include <os_memory.h>
#include <dlog.h>
#include <vfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/errno.h>
#include "cutefs_block.h"
#include "cutefs.h"

#define CUTEFS_TAG              "CUTEFS"

#define SEPARETE_NAME_END       0
#define SEPARETE_NAME_FAIL      1
#define SEPARETE_NAME_SUBDIR    2

#define CUTEFS_TIMEOUT          OS_WAIT_FOREVER

#define ENTER_CUTEFS(cfs)       os_mutex_lock(&cfs->lock, CUTEFS_TIMEOUT)
#define LEAVE_CUTEFS(cfs)       os_mutex_unlock(&cfs->lock)

static char cutefs_separete_name(const char **path, char *dirname)
{
    char n, c;

    memset(dirname, 0, CUTEFS_NAME_MAX);
    n = 0;
    for (;;) {
        c = *(*path)++;
        if (c <= ' ')
        {
            c = 0;
        }
        if ((c == 0) || (c == '/')) /* Reached to end of str or directory separator */
        {
            if (n == 0)
            {
                break;
            }
            if (c == 0)
            {
                return SEPARETE_NAME_END;
            }
            else
            {
                return SEPARETE_NAME_SUBDIR;
            }
        }
        if (c >= 0x7F) goto next;               /* Accept \x7F-0xFF */
        if (c == '"') break;                    /* Reject " */
        if (c <= ')') goto next;                /* Accept ! # $ % & ' ( ) */
        if (c <= ',') break;                    /* Reject * + , */
        if (c <= '9') goto next;                /* Accept - 0-9 */
        if (c <= '?') break;                    /* Reject : ; < = > ? */
        if (c == '|') break;                    /* Reject | */
        if ((c >= '[') && (c <= ']')) break;    /* Reject [ \ ] */
    next:
        if (n >= CUTEFS_NAME_MAX) break;
        dirname[n++] = c;
    }

    return SEPARETE_NAME_FAIL;
}

static struct cutefs_dirent *cutefs_trace_path(struct cutefs_dirent *root, const char *path, struct cutefs_dirent **dir_scan, char *fn)
{
    char result;
    os_list_node_t *list_head = OS_NULL;
    struct cutefs_dirent *dirent = OS_NULL;
    os_bool_t match_flag;

    *dir_scan = OS_NULL;
    while ((*path == ' ') || (*path == '/')) path++;

    if (!(*path))        /*root path */
    {
        return root;
    }

    if (*path < ' ')    /*Invalid path */
    {        
        return OS_NULL;
    }

    match_flag = OS_FALSE;
    list_head = &root->dir_head;
    while(1){
        result = cutefs_separete_name(&path, fn);
        if (result == SEPARETE_NAME_FAIL)
        {
            return OS_NULL;
        }

        os_list_for_each_entry(dirent, list_head, struct cutefs_dirent, list_node)
        { 
            if (strncmp(dirent->name, fn, CUTEFS_NAME_MAX) == 0)
            {
                match_flag = OS_TRUE;
                break;
            }
        }

        if (match_flag == OS_FALSE)
        {
            if (result == SEPARETE_NAME_END)    /*if only last name not match, need save the father dir */
            {
                *dir_scan = os_list_entry(list_head, struct cutefs_dirent, dir_head);
            }
            return OS_NULL;
        }
        else
        {
            if (result == SEPARETE_NAME_END)    /* Find the matched file or sub dir */
            {            
                *dir_scan = os_list_entry(list_head, struct cutefs_dirent, dir_head);
                return dirent;
            }

            if (dirent->type == FT_DIRECTORY)   /* Countinue to next turn, find in the sub dir*/
            {
                list_head = &dirent->dir_head;
                match_flag = OS_FALSE;
            }
            else
            {
                return OS_NULL;
            }
        }
    }
}

static struct cutefs_dirent *cutefs_entry_create(struct cutefs_dirent *father_dir, char *name, os_uint16_t entry_type)
{    
    struct cutefs_dirent *dirent;

    if ((!father_dir) || (father_dir->type != FT_DIRECTORY))
    {
        return OS_NULL;
    }

    /* Create a new entry. */
    dirent = (struct cutefs_dirent *)os_malloc(sizeof(struct cutefs_dirent));
    if (!dirent)
    {
        return OS_NULL;
    }

    strncpy(dirent->name, name, CUTEFS_NAME_MAX);
    dirent->data.pblock = OS_NULL;
    dirent->data.block_cnt = 0;
    dirent->size = 0;
    dirent->type = entry_type;
    dirent->ref_cnt = 0;
    os_list_init(&(dirent->dir_head));
    os_list_init(&(dirent->list_node));
    os_list_add_tail(&(father_dir->dir_head), &(dirent->list_node));  /* Add to the father directory */

    return dirent;
}

static void cutefs_entry_destory(struct cutefs_dirent *dirent)
{
    if (!dirent)
    {
        return;
    }

    os_list_del(&(dirent->list_node));
    os_free(dirent);
}

static int cutefs_entry_move(struct cutefs_dirent *dirent, struct cutefs_dirent *father_dir_new)
{
    if ((!dirent) || (!father_dir_new))
    {
        return -ENOENT;        
    }

    os_list_del(&dirent->list_node);
    os_list_add(&father_dir_new->dir_head, &dirent->list_node);

    return 0;
}

static void cutefs_entry_del_block(struct cutefs *cfs, struct cutefs_dirent *dirent)
{    
    os_uint32_t i = 0;

    if ((!cfs) || (!dirent))
    {
        return;
    }

    dirent->size = 0;
    if (dirent->data.pblock)
    {
        for (i = 0; i < dirent->data.block_cnt; i++)
        {
            cutefs_block_free(cfs->block_pool, (void *)dirent->data.pblock[i]);
            cfs->block_freecnt++;
        }
        os_free(dirent->data.pblock);
        dirent->data.pblock = OS_NULL;
        dirent->data.block_cnt = 0;
    }
}

static int cutefs_entry_add_block(struct cutefs *cfs, struct cutefs_dirent *dirent, os_uint32_t add_cnt)
{    
    void *p_block_addr;
    void *block;
    os_uint32_t total_cnt = 0;

    if (add_cnt == 0)
    {
        return 0;
    }

    total_cnt = (dirent->data.block_cnt + add_cnt);
    p_block_addr = os_realloc(dirent->data.pblock, total_cnt * sizeof(block_addr_t));
    if (!p_block_addr)
    {
        return -ENOMEM;
    }
    dirent->data.pblock = p_block_addr;

    while (dirent->data.block_cnt < total_cnt)
    {
        block = cutefs_block_alloc(cfs->block_pool);
        if (!block)
        {
            return -ENOMEM;
        }
        dirent->data.pblock[dirent->data.block_cnt] = (block_addr_t)block;
        dirent->data.block_cnt++;
        cfs->block_freecnt--;
    }

    return 0;
}

static int cutefs_entry_unlink(struct cutefs *cfs, struct cutefs_dirent *dirent)
{
    if (!dirent)
    {
        return -EINVAL;
    }

    if (dirent->ref_cnt > 0)
    {
        LOG_W(CUTEFS_TAG, "%s has been opened, ref_cnt:%d", dirent->name, dirent->ref_cnt);
        return -EBUSY;
    }

    if ((dirent->type == FT_DIRECTORY) && (!os_list_empty(&dirent->dir_head)))
    {
        LOG_W(CUTEFS_TAG, "directory:%s not empty.", dirent->name);
        return -EINVAL;
    }

    if (dirent->type == FT_REGULAR)
    {
        cutefs_entry_del_block(cfs, dirent);
    }
    cutefs_entry_destory(dirent);

    return 0;
}

static int cutefs_dir_del_recursive(struct cutefs *cfs, struct cutefs_dirent *dir)
{
    os_list_node_t *list_head = &dir->dir_head;
    struct cutefs_dirent *dirent;
    struct cutefs_dirent *dirent_temp;
    int result = 0;

    os_list_for_each_entry_safe(dirent, dirent_temp, list_head, struct cutefs_dirent, list_node)
    {
        if (dirent->type == FT_DIRECTORY)
        {
            if (!os_list_empty(&dirent->dir_head))      /* Dirctory, but not empty, recursive delete. */
            {
                result = cutefs_dir_del_recursive(cfs, dirent);
                if (result < 0)
                {
                    return result;
                }
            }
            result = cutefs_entry_unlink(cfs, dirent);  /* Now should be empty directory, delete it.*/
            if (result < 0)
            {
                return result;
            }
        }
        else                                            /* File, delete it */
        {
            result = cutefs_entry_unlink(cfs, dirent);
            if (result < 0)
            {
                return result;
            }
        }
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Mount cute filesystem.
 *
 * @param[in]       device          The block device.
 *
 * @return          The mount result.
 * @retval          !OS_NULL        Mount successfully, return the object of cute filesystem.
 * @retval          OS_NULL         Mount failed.
 ***********************************************************************************************************************
 */
struct cutefs *cutefs_mount(os_device_t *device)
{    
    struct cutefs *cfs;
    char name[OS_NAME_MAX];

    if (!device)
    {
        return OS_NULL;
    }

    cfs = os_malloc(sizeof(struct cutefs));
    if (!cfs)
    {
       return OS_NULL;
    }
    memset(cfs, 0, sizeof(struct cutefs));

    cfs->block_pool = cutefs_block_init(device);
    if (!cfs->block_pool)
    {        
        os_free(cfs);
        return OS_NULL;
    }
    if (OS_EOK != cutefs_block_getinfo(cfs->block_pool, &cfs->block_count, &cfs->block_size))
    {        
        os_free(cfs);
        return OS_NULL;
    }
    cfs->block_freecnt = cfs->block_count;
    cfs->temp_buff = os_malloc(cfs->block_size);
    if (!cfs->temp_buff)
    {
        os_free(cfs);
        return OS_NULL;
    }

    os_snprintf(name, sizeof(name), "CUTE%p", cfs);
    os_mutex_init(&cfs->lock, name, OS_FALSE);
    os_list_init(&(cfs->root.list_node));
    os_list_init(&(cfs->root.dir_head));
    cfs->root.data.pblock = OS_NULL;
    cfs->root.data.block_cnt = 0;
    strcpy(cfs->root.name, ".");
    cfs->root.size = 0;
    cfs->root.type = FT_DIRECTORY;

    return cfs;
}

/**
 ***********************************************************************************************************************
 * @brief           Unmount cute filesystem.
 *
 * @param[in]       cfs             The object of cute filesystem.
 *
 * @return          The unmount result.
 * @retval          0               Unmount successfully.
 * @retval          -1              Unmount failed.
 ***********************************************************************************************************************
 */
int cutefs_unmount(struct cutefs *cfs)
{
    if ((!cfs) || (!cfs->block_pool) || (!cfs->temp_buff))
    {
        return -1;
    }

    ENTER_CUTEFS(cfs);
    if(cutefs_dir_del_recursive(cfs, &cfs->root) < 0)
    {
        LEAVE_CUTEFS(cfs);
        return -1;
    }
    cutefs_block_deinit(cfs->block_pool);
    os_free(cfs->temp_buff);
    LEAVE_CUTEFS(cfs);
    os_mutex_deinit(&cfs->lock);
    os_free(cfs);

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the cute filesystem stat.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[out]      buf             The buffer to save stat.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void cutefs_statfs(struct cutefs *cfs, struct statfs *buf)
{
    ENTER_CUTEFS(cfs);
    buf->f_bsize  = cfs->block_size;
    buf->f_blocks = cfs->block_count;
    buf->f_bfree  = cfs->block_freecnt;
    LEAVE_CUTEFS(cfs);
}

/**
 ***********************************************************************************************************************
 * @brief           Open file/directory.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in,out]   cfs_fd          The cfs file descriptor.
 * @param[in]       path            The path of file or directory.
 * @param[in]       flags           The file flag. 
 *
 * @return          The open result.
 * @retval          0               Open successfully.
 * @retval          negative int    Open failed, return error code.
 ***********************************************************************************************************************
 */
int cutefs_open(struct cutefs *cfs, struct cutefs_fd *cfs_fd, char *path, os_uint32_t flags)
{
    struct cutefs_dirent *dirent;
    struct cutefs_dirent *father_dir;
    char name_buff[CUTEFS_NAME_MAX];
    int result = 0;

    ENTER_CUTEFS(cfs);
    dirent = cutefs_trace_path(&cfs->root, path, &father_dir, &name_buff[0]);
    if (flags & O_DIRECTORY)
    {
        if (dirent)
        {
            if (dirent->type != FT_DIRECTORY)
            {
                result = -ENOTDIR;
                goto fail;
            }
            if (flags & O_CREAT)
            {
                result = -EEXIST;
                goto fail;
            }
        }
        else
        {
            if (flags & O_CREAT)
            {
                dirent = cutefs_entry_create(father_dir, &name_buff[0], FT_DIRECTORY);
                if (!dirent)
                {
                    result = -ENOMEM;
                    goto fail;
                }
            }
            else
            {
                result = -ENOENT;
                goto fail;
            }
        }
    }
    else
    {
        if (dirent)
        {
            if (dirent->type != FT_REGULAR)
            {
                result = -EISDIR;
                goto fail;
            }
            if ((flags & O_CREAT) && (flags & O_EXCL))
            {
                result = -EEXIST;
                goto fail;
            }
        }
        else
        {
            if (flags & O_CREAT)
            {
                dirent = cutefs_entry_create(father_dir, &name_buff[0], FT_REGULAR);
                if (!dirent)
                {
                    result = -ENOMEM;
                    goto fail;
                }
            }
            else
            {
                result = -ENOENT;
                goto fail;
            }
        }

        /*If the file existed and need be truncated, it will be overwritten. */
        if ((flags & O_TRUNC) && ((flags & O_RDWR) || (flags & O_WRONLY)))
        {
            cutefs_entry_del_block(cfs, dirent);
        }
    }
    dirent->ref_cnt++;
    cfs_fd->dirent = dirent;
    cfs_fd->flags = flags;
    cfs_fd->size = dirent->size;
    if ((flags & O_APPEND))
    {
        cfs_fd->pos = cfs_fd->size;
    }
    else
    {
        cfs_fd->pos = 0;
    }
    LEAVE_CUTEFS(cfs);
    return 0;

fail:
    LEAVE_CUTEFS(cfs);
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Close file/directory.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in,out]   cfs_fd          The cfs file descriptor.
 *
 * @return          The close result.
 * @retval          0               Close successfully.
 * @retval          negative int    Close failed, return error code.
 ***********************************************************************************************************************
 */
int cutefs_close(struct cutefs *cfs, struct cutefs_fd *cfs_fd)
{
    int result = -EINVAL;

    ENTER_CUTEFS(cfs);
    if ((cfs_fd) && (cfs_fd->dirent) && (cfs_fd->dirent->ref_cnt > 0))
    {
        cfs_fd->dirent->ref_cnt--;
        result = 0;
    }
    LEAVE_CUTEFS(cfs);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Read file.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in]       cfs_fd          The cfs file descriptor.
 * @param[out]      buf             The buffer to save data.
 * @param[in]       pos             The file position.
 * @param[in]       size            The expected read size.
 *
 * @return          The actual read size.
 * @retval          positive int    The actual read size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
int cutefs_read(struct cutefs *cfs, struct cutefs_fd *cfs_fd, void *buf, size_t size)
{
    struct cutefs_dirent *dirent;
    os_uint32_t block_start;    
    os_uint32_t block_cur;
    os_uint32_t block_end;
    os_uint32_t block_start_offset;
    os_uint32_t block_end_offset;
    os_uint32_t block_read_size;
    os_uint32_t actual_read_size;
    block_addr_t block;
    char *dest;
    off_t pos;

    dirent = cfs_fd->dirent;
    if ((!dirent) || (dirent->type != FT_REGULAR))
    {
        return -EINVAL;
    }
    if (cfs_fd->flags & O_WRONLY)
    {
        return -EINVAL;
    }

    ENTER_CUTEFS(cfs);
    pos = cfs_fd->pos;
    if (dirent->size < pos)
    {
        LEAVE_CUTEFS(cfs);
        return -EINVAL;
    }

    if (size > (dirent->size - pos))
    {
        size = dirent->size - pos;
    }
    if (size == 0)
    {
        LEAVE_CUTEFS(cfs);
        return 0;
    }

    dest = (char *)buf;
    block_start = pos / cfs->block_size;
    block_end = (pos + size) / cfs->block_size;
    if (block_end > dirent->data.block_cnt)
    {
        LEAVE_CUTEFS(cfs);
        return -EINVAL;
    }
    block_start_offset = pos % cfs->block_size;
    block_end_offset = (pos + size) % cfs->block_size;

    block_cur = block_start;
    if (block_start_offset)
    {
        block = dirent->data.pblock[block_cur];
        if (cutefs_block_read(cfs->block_pool, (void *)block, cfs->temp_buff) == 0)
        {
            goto end;
        }
        if ((block_start_offset + size) > cfs->block_size)
        {
            block_read_size = cfs->block_size - block_start_offset;
        }
        else
        {
            block_read_size = size;
        }
        memcpy(dest, cfs->temp_buff + block_start_offset, block_read_size);
        dest += block_read_size;
        size -= block_read_size;
        block_cur ++;
    }

    while (block_cur < block_end)
    {
        block = dirent->data.pblock[block_cur];
        if (cutefs_block_read(cfs->block_pool, (void *)block, dest) == 0)
        {
            goto end;
        }
        dest += cfs->block_size;
        size -= cfs->block_size;
        block_cur ++;
    }

    if (block_cur == block_end)
    {
        block = dirent->data.pblock[block_cur];
        if (cutefs_block_read(cfs->block_pool, (void *)block, cfs->temp_buff) == 0)
        {
            goto end;
        }
        memcpy(dest, cfs->temp_buff, block_end_offset);
        dest += block_end_offset;
        size -= block_end_offset;
        block_cur ++;
    }

    if (size)
    {
        LEAVE_CUTEFS(cfs);
        return -EINVAL;
    }

end:
    actual_read_size = (dest - (char *)buf);
    cfs_fd->pos += actual_read_size;
    LEAVE_CUTEFS(cfs);
    return actual_read_size;
}

/**
 ***********************************************************************************************************************
 * @brief           Write file.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in]       cfs_fd          The cfs file descriptor.
 * @param[in]       buf             The data to write.
 * @param[in]       pos             The file position.
 * @param[in]       size            The expected write size. 
 *
 * @return          The actual write size.
 * @retval          positive int    The actual write size.
 * @retval          negative int    Write failed, return error code.
 ***********************************************************************************************************************
 */
int cutefs_write(struct cutefs *cfs, struct cutefs_fd *cfs_fd, const void *buf, size_t size)
{
    struct cutefs_dirent *dirent;
    os_uint32_t block_start;
    os_uint32_t block_cur;
    os_uint32_t block_end;
    os_uint32_t block_start_offset;
    os_uint32_t block_end_offset;
    os_uint32_t block_write_size;
    os_uint32_t block;
    os_uint32_t actual_write_size;
    os_uint32_t block_add_cnt;
    char *src;
    off_t pos;

    dirent = cfs_fd->dirent;
    if ((!dirent) || (dirent->type != FT_REGULAR))
    {
        return -EINVAL;
    }
    if ((!(cfs_fd->flags & O_RDWR)) && (!(cfs_fd->flags & O_WRONLY)))
    {
        return -EINVAL;
    }

    ENTER_CUTEFS(cfs);
    pos = cfs_fd->pos;
    block_start = pos / cfs->block_size;
    block_end = (pos + size) / cfs->block_size;
    block_start_offset = pos % cfs->block_size;
    block_end_offset = (pos + size) % cfs->block_size;

    if (block_end >= dirent->data.block_cnt)
    {
        block_add_cnt = block_end + 1 - dirent->data.block_cnt;

        /* If block not enough, limit the write size */
        if (block_add_cnt > cfs->block_freecnt)
        {
            block_add_cnt = cfs->block_freecnt;
            block_end = dirent->data.block_cnt + block_add_cnt - 1;
            block_end_offset = cfs->block_size - 1;
            size = block_add_cnt * cfs->block_size + (cfs->block_size - block_start_offset); 
        }

        /* Modify the block count */
        if (cutefs_entry_add_block(cfs, dirent, block_add_cnt) != 0)
        {
            LEAVE_CUTEFS(cfs);
            return -ENOMEM;
        }
    }

    src = (char *)buf;
    block_cur = block_start;
    if (block_start_offset)
    {
        block = dirent->data.pblock[block_cur];
        if (cutefs_block_read(cfs->block_pool, (void *)block, cfs->temp_buff) == 0)
        {
            goto end;
        }
        if ((block_start_offset + size) > cfs->block_size)
        {
            block_write_size = cfs->block_size - block_start_offset;
        }
        else
        {
            block_write_size = size;
        }
        memcpy(cfs->temp_buff + block_start_offset, src, block_write_size);
        if (cutefs_block_write(cfs->block_pool, (void *)block, cfs->temp_buff) == 0)
        {
            goto end;
        }
        src += block_write_size;
        size -= block_write_size;
        block_cur ++;
    }

    while (block_cur < block_end)
    {
        block = dirent->data.pblock[block_cur];
        if (cutefs_block_write(cfs->block_pool, (void *)block, src) == 0)
        {
            goto end;
        }
        src += cfs->block_size;
        size -= cfs->block_size;
        block_cur ++;
    }

    if (block_cur == block_end)
    {
        block = dirent->data.pblock[block_cur];
        if (cutefs_block_read(cfs->block_pool, (void *)block, cfs->temp_buff) == 0)
        {
            goto end;
        }
        memcpy(cfs->temp_buff, src, block_end_offset);
        if (cutefs_block_write(cfs->block_pool, (void *)block, cfs->temp_buff) == 0)
        {
            goto end;
        }
        src += block_end_offset;
        size -= block_end_offset;
        block_cur ++;
    }

    if (size)
    {
        LEAVE_CUTEFS(cfs);
        return -EINVAL;
    }

end:
    actual_write_size = (src - (char *)buf);
    if ((pos + actual_write_size) > dirent->size)
    {
        dirent->size = pos +actual_write_size;
        cfs_fd->size = dirent->size;
    }
    cfs_fd->pos += actual_write_size;
    LEAVE_CUTEFS(cfs);
    return actual_write_size;
}


int cutefs_seek(struct cutefs *cfs, struct cutefs_fd *cfs_fd, off_t offset)
{
    ENTER_CUTEFS(cfs);
    cfs_fd->pos = offset;
    LEAVE_CUTEFS(cfs);

    return offset;
}

int cutefs_tell(struct cutefs *cfs, struct cutefs_fd *cfs_fd)
{
    return cfs_fd->pos;
}

/**
 ***********************************************************************************************************************
 * @brief           Get dir entry.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in]       cfs_fd          The cfs file descriptor.
 * @param[out]      dirp            The pointer to save dir entry.
 * @param[in]       pos             The entry position.
 * @param[out]      count           The buffer size to save dir entry.
 *
 * @return          The read size.
 * @retval          positive int    The actual read size.
 * @retval          negative int    Read failed, return error code.
 ***********************************************************************************************************************
 */
int cutefs_readdir(struct cutefs *cfs, struct cutefs_fd *cfs_fd, struct dirent *dent)
{
    os_uint32_t index = 0;
    os_list_node_t *dir_head;
    struct cutefs_dirent *dirent;
    int read_cnt = 0;

    if ((!cfs_fd->dirent) || (cfs_fd->dirent->type != FT_DIRECTORY))
    {
        return -EINVAL;
    }

    ENTER_CUTEFS(cfs);
    dir_head = &cfs_fd->dirent->dir_head;
    os_list_for_each_entry(dirent, dir_head, struct cutefs_dirent, list_node)
    {
        if (index >= (os_size_t)cfs_fd->pos)
        {
            dent->d_type = (dirent->type == FT_REGULAR) ? DT_REG : DT_DIR;
            //d->d_namlen = OS_NAME_MAX;
            //d->d_reclen = (os_uint16_t)sizeof(struct dirent);
            strncpy(dent->d_name, dirent->name, CUTEFS_NAME_MAX);
            cfs_fd->pos++;
            read_cnt++;
            break;
        }
        index++;
    }
    LEAVE_CUTEFS(cfs);

    return read_cnt;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete dir entry from cute filesystem.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in]       path            The path (the file name) to be delete.
 *
 * @return          The delete result.
 * @retval          0               Delete successfully.
 * @retval          negative int    Delete fail, return err code.
 ***********************************************************************************************************************
 */
int cutefs_unlink(struct cutefs *cfs, const char *path)
{
    struct cutefs_dirent *dirent;
    struct cutefs_dirent *dir_scan;
    char name_buff[CUTEFS_NAME_MAX];
    int result = -ENOENT;

    ENTER_CUTEFS(cfs);
    dirent = cutefs_trace_path(&cfs->root, path, &dir_scan, &name_buff[0]);
    if (dirent)
    {
        result = cutefs_entry_unlink(cfs, dirent);
    }
    LEAVE_CUTEFS(cfs);
 
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the file/directory entry.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in]       path            The file/directory path.
 * @param[out]      info            The file info.
 *
 * @return          The operation result.
 * @retval          0               Get stat successfully.
 * @retval          negative int    Get stat fail, return error code.
 ***********************************************************************************************************************
 */
int cutefs_stat(struct cutefs *cfs, const char *path, struct cutefs_fileinfo *info)
{
    struct cutefs_dirent *dirent;
    struct cutefs_dirent *dir_scan;
    char name_buff[CUTEFS_NAME_MAX];
    int result = -ENOENT;

    ENTER_CUTEFS(cfs);
    dirent = cutefs_trace_path(&cfs->root, path, &dir_scan, &name_buff[0]);
    if (dirent)
    {
        info->size = dirent->size;
        info->type = dirent->type;
        result = 0;
    }
    LEAVE_CUTEFS(cfs);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           Rename file/directory in cute filesystem.
 *
 * @param[in]       cfs             The object of cute filesystem.
 * @param[in]       oldpath         The old file/directory name.
 * @param[in]       newpath         The new file/directory name.
 *
 * @return          The rename result.
 * @retval          0               Rename successfully.
 * @retval          negative int    Rename fail, return error code.
 ***********************************************************************************************************************
 */
int cutefs_rename(struct cutefs *cfs, const char *oldpath, const char *newpath)
{
    struct cutefs_dirent *father_dir_new;
    struct cutefs_dirent *father_dir_old;
    struct cutefs_dirent *dirent_new;
    struct cutefs_dirent *dirent_old;
    char *name_new;
    char *name_old;
    int err_code = 0;

    /* The new pathname shall not contain a path prefix that names old. */
    if (strlen(newpath) >= strlen(oldpath))
    {
        if (strncmp(newpath, oldpath, strlen(oldpath)) == 0)
        {
            if ('/' == newpath[strlen(oldpath)])
            {
                return -EINVAL;
            }
        }
    }

    name_new = os_malloc(CUTEFS_NAME_MAX);
    if (!name_new)
    {
        return -ENOMEM;
    }
    name_old = os_malloc(CUTEFS_NAME_MAX);
    if (!name_old)
    {
        os_free(name_new);
        return -ENOMEM;
    }

    ENTER_CUTEFS(cfs);
    dirent_new = cutefs_trace_path(&cfs->root, newpath, &father_dir_new, name_new);
    dirent_old = cutefs_trace_path(&cfs->root, oldpath, &father_dir_old, name_old);
    /* The old pathname not exist, return fail. */
    if (!dirent_old)
    {
        err_code = -ENOENT;
        goto end;
    }

    /* The old pathname has been open, return fail. */
    if (dirent_old->ref_cnt > 0)
    {
        err_code = -EBUSY;
        LOG_W(CUTEFS_TAG, "%s has been opened, ref_cnt:%d", dirent_old->name, dirent_old->ref_cnt);
        goto end;
    }

    /* If newpath and oldpath are the same existing file, shall return successfully and perform no other action.*/
    if (strcmp(newpath, oldpath) == 0)
    {
        err_code = 0;
        goto end;
    }

    if (dirent_new)
    {
        /* If new pathname's type not same as old pathname's type, return fail.*/
        if (dirent_new->type != dirent_old->type)
        {
            err_code = (dirent_new->type == FT_DIRECTORY) ? (-EISDIR) : (-ENOTDIR);
            goto end;
        }

        /* If new pathname has been open, will return fail. */
        /* If new pathname is a directory, and the directory is not empty, will return fail. */
        err_code = cutefs_entry_unlink(cfs, dirent_new);
        if (err_code < 0)
        {
            goto end;
        }
    }

    /* If father_dir_new not exist, will return fail.*/
    err_code = cutefs_entry_move(dirent_old, father_dir_new);
    if (err_code == 0)
    {
        strncpy(dirent_old->name, name_new, CUTEFS_NAME_MAX);
    }

end:
    LEAVE_CUTEFS(cfs);
    os_free(name_new);
    os_free(name_old);
    return err_code;
}


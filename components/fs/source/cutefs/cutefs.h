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
 * @file        cutefs.h
 *
 * @brief       Internal header file for cute filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __CUTEFS_H__
#define __CUTEFS_H__

#include <os_types.h>
#include <os_list.h>
#include <device.h>
#include <os_mutex.h>

#define CUTEFS_NAME_MAX     32              /* The max length of RAMFS file. */

typedef os_uint32_t         block_addr_t;

struct file_data
{
    block_addr_t           *pblock;         /* The addr to save block info */
    os_uint32_t             block_cnt;      /* The block cnt */
};

struct cutefs_dirent
{
    os_list_node_t          list_node;      /* List chain for same level file */
    os_list_node_t          dir_head;       /* If dir, it's the list head for the content in dir */
    char                    name[CUTEFS_NAME_MAX]; /* Dirent name (file name). */
    struct file_data        data;           /* Real data (file content). */
    os_size_t               size;           /* File size. */
    os_uint16_t             type;           /* Type: directory or file */
    os_uint16_t             ref_cnt;        /* Reference count, increase when open */
};

struct cutefs_fd
{
    struct cutefs_dirent   *dirent;         /* Pointer to the dirent*/
    os_uint32_t             flags;          /* File flags*/
    os_size_t               size;           /* File size. */
    off_t                   pos;            /* File position or dir entry position. */
};

struct cutefs_fileinfo
{
    os_size_t               size;           /* File size. */
    os_uint16_t             type;           /* Type: directory or file */
};

struct cutefs
{
    char                   *block_pool;     /* The pointer of block pool */
    char                   *temp_buff;      /* Temp buffer when read/write data */
    os_uint32_t             block_count;    /* Tatol block count */
    os_uint32_t             block_size;     /* Size of every block */
    os_uint32_t             block_freecnt;  /* The block not used now */
    struct cutefs_dirent    root;           /* Root dir entry. */
    os_mutex_t              lock;           /* cutefs lock */
};

extern struct cutefs *cutefs_mount(os_device_t *device);
extern int cutefs_unmount(struct cutefs *cfs);
extern void cutefs_statfs(struct cutefs *cfs, struct statfs *buf);
extern int cutefs_open(struct cutefs *cfs, struct cutefs_fd *cfs_fd, char *path, os_uint32_t flags);
extern int cutefs_close(struct cutefs *cfs, struct cutefs_fd *cfs_fd);
extern int cutefs_stat(struct cutefs *cfs, const char *path, struct cutefs_fileinfo *info);
extern int cutefs_read(struct cutefs *cfs, struct cutefs_fd *cfs_fd, void *buf, size_t size);
extern int cutefs_write(struct cutefs *cfs, struct cutefs_fd *cfs_fd, const void *buf, size_t size);
extern int cutefs_seek(struct cutefs *cfs, struct cutefs_fd *cfs_fd, off_t offset);
extern int cutefs_tell(struct cutefs *cfs, struct cutefs_fd *cfs_fd);
extern int cutefs_readdir(struct cutefs *cfs, struct cutefs_fd *cfs_fd, struct dirent *dent);
extern int cutefs_unlink(struct cutefs *cfs, const char *path);
extern int cutefs_rename(struct cutefs *cfs, const char *oldpath, const char *newpath);

#endif

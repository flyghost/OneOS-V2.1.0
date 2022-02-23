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
 * @file        nfs.h
 *
 * @brief       Hearder file of the nfs interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      Rafactor nfs code
 ***********************************************************************************************************************
 */

#ifndef __NFS_H__
#define __NFS_H__

#include <rpc/rpc.h>
#include "mount.h"
#include "nfs_clnt.h"

#define NAME_MAX    64
#define DFS_NFS_MAX_MTU  1024

#define NFS_BLOCK_SIZE  1024    /* Can't obtain block size, suppose the server's block size is 1024*/

#define HOST_LENGTH         32
#define EXPORT_PATH_LENGTH  32

#ifdef _WIN32
#define strtok_r strtok_s
#endif

struct nfs_fd
{
    nfs_fh3 *handle;        /* handle */
    size_t offset;          /* current offset */

    size_t size;            /* total size */
    bool_t eof;             /* end of file */
};

struct nfs_dir
{
    nfs_fh3 *handle;
    cookie3 cookie;
    bool_t eof;             /* If FALSE, there may be more entries to read. */
    bool_t readflag;        /* The flag to indicate whether it has readdir */
    cookieverf3 cookieverf;
    entryplus3 *entry;
    READDIRPLUS3res res;
};

struct nfs_filesystem
{
    nfs_fh3 root_handle;
    CLIENT *nfs_client;
    CLIENT *mount_client;
    char host[HOST_LENGTH];
    char export[EXPORT_PATH_LENGTH];
};

extern struct nfs_filesystem *nfs_mount(char *data);
extern int nfs_unmount(struct nfs_filesystem *nfs);
extern int nfs_stat(struct nfs_filesystem *nfs, const char *name, fattr3 *attributes);
extern size_t nfs_get_filesize(struct nfs_filesystem *nfs, const char *name);
extern int nfs_get_type(struct nfs_filesystem *nfs, const char *name);
extern int nfs_read(struct nfs_filesystem *nfs, struct nfs_fd *n_fd, void *buf, off_t pos, size_t size);
extern int nfs_write(struct nfs_filesystem *nfs, struct nfs_fd *n_fd, const void *buf, off_t pos, size_t size);
extern void nfs_seekfile(struct nfs_fd *n_fd, off_t offset);
extern void nfs_seekdir(struct nfs_dir *dir, off_t offset);
extern off_t nfs_telldir(struct nfs_dir *dir);
extern void nfs_closefile(struct nfs_fd *n_fd);
extern void nfs_closedir(struct nfs_dir *dir);
extern struct nfs_fd *nfs_openfile(struct nfs_filesystem *nfs, const char *path, os_uint32_t flags);
extern struct nfs_dir *nfs_opendir(struct nfs_filesystem *nfs, const char *path);
extern int nfs_mkfile(struct nfs_filesystem *nfs, const char *name, mode_t mode);
extern int nfs_mkdir(struct nfs_filesystem *nfs, const char *name, mode_t mode);
extern int nfs_readdir(struct nfs_filesystem *nfs, struct nfs_dir *dir, struct dirent *d);
extern int nfs_statfs(struct nfs_filesystem *nfs, struct statfs *buf);
extern int nfs_unlinkfile(struct nfs_filesystem *nfs, const char *path);
extern int nfs_unlinkdir(struct nfs_filesystem *nfs, const char *path);
extern int nfs_rename(struct nfs_filesystem *nfs, const char *oldpath, const char *newpath);
//extern int nfs_tell(struct nfs_filesystem *nfs, struct nfs_fd *nfs_fd);
#endif

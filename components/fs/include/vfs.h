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
 * @file        vfs.h
 *
 * @brief       Header file for virtual filesystem data structure.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_H__
#define __VFS_H__

#include <oneos_config.h>
#include <os_types.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include <libc_ext.h>
#include <device.h>
#include <stdio.h>

#include <vfs_poll.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VFS_PATH_MAX                256

/* File types */
#define FT_INVALID                  0           /* Invalid type */
#define FT_REGULAR                  1           /* Regular file */
#define FT_DIRECTORY                2           /* Directory */
#define FT_DEVICE                   3           /* Device file */
#define FT_TYPEMAX                  4           /* Max type */

struct vfs_file
{
    unsigned short type;                /* Type (regular or directory) */
    int            flags;               /* Open mode flags */
    const char    *path;                /* Name (below mount point) */

    struct vfs_mountpoint *mnt_point;
    void          *desc;                /* Record the descriptor of open file. */
#ifdef OS_USING_VFS_DEVFS
    void          *private;             /* Record the private data when register dev. */
#endif
};

struct vfs_dir
{
    struct vfs_file *fp;
    struct dirent   entry;              /* Populated when readdir is called */
};

struct vfs_filesystem_ops
{
    char *fs_name;

    /* Operations for file */
    int (*open)     (struct vfs_file *fp);
    int (*close)    (struct vfs_file *fp);
    int (*read)     (struct vfs_file *fp, void *buf, size_t count);
    int (*write)    (struct vfs_file *fp, const void *buf, size_t count);
    int (*lseek)    (struct vfs_file *fp, off_t offset, int whence);
    int (*sync)     (struct vfs_file *fp);
    int (*ioctl)    (struct vfs_file *fp, unsigned long cmd, void *args);
#ifdef OS_USING_IO_MULTIPLEXING
    int (*poll)     (struct vfs_file *fp, struct vfs_pollfd *poll_fd, os_bool_t poll_setup);
#endif

    /* Operations for dirctory */
    int (*opendir)  (struct vfs_dir *dp);
    int (*closedir) (struct vfs_dir *dp);
    int (*readdir)  (struct vfs_dir *dp, struct dirent *dentry);
    int (*seekdir)  (struct vfs_dir *dp, off_t offset);
    long (*telldir) (struct vfs_dir *dp);

    /* Operations for file or dircotry */
    int (*rename)   (struct vfs_mountpoint *mnt_point, const char *oldpath, const char *newpath);
    int (*unlink)   (struct vfs_mountpoint *mnt_point, const char *path);
    int (*stat)     (struct vfs_mountpoint *mnt_point, const char *path, struct stat *buf);

    /* Operations for filesystem */
    int (*mkfs)     (void   *dev);
    int (*statfs)   (struct vfs_mountpoint *mnt_point, struct statfs *buf);
    int (*mount)    (struct vfs_mountpoint *mnt_point, unsigned long mountflag, const void *data);
    int (*unmount)  (struct vfs_mountpoint *mnt_point);
};

struct vfs_mountpoint
{
    unsigned short                      is_inited;
    unsigned short                      ref_cnt;        /* Record referecnt count of this mount point. */
    void                               *dev;            /* Attached device pointer */
    char                               *mnt_path;       /* File system mount point */
    const struct vfs_filesystem_ops    *ops;            /* Operations for file system type */
    void                               *fs_instance;    /* Actual file system */
};

extern struct vfs_file *fd_to_fp(int fd);

#ifdef __cplusplus
}
#endif

#endif

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
 * @file        vfs_posix.h
 *
 * @brief       Header file for VFS API which compliance with POSIX standard.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_POSIX_H__
#define __VFS_POSIX_H__

#include <libc_ext.h>
#include <vfs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @struct      DIR
 *
 * @brief       A structure representing a directory stream
 ***********************************************************************************************************************
 */
typedef struct vfs_dir FS_DIR;

/* File API*/
int      vfs_open(const char *path, int flags, ...);
int      vfs_close(int fd);
int      vfs_read(int fd, void *buf, size_t len);
int      vfs_write(int fd, const void *buf, size_t len);
off_t    vfs_lseek(int fd, off_t offset, int whence);
int      vfs_fsync(int fd);
int      vfs_fstat(int fd, struct stat *buf);
int      vfs_fcntl(int fd, int cmd, ...);
int      vfs_ioctl(int fd, unsigned long request, ...);

/* Directory API*/
int      vfs_mkdir(const char *path, mode_t mode);
FS_DIR  *vfs_opendir(const char *path);
int      vfs_closedir(FS_DIR *dp);
struct dirent *vfs_readdir(FS_DIR *dp);
long     vfs_telldir(FS_DIR *dp);
void     vfs_seekdir(FS_DIR *dp, off_t offset);
void     vfs_rewinddir(FS_DIR *dp);
int      vfs_chdir(const char *path);
char    *vfs_getcwd(char *buf, size_t size);
int      vfs_rmdir(const char *path);

/* File or Directory api*/
int      vfs_rename(const char *oldpath, const char *newpath);
int      vfs_unlink(const char *path);
int      vfs_access(const char *path, int amode);
int      vfs_stat(const char *path, struct stat *buf);

/* File system api */
int      vfs_statfs(const char *path, struct statfs *buf);

#ifdef __cplusplus
}
#endif

#endif

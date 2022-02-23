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
 * @file        vfs_private.h
 *
 * @brief       Declare internel interface for virtual filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef VFS_PRIVATE_H__
#define VFS_PRIVATE_H__

#include <os_mutex.h>
#include <vfs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VFS_TAG             "VFS"

#define DEVFS_PATH          "/dev"

#define VFS_FD_OFFSET       3

#define VFS_LOCK_INIT() do {                                                                        \
                            os_err_t vfslock_ret = os_mutex_init(&vfs_lock, "vfslock", OS_FALSE);   \
                            if (OS_EOK != vfslock_ret) {                                            \
                                LOG_E(VFS_TAG, "vfslock init faild:%d", vfslock_ret);               \
                            }                                                                       \
                            OS_ASSERT(OS_EOK == vfslock_ret);                                       \
                        }while(0)

#define VFS_LOCK()      do {                                                                        \
                            os_err_t vfslock_ret = os_mutex_lock(&vfs_lock, OS_WAIT_FOREVER);       \
                            if (OS_EOK != vfslock_ret) {                                            \
                                LOG_E(VFS_TAG, "vfslock lock faild:%d", vfslock_ret);               \
                            }                                                                       \
                            OS_ASSERT(OS_EOK == vfslock_ret);                                       \
                        }while(0)

#define VFS_UNLOCK()    do {\
                            os_err_t vfslock_ret = os_mutex_unlock(&vfs_lock);                      \
                            if (OS_EOK != vfslock_ret) {                                            \
                                LOG_E(VFS_TAG, "vfslock unlock faild:%d", vfslock_ret);             \
                            }                                                                       \
                            OS_ASSERT(OS_EOK == vfslock_ret);                                       \
                        }while(0)

extern os_mutex_t vfs_lock;

#define VFS_SET_ERRNO(no)       os_set_errno(-(no))

extern int do_openfile(struct vfs_file *fp, const char *path, int flags);
extern int do_closefile(struct vfs_file *fp);
extern int do_readfile(struct vfs_file *fp, void *buf, size_t len);
extern int do_writefile(struct vfs_file *fp, const void *buf, size_t len);
extern int do_lseekfile(struct vfs_file *fp, off_t offset, int whence);
extern int do_syncfile(struct vfs_file *fp);
extern int do_fcntl(struct vfs_file *fp, int cmd, void *args);
extern int do_ioctl(struct vfs_file *fp, unsigned long request, void *args);
extern int do_opendir(struct vfs_dir *dp, const char *path, int flags);
extern int do_closedir(struct vfs_dir *dp);
extern int do_readdir(struct vfs_dir *dp, struct dirent *entry);
extern int do_seekdir(struct vfs_dir *dp, off_t offset);
extern long do_telldir(struct vfs_dir *dp);
extern int do_rename(const char *oldpath, const char *newpath);
extern int do_unlink(const char *path);
extern int do_stat(const char *path, struct stat *buf);
extern int do_statfs(const char *path, struct statfs *buf);

extern void fd_table_init(void);
extern int  fd_alloc(void);
extern void fd_ref_inc(int fd);
extern void fd_ref_dec(int fd);
extern void fd_free(int fd);
extern struct vfs_dir *dp_alloc(void);
extern void dp_ref_inc(struct vfs_dir *dp);
extern void dp_ref_dec(struct vfs_dir *dp);
extern void dp_free(struct vfs_dir *dp);
extern os_bool_t dp_check_valid(struct vfs_dir *dp);

extern char *vfs_create_absolute_path(const char *directory, const char *path);
extern void  vfs_destroy_absolute_path(char *path);
extern const char *vfs_create_rel_mnt_path(const char *directory, const char *abs_path);
extern const char *vfs_get_rel_mnt_path(const char *directory, const char *abs_path);
extern char *vfs_get_path_lastname(const char *path);

extern void working_dir_init(void);
extern char *working_dir_get(char *buf, size_t size);
extern void working_dir_set(char *path);

extern struct vfs_mountpoint *vfs_mount_point_find_and_ref(const char* abspath);
extern void vfs_mount_point_deref(struct vfs_mountpoint *mnt_point);

#ifdef __cplusplus
}
#endif

#endif


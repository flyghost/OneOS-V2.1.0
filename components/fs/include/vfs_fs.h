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
 * @file        vfs_fs.h
 *
 * @brief       Declare the filesystem related APIs of vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_FS_H__
#define __VFS_FS_H__

#include <vfs.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <vfs.h>

extern int vfs_init(void);
extern int vfs_register(const struct vfs_filesystem_ops *ops);
extern int vfs_mount(const char *device_name, const char *path, const char *fs_name, unsigned long mountflag, const void *data);
extern int vfs_unmount(const char *path);
extern int vfs_mkfs(const char *fs_name, const char *device_name);

#ifdef __cplusplus
}
#endif

#endif


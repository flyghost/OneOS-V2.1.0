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
 * @file        vfs_devfs.h
 *
 * @brief       Header file for  device filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-26   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_DEVICE_FS_H__
#define __VFS_DEVICE_FS_H__

#include <os_sem.h>
#include <vfs.h>
#include <vfs_poll.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dev_file_ops
{
    int (*open)     (struct vfs_file *fp);
    int (*close)    (struct vfs_file *fp);
    int (*read)     (struct vfs_file *fp, off_t pos, void *buf, size_t count);      //TODO, do we need pos ?
    int (*write)    (struct vfs_file *fp, off_t pos, const void *buf, size_t count);//TODO, do we need pos ?
    int (*ioctl)    (struct vfs_file *fp, unsigned long cmd, void *args);
#ifdef OS_USING_IO_MULTIPLEXING
    int (*poll)     (struct vfs_file *fp, struct vfs_pollfd *poll_fd, os_bool_t poll_setup);
#endif
};

#ifdef OS_USING_IO_MULTIPLEXING
OS_INLINE os_err_t devfs_poll_notify(struct vfs_pollfd *poll_fd, os_uint16_t revents)
{
    return vfs_poll_notify(poll_fd, revents);
}
#endif

extern os_err_t devfs_register_device(const char *dev_name, struct dev_file_ops *ops, void *private);
extern os_err_t devfs_unregister_device(const char *dev_name);
extern int vfs_devfs_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __VFS_DEVICE_FS_H__ */

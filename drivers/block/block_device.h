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
 * @file        block.h
 *
 * @brief       block
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BLOCK_DEVICE_H__
#define __BLOCK_DEVICE_H__

#include <stdint.h>
#include <stdlib.h>
#include <os_list.h>
#include <device.h>

#define OS_DEVICE_CTRL_BLK_GETGEOME     IOC_BLOCK(0)        /* Get geometry information.   */
#define OS_DEVICE_CTRL_BLK_SYNC         IOC_BLOCK(1)        /* Flush data to block device. */
#define OS_DEVICE_CTRL_BLK_AUTOREFRESH  IOC_BLOCK(3)        /* Block device : enter/exit auto refresh mode. */

typedef struct os_blk_device os_blk_device_t;

struct os_blk_geometry
{
    os_uint32_t block_size;     /* block size measured in bytes */
    os_uint32_t capacity;       /* capacity measured in bytes */
};

struct os_blk_ops
{
    int (*read_block)(os_blk_device_t *blk, os_uint32_t block_addr, os_uint8_t *buff, os_uint32_t block_nr);
    int (*write_block)(os_blk_device_t *blk, os_uint32_t block_addr, const os_uint8_t *buff, os_uint32_t block_nr);
};

struct os_blk_device
{
    struct os_device         blk_dev;
    struct os_blk_geometry   geometry;
    const struct os_blk_ops *blk_ops;

    void *priv;
};

os_err_t block_device_register(os_blk_device_t *blk_dev, const char *name);
os_err_t block_device_unregister(os_blk_device_t *blk_dev);

#endif    /* __BLOCK_DEVICE_H__ */


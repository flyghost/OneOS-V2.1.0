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

#ifndef __MTD_DEVICE_H__
#define __MTD_DEVICE_H__

#include <stdint.h>
#include <stdlib.h>
#include <os_list.h>
#include <device.h>

#define OS_DEVICE_CTRL_MTD_GETGEOME     IOC_MTD(0)        /* Get geometry information.   */
#define OS_DEVICE_CTRL_MTD_SYNC         IOC_MTD(1)        /* Flush data to block device. */
#define OS_DEVICE_CTRL_MTD_ERASE        IOC_MTD(2)        /* Erase block on block device. */
#define OS_DEVICE_CTRL_MTD_AUTOREFRESH  IOC_MTD(3)        /* Block device : enter/exit auto refresh mode. */

typedef struct os_mtd_device os_mtd_device_t;

struct os_mtd_geometry
{
    os_uint32_t page_size;      /* page size measured in bytes */
    os_uint32_t block_size;     /* block size measured in bytes */
    os_uint32_t capacity;       /* capacity measured in bytes */
};

struct os_mtd_ops
{
    int (*read_page)(os_mtd_device_t *mtd, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr);
    int (*write_page)(os_mtd_device_t *mtd, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr);
    int (*erase_block)(os_mtd_device_t *mtd, os_uint32_t page_addr, os_uint32_t page_nr);
};

struct os_mtd_device
{
    struct os_device         char_dev;
    struct os_device         mtd_dev;
    struct os_mtd_geometry   geometry;
    const struct os_mtd_ops *mtd_ops;

    void *priv;
};

os_err_t mtd_device_register(os_mtd_device_t *mtd_dev, const char *name);
os_err_t mtd_device_unregister(os_mtd_device_t *mtd_dev);

#endif    /* __MTD_DEVICE_H__ */


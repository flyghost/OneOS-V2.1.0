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
 * @file        ramdisk.h
 *
 * @brief       Header file of ramdisk.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */


#ifndef __RAMDISK_H__
#define __RAMDISK_H__

#include <device.h>
#include <block/block_device.h>

#define BLOCK_MINSIZE   32

struct ramdisk_device
{
    os_blk_device_t     blk_dev;
    char               *ram_addr;       /* The start address of ramdisk. */
    os_uint32_t         ram_size;       /* The ramdisk size */
    os_uint32_t         block_cnt;      /* The total block count of ramdisk. */
    os_uint32_t         block_size;     /* The size of every block */
    os_bool_t           init_flag;      /* The inital flag */
};
typedef struct ramdisk_device ramdisk_dev_t;

extern os_err_t ramdisk_dev_init(ramdisk_dev_t *ram_dev, void *addr, const char *name, os_uint32_t size, os_uint32_t block_size);
extern void     ramdisk_dev_deinit(ramdisk_dev_t* ram_dev);
#ifdef OS_USING_SYS_HEAP
extern ramdisk_dev_t *ramdisk_dev_create(const char *name, os_uint32_t size, os_uint32_t block_size);
extern void           ramdisk_dev_destroy(ramdisk_dev_t *ram_dev);
#endif
#endif

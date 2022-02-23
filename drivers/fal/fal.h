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
 * @file        fal.h
 *
 * @brief       fal
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __FAL_H__
#define __FAL_H__

#include <stdint.h>
#include <stdlib.h>
#include <os_list.h>
#include <arch_misc.h>
#include <mtd/mtd.h>
#include <block/block_device.h>

#ifndef FAL_DEV_NAME_MAX
#define FAL_DEV_NAME_MAX 16
#endif

typedef struct fal_flash fal_flash_t;

struct fal_flash_ops
{
    int (*read_page)(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr);
    int (*write_page)(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr);
    int (*erase_block)(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr);
};

struct fal_flash
{
    char name[FAL_DEV_NAME_MAX];

    os_uint32_t page_size;      /* flash memory page (without spare area) size measured in bytes */
    os_uint32_t block_size;     /* flash memory block size measured in number of pages */
    os_uint32_t capacity;       /* flash capacity measured in bytes */
    
    struct fal_flash_ops ops;

    os_list_node_t list;
    
    void *priv;
};

struct onchip_flash_info 
{
    uint32_t start_addr;
    uint32_t capacity;
    uint32_t block_size;
    uint32_t page_size;
};

int fal_flash_register(fal_flash_t *flash);
fal_flash_t *fal_flash_find(const char *name);
os_mtd_device_t *fal_mtd_device_create(const char *parition_name);
os_blk_device_t *fal_blk_device_create(const char *parition_name);

#define fal_block_shift(flash_dev)   (os_ffs(flash_dev->block_size)-1)
#define fal_page_shift(flash_dev)    (os_ffs(flash_dev->page_size)-1)

#include <fal/fal_part.h>

#endif    /* __FAL_H__ */

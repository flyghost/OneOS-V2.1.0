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
 * @file        fal_mtd.c
 *
 * @brief       The fal test function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-20   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <fal/fal.h>
#include <os_memory.h>
#include <string.h>
#include <stdlib.h>
#include <os_clock.h>

#include <os_task.h>
#include <drv_cfg.h>
#include <dlog.h>

#define DBG_TAG   "drv.fal_block"

struct fal_mtd_device
{
    os_mtd_device_t mtd_dev;
    fal_part_t     *fal_part;
};

int fal_mtd_read_page(os_mtd_device_t *mtd, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int count;
    
    struct fal_mtd_device *fal_mtd_dev = (struct fal_mtd_device *)mtd;
    fal_part_t *part = fal_mtd_dev->fal_part;

    count = fal_part_read(part, page_addr * mtd->geometry.page_size, buff, page_nr * mtd->geometry.page_size);

    if (count != page_nr * mtd->geometry.page_size)
        return OS_ERROR;

    return OS_EOK;
}

int fal_mtd_write_page(os_mtd_device_t *mtd, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int count;
    
    struct fal_mtd_device *fal_mtd_dev = (struct fal_mtd_device *)mtd;
    fal_part_t *part = fal_mtd_dev->fal_part;

    count = fal_part_write(part, page_addr * mtd->geometry.page_size, buff, page_nr * mtd->geometry.page_size);

    if (count != page_nr * mtd->geometry.page_size)
        return OS_ERROR;

    return OS_EOK;
}

int fal_mtd_erase_block(os_mtd_device_t *mtd, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int count;
    
    struct fal_mtd_device *fal_mtd_dev = (struct fal_mtd_device *)mtd;
    fal_part_t *part = fal_mtd_dev->fal_part;

    count = fal_part_erase(part, page_addr * mtd->geometry.page_size, page_nr * mtd->geometry.page_size);

    if (count != page_nr * mtd->geometry.page_size)
        return OS_ERROR;

    return OS_EOK;
}

const static struct os_mtd_ops fal_mtd_ops = {
    .read_page   = fal_mtd_read_page,
    .write_page  = fal_mtd_write_page,
    .erase_block = fal_mtd_erase_block,
};

/**
 * create block device by specified partition
 *
 * @param parition_name partition name
 *
 * @return != NULL: created block device
 *            NULL: created failed
 */
os_mtd_device_t *fal_mtd_device_create(const char *parition_name)
{
    struct fal_mtd_device *fal_mtd_dev;
    struct fal_part *fal_part = fal_part_find(parition_name);
    const struct fal_flash *fal_flash = fal_part->flash;

    if (!fal_part)
    {
        LOG_E(DBG_TAG,"Error: the partition name (%s) is not found.", parition_name);
        return NULL;
    }

    fal_mtd_dev = (struct fal_mtd_device*) os_calloc(1, sizeof(struct fal_mtd_device));
    if (fal_mtd_dev == OS_NULL)
    {
        LOG_E(DBG_TAG,"Error: no memory for create FAL block device");
        return OS_NULL;
    }

    fal_mtd_dev->fal_part = fal_part;
    
    fal_mtd_dev->mtd_dev.geometry.page_size  = fal_flash->page_size;
    fal_mtd_dev->mtd_dev.geometry.block_size = fal_flash->block_size;
    fal_mtd_dev->mtd_dev.geometry.capacity   = fal_part->info->size;

    fal_mtd_dev->mtd_dev.mtd_ops = &fal_mtd_ops;

    mtd_device_register(&fal_mtd_dev->mtd_dev, parition_name);

    LOG_I(DBG_TAG,"The FAL mtd device (%s) created successfully", parition_name);

    return &fal_mtd_dev->mtd_dev;
}


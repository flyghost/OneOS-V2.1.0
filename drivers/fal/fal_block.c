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
 * @file        fal_block.c
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

struct fal_blk_device
{
    os_blk_device_t blk_dev;
    fal_part_t     *fal_part;
};

int fal_blk_read_block(os_blk_device_t *blk, os_uint32_t block_addr, os_uint8_t *buff, os_uint32_t block_nr)
{
    int count;
    
    struct fal_blk_device *fal_blk_dev = (struct fal_blk_device *)blk;
    fal_part_t *part = fal_blk_dev->fal_part;

    count = fal_part_read(part, block_addr * blk->geometry.block_size, buff, block_nr * blk->geometry.block_size);

    if (count != block_nr * blk->geometry.block_size)
        return OS_ERROR;

    return OS_EOK;
}

int fal_blk_write_block(os_blk_device_t *blk, os_uint32_t block_addr, const os_uint8_t *buff, os_uint32_t block_nr)
{
    int ret = OS_ERROR;
    int count;
    
    struct fal_blk_device *fal_blk_dev = (struct fal_blk_device *)blk;
    fal_part_t *part = fal_blk_dev->fal_part;
    const struct fal_flash *fal_flash = part->flash;

    os_uint32_t start_addr = blk->geometry.block_size * block_addr;
    os_uint32_t end_addr   = blk->geometry.block_size * (block_addr + block_nr) - 1;

    os_uint32_t fal_block_start_addr = start_addr & ~(fal_flash->block_size - 1);
    os_uint32_t fal_block_end_addr   = end_addr   & ~(fal_flash->block_size - 1);

    os_uint32_t fal_block_size = fal_block_end_addr - fal_block_start_addr + fal_flash->block_size;

    os_uint8_t *fal_block_buff = os_calloc(1, fal_block_size);

    OS_ASSERT(fal_block_buff != OS_NULL);

    count = fal_part_read(part, fal_block_start_addr, fal_block_buff, fal_block_size);
    if (count != fal_block_size)
        goto end;

    memcpy(fal_block_buff + start_addr % fal_flash->block_size, buff, block_nr * blk->geometry.block_size);
    
    count = fal_part_erase(part, fal_block_start_addr, fal_block_size);
    
    if (count != fal_block_size)
        goto end;

    count = fal_part_write(part, fal_block_start_addr, fal_block_buff, fal_block_size);

    if (count != fal_block_size)
        goto end;

    ret = OS_EOK;

end:
    os_free(fal_block_buff);
    return ret;
}

const static struct os_blk_ops fal_blk_ops = {
    .read_block   = fal_blk_read_block,
    .write_block  = fal_blk_write_block,
};

/**
 * create block device by specified partition
 *
 * @param parition_name partition name
 *
 * @return != NULL: created block device
 *            NULL: created failed
 */
os_blk_device_t *fal_blk_device_create(const char *parition_name)
{
    struct fal_blk_device *fal_blk_dev;
    struct fal_part *fal_part = fal_part_find(parition_name);

    if (!fal_part)
    {
        LOG_E(DBG_TAG,"Error: the partition name (%s) is not found.", parition_name);
        return NULL;
    }

    fal_blk_dev = (struct fal_blk_device*)os_calloc(1, sizeof(struct fal_blk_device));
    if (fal_blk_dev == OS_NULL)
    {
        LOG_E(DBG_TAG,"Error: no memory for create FAL block device");
        return OS_NULL;
    }

    memset(fal_blk_dev, 0, sizeof(struct fal_blk_device));
    
    fal_blk_dev->fal_part = fal_part;
    fal_blk_dev->blk_dev.geometry.block_size = 512;
    fal_blk_dev->blk_dev.geometry.capacity   = fal_part->info->size;

    fal_blk_dev->blk_dev.blk_ops = &fal_blk_ops;

    block_device_register(&fal_blk_dev->blk_dev, parition_name);

    LOG_I(DBG_TAG,"The FAL block device (%s) created successfully", parition_name);

    return &fal_blk_dev->blk_dev;
}


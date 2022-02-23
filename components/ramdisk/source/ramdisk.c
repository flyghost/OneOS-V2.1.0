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
 * @file        ramdisk.c
 *
 * @brief       This file implement ramdisk device.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <string.h>
#include <dlog.h>
#include <os_errno.h>
#include <os_memory.h>
#include <device.h>
#include "ramdisk.h"

#define RAMDISK_TAG         "RAMDISK"

static int ramdisk_blk_read_block(os_blk_device_t *blk, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    ramdisk_dev_t *ram_dev;

    if ((!blk) || (!buff))
    {        
        LOG_E(RAMDISK_TAG, "invalid paramater");
        return OS_ERROR;
    }

    ram_dev = (ramdisk_dev_t *)blk;
    if (ram_dev->init_flag == OS_FALSE)
    {
        return OS_ERROR;
    }
    if (page_addr > ram_dev->block_cnt)
    {
        LOG_E(RAMDISK_TAG, "invalid paramater, page_addr:%d", page_addr);
        return OS_ERROR;
    }

    if (page_addr + page_nr > ram_dev->block_cnt)
    {
        page_nr = ram_dev->block_cnt - page_addr;
    }

    memcpy(buff, ram_dev->ram_addr + ram_dev->block_size * page_addr, ram_dev->block_size * page_nr);

    return OS_EOK;
}

static int ramdisk_blk_write_block(os_blk_device_t *blk, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    ramdisk_dev_t *ram_dev;

    if ((!blk) || (!buff))
    {        
        LOG_E(RAMDISK_TAG, "invalid paramater");
        return OS_ERROR;
    }

    ram_dev = (ramdisk_dev_t *)blk;
    if (ram_dev->init_flag == OS_FALSE)
    {
        return OS_ERROR;
    }
    if (page_addr > ram_dev->block_cnt)
    {
        LOG_E(RAMDISK_TAG, "invalid paramater, page_addr:%d", page_addr);
        return OS_ERROR;
    }

    if (page_addr + page_nr > ram_dev->block_cnt)
    {
        page_nr = ram_dev->block_cnt - page_addr;
    }
    memcpy(ram_dev->ram_addr + ram_dev->block_size * page_addr, buff, ram_dev->block_size * page_nr);

    return OS_EOK;
}

const static struct os_blk_ops ramdisk_blk_ops = {
    .read_block   = ramdisk_blk_read_block,
    .write_block  = ramdisk_blk_write_block,
};

/**
 ***********************************************************************************************************************
 * @brief           Init ramdisk device.
 *
 * @param[in,out]   ram_dev         The pointer of ramdisk device.
 * @param[in]       addr            The start address of ramdisk.
 * @param[in]       name            Ramdisk name.
 * @param[in]       size            Ramdisk size.
 * @param[in]       block_size      Size of every block.
 *
 * @return          The initial result.
 * @retval          OS_EOK          Init successfully.
 * @retval          Others          Init failed, return the error code.
 ***********************************************************************************************************************
 */
os_err_t ramdisk_dev_init(ramdisk_dev_t *ram_dev, void *addr, const char *name, os_uint32_t size, os_uint32_t block_size)
{
    os_uint32_t block_cnt;

    block_size = OS_ALIGN_UP(block_size, 4);
    if ((!ram_dev) || (!addr) || (!name) || (block_size < BLOCK_MINSIZE) || (size < block_size))
    {
        LOG_E(RAMDISK_TAG, "invalid device_name or ramdisk_size");
        return OS_EINVAL;
    }

    ram_dev->ram_addr = (char *)OS_ALIGN_UP((os_uint32_t)addr, 4);
    size -= (ram_dev->ram_addr - (char *)addr);
    block_cnt = size/block_size;
    ram_dev->ram_size = block_cnt*block_size;
    ram_dev->block_size = block_size;
    ram_dev->block_cnt = block_cnt;
    ram_dev->init_flag = OS_TRUE;
    memset(ram_dev->ram_addr, 0, block_cnt*block_size);
    memset(&ram_dev->blk_dev, 0, sizeof(os_blk_device_t));

    ram_dev->blk_dev.geometry.block_size = ram_dev->block_size;
    ram_dev->blk_dev.geometry.capacity   = ram_dev->ram_size;
    
    ram_dev->blk_dev.blk_ops = &ramdisk_blk_ops;
    
    if(OS_EOK != block_device_register(&ram_dev->blk_dev, name))
    {
        LOG_W(RAMDISK_TAG, "create ramdisk device failed");
        return OS_ERROR;
    }

    LOG_W(RAMDISK_TAG, "create ramdisk device:%s successfully", name);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Deinit ramdisk device.
 *
 * @param[in,out]   ram_dev         The pointer of ramdisk device.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void ramdisk_dev_deinit(ramdisk_dev_t* ram_dev)
{
    if (!ram_dev)
    {
        LOG_E(RAMDISK_TAG, "invalid device");
        return;
    }

    block_device_unregister(&ram_dev->blk_dev);
    ram_dev->ram_addr = OS_NULL;
    ram_dev->ram_size = 0;
    ram_dev->block_cnt = 0;
    ram_dev->block_size = 0;    
    ram_dev->init_flag = OS_FALSE;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           Create ramdisk device, the ramdisk's memory will be dynamci allocted.
 *
 * @param[in]       name            Ramdisk name.
 * @param[in]       size            Ramdisk size.
 * @param[in]       block_size      Size of every block.
 *
 * @return          The initial result.
 * @retval          ramdisk_dev_t*  The pointer of ramdisk device..
 * @retval          OS_NULL         Create ramdisk failed.
 ***********************************************************************************************************************
 */
ramdisk_dev_t *ramdisk_dev_create(const char *name, os_uint32_t size, os_uint32_t block_size)
{
    ramdisk_dev_t *ram_dev;
    os_uint32_t block_cnt;

    block_size = OS_ALIGN_UP(block_size, 4);
    if ((!name) || (block_size < BLOCK_MINSIZE) || (size < block_size))
    {
        LOG_E(RAMDISK_TAG, "invalid device_name or ramdisk_size");
        return OS_NULL;
    }

    ram_dev = os_malloc(sizeof(ramdisk_dev_t));
    if (!ram_dev)
    {
        LOG_E(RAMDISK_TAG, "no memory for create ramdisk device");
        return OS_NULL;
    }

    block_cnt = size/block_size;
    ram_dev->ram_addr = os_malloc(block_cnt*block_size);
    if (!ram_dev->ram_addr)
    {
        os_free(ram_dev);
        LOG_E(RAMDISK_TAG, "no memory for create ramdisk");
        return OS_NULL;
    }
    memset(ram_dev->ram_addr, 0, block_cnt*block_size);

    ram_dev->ram_size = block_cnt*block_size;
    ram_dev->block_size = block_size;
    ram_dev->block_cnt = block_cnt;
    ram_dev->init_flag = OS_TRUE;

    memset(&ram_dev->blk_dev, 0, sizeof(os_blk_device_t));

    ram_dev->blk_dev.geometry.block_size = ram_dev->block_size;
    ram_dev->blk_dev.geometry.capacity   = ram_dev->ram_size;

    ram_dev->blk_dev.blk_ops = &ramdisk_blk_ops;
    
    if(OS_EOK != block_device_register(&ram_dev->blk_dev, name))
    {
        os_free(ram_dev->ram_addr);
        os_free(ram_dev);
        LOG_W(RAMDISK_TAG, "create ramdisk device failed");
        return OS_NULL;
    }

    LOG_W(RAMDISK_TAG, "create ramdisk device:%s successfully", name);

    return ram_dev;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy ramdisk device, the ramdisk's memory will be free.
 *
 * @param[in,out]   ram_dev         The pointer of ramdisk device.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void ramdisk_dev_destroy(ramdisk_dev_t *ram_dev)
{
    if (!ram_dev)
    {
        LOG_E(RAMDISK_TAG, "invalid device");
        return;
    }

    block_device_unregister(&ram_dev->blk_dev);
    os_free(ram_dev->ram_addr);
    os_free(ram_dev);
}
#endif

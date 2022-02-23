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
 * @file        block.c
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <fal/fal.h>
#include <os_memory.h>
#include <string.h>
#include <stdlib.h>
#include <os_clock.h>
#include <arch_misc.h>
#include <os_task.h>
#include <drv_cfg.h>
#include <dlog.h>

#define DBG_TAG "drv.block"

static os_size_t blk_dev_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    int ret;
    os_blk_device_t *blk_dev = (os_blk_device_t *)os_container_of(dev, os_blk_device_t, blk_dev);

    os_uint32_t block_addr = pos;
    os_uint32_t block_nr   = size;

    ret = blk_dev->blk_ops->read_block(blk_dev, block_addr, buffer, block_nr);

    if (ret == OS_EOK)
        return size;

    return 0;
}

static os_size_t blk_dev_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    int ret;
    os_blk_device_t *blk_dev = (os_blk_device_t *)os_container_of(dev, os_blk_device_t, blk_dev);

    os_uint32_t block_addr = pos;
    os_uint32_t block_nr   = size;
    
    ret = blk_dev->blk_ops->write_block(blk_dev, block_addr, buffer, block_nr);

    if (ret == OS_EOK)
        return size;

    return 0;
}

static os_err_t blk_dev_control(os_device_t *dev, int cmd, void *args)
{
    os_blk_device_t *blk_dev = (os_blk_device_t *)os_container_of(dev, os_blk_device_t, blk_dev);

    OS_ASSERT(blk_dev != OS_NULL);

    if (cmd == OS_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct os_blk_geometry *geometry;

        geometry = (struct os_blk_geometry *) args;
        if (geometry == OS_NULL)
        {
            return OS_ERROR;
        }

        memcpy(geometry, &blk_dev->geometry, sizeof(struct os_blk_geometry));
    }

    return OS_EOK;
}

const static struct os_device_ops blk_ops = {
    .read    = blk_dev_read,
    .write   = blk_dev_write,
    .control = blk_dev_control,
};

os_err_t block_device_register(os_blk_device_t *blk_dev, const char *name)
{
    OS_ASSERT(blk_dev->blk_ops != OS_NULL);
    OS_ASSERT(blk_dev->blk_ops->read_block != OS_NULL);
    OS_ASSERT(blk_dev->blk_ops->write_block != OS_NULL);

    /* block device */
    blk_dev->blk_dev.type = OS_DEVICE_TYPE_BLOCK;
    blk_dev->blk_dev.ops  = &blk_ops;

    os_device_register(&blk_dev->blk_dev, name);

    return OS_EOK;
}

os_err_t block_device_unregister(os_blk_device_t *blk_dev)
{
    os_device_unregister(&blk_dev->blk_dev);
    return OS_EOK;
}


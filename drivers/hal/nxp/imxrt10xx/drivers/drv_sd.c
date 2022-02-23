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
 * @file        drv_sd.c
 *
 * @brief       This file provides sd card device register.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_memory.h>
#include <block/block_device.h>
#include <dlog.h>
#include "fsl_sd.h"

#include <dma_ram.h>

#include <drv_usdhc.h>
#include <drv_sd.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sd"
#include <drv_log.h>

#define DATA_BLOCK_COUNT (62333952U)

struct imxrt_sd_device
{
    os_blk_device_t         blk_dev;
    struct os_imxrt_usdhc  *imxrt_usdhc;
};

void imxrt_sd_callback(bool isInserted, void *userData)
{
    
}

static os_err_t imxrt_sd_init(struct imxrt_sd_device *sd_dev)
{
    sd_dev->imxrt_usdhc->callback = imxrt_sd_callback;
    sd_dev->imxrt_usdhc->userData = sd_dev;

    if (sd_dev->imxrt_usdhc->ops->init == OS_NULL)
    {
        return OS_ERROR;
    }
    
    if (sd_dev->imxrt_usdhc->ops->init(sd_dev->imxrt_usdhc) != OS_EOK)
    {
        return OS_ERROR;
    }

    
    if (SD_Init(sd_dev->imxrt_usdhc->sd_card) != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "SD card init fail");
        return OS_ERROR;
    }
    
    return OS_EOK;
}

static int os_imxrt_sd_read_block(os_blk_device_t *blk, os_uint32_t block_addr, os_uint8_t *buff, os_uint32_t block_nr)
{
    OS_ASSERT(blk != OS_NULL);

    struct imxrt_sd_device *sd_dev  = (struct imxrt_sd_device *)blk;

    if (kStatus_Success != SD_ReadBlocks(sd_dev->imxrt_usdhc->sd_card, buff, block_addr, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "read addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    return OS_EOK;
}

static int os_imxrt_sd_write_block(os_blk_device_t *blk, os_uint32_t block_addr, const os_uint8_t *buff, os_uint32_t block_nr)
{
    OS_ASSERT(blk != OS_NULL);

    struct imxrt_sd_device *sd_dev  = (struct imxrt_sd_device *)blk;

    if (kStatus_Success != SD_EraseBlocks(sd_dev->imxrt_usdhc->sd_card, block_addr, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "erase addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    if (kStatus_Success != SD_WriteBlocks(sd_dev->imxrt_usdhc->sd_card, buff, block_addr, block_nr))
    {
        LOG_E(DRV_EXT_TAG, "write addr: %d, count: %d", block_addr, block_nr);
        return OS_ERROR;
    }

    return OS_EOK;
}

const static struct os_blk_ops sd_blk_ops = {
    .read_block   = os_imxrt_sd_read_block,
    .write_block  = os_imxrt_sd_write_block,
};

os_err_t imxrt_usdhc_sd_register(struct os_imxrt_usdhc *imxrt_usdhc)
{
    int ret;
    char sd_dev_name[4] = "sd0";

    struct imxrt_sd_device *sd_dev = os_calloc(1, sizeof(struct imxrt_sd_device));
    if (sd_dev == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "sd_dev memory call failed!");
        return OS_ENOMEM;
    }

    sd_dev->imxrt_usdhc = imxrt_usdhc;

    if (imxrt_sd_init(sd_dev) != OS_EOK)
    {
        return OS_ERROR;
    }

    if (imxrt_usdhc->usdhc_info->base == USDHC1)
    {
        sd_dev_name[2] = '0';
    }
    else if (imxrt_usdhc->usdhc_info->base == USDHC2)
    {
        sd_dev_name[2] = '1';
    }

    sd_dev->blk_dev.geometry.block_size = FSL_SDMMC_DEFAULT_BLOCK_SIZE;
    sd_dev->blk_dev.geometry.capacity   = DATA_BLOCK_COUNT * FSL_SDMMC_DEFAULT_BLOCK_SIZE;
    sd_dev->blk_dev.blk_ops = &sd_blk_ops;
    
    return block_device_register(&sd_dev->blk_dev, &sd_dev_name[0]);
}



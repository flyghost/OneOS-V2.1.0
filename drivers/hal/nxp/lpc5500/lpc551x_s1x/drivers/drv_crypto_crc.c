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
 * @file        drv_crypto.c
 *
 * @brief       This file implements lpc crypto driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include "drv_crypto.h"
#include <board.h>
#include <os_memory.h>

static os_int32_t lpc_crc_config(struct hwcrypto_crc *ctx)
{
    lpc_hwcrypto_device_t *lpc_hw_dev = (lpc_hwcrypto_device_t *)ctx->parent.device->user_data;

    return 0;
}

static os_uint32_t lpc_crc_update(struct hwcrypto_crc *ctx, const os_uint8_t *in, os_size_t length)
{
    lpc_hwcrypto_device_t *lpc_hwcrypto = (lpc_hwcrypto_device_t *)ctx->parent.device->user_data;

    os_mutex_lock(&lpc_hwcrypto->mutex, OS_WAIT_FOREVER);
    CRC_WriteData(lpc_hwcrypto->crc_info->crc_base, in, length);
    os_mutex_unlock(&lpc_hwcrypto->mutex);

    if (lpc_hwcrypto->crc_info->crc_config->polynomial == kCRC_Polynomial_CRC_32)
        return CRC_Get32bitResult(lpc_hwcrypto->crc_info->crc_base);
    else
        return CRC_Get16bitResult(lpc_hwcrypto->crc_info->crc_base);
}

static const struct hwcrypto_crc_ops crc_ops = {
    .config = lpc_crc_config,
    .update = lpc_crc_update,
};

static os_err_t lpc_crc_crypto_create(struct os_hwcrypto_ctx *ctx)
{
    lpc_hwcrypto_device_t *lpc_hwcrypto = (lpc_hwcrypto_device_t *)ctx->device->user_data;
    
//    CRC_Init(lpc_hwcrypto->crc_info->crc_base, lpc_hwcrypto->crc_info->crc_config);

    ctx->contex = lpc_hwcrypto->crc_info->crc_base;
    ((struct hwcrypto_crc *)ctx)->ops = &crc_ops;
    
    return OS_EOK;
}

static void lpc_crc_crypto_destroy(struct os_hwcrypto_ctx *ctx)
{
    lpc_hwcrypto_device_t *lpc_hwcrypto = (lpc_hwcrypto_device_t *)ctx->device->user_data;
    
    CRC_Deinit(lpc_hwcrypto->crc_info->crc_base);
}

static os_err_t lpc_crc_crypto_clone(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    if (des->contex && src->contex)
    {
        memcpy(des->contex, src->contex, sizeof(CRC_Type));
    }

    return OS_EOK;
}

static void lpc_crc_crypto_reset(struct os_hwcrypto_ctx *ctx)
{
    lpc_hwcrypto_device_t *lpc_hwcrypto = (lpc_hwcrypto_device_t *)ctx->device->user_data;
    
    CRC_Reset(lpc_hwcrypto->crc_info->crc_base);
}

static const struct os_hwcrypto_ops _ops = {
    .create  = lpc_crc_crypto_create,
    .destroy = lpc_crc_crypto_destroy,
    .copy    = lpc_crc_crypto_clone,
    .reset   = lpc_crc_crypto_reset,
};

static int lpc_crc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    lpc_hwcrypto_device_t *lpc_hwcrypto = os_calloc(1, sizeof(lpc_hwcrypto_device_t));

    OS_ASSERT(lpc_hwcrypto);

    struct os_hwcrypto_device *hwcrypto = &lpc_hwcrypto->hwcrypto;

    lpc_hwcrypto->crc_info = (struct lpc_crc_engine_info *)dev->info;

    hwcrypto->ops = &_ops;

//    hwcrypto->id = lpc_hwcrypto_uid();

    hwcrypto->user_data = hwcrypto;

    if (os_hwcrypto_crc_register(hwcrypto, dev->name) != OS_EOK)
    {
        os_kprintf("nxp crc probe failed %s.\r\n", dev->name);
        os_free(lpc_hwcrypto);
        return OS_ERROR;
    }
    
    os_mutex_init(&lpc_hwcrypto->mutex, OS_HWCRYPTO_DEFAULT_NAME, OS_FALSE);
    return OS_EOK;
}

OS_DRIVER_INFO lpc_crc_driver = {
    .name   = "CRC_ENGINE_Type",
    .probe  = lpc_crc_probe,
};

OS_DRIVER_DEFINE(lpc_crc_driver, DEVICE, OS_INIT_SUBLEVEL_LOW);


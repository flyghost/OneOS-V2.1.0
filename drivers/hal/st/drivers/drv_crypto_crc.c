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
 * @brief       This file implements stm32 crypto driver.
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

#undef STM32_CRC_PARAM_FIX
#ifndef DEFAULT_POLYNOMIAL_DISABLE
#define STM32_CRC_PARAM_FIX
#endif

#ifdef STM32_CRC_PARAM_FIX
static os_int32_t stm32_crc_config(struct hwcrypto_crc *ctx)
{
    if (ctx->crc_cfg.last_val != 0xFFFFFFFF 
    || ctx->crc_cfg.poly != 0x04C11DB7 
    || ctx->crc_cfg.width != 32)
    {
        os_kprintf("only support crc param:\r\n"
                   "last_val: 0xFFFFFFFF\r\n"
                   "poly    : 0x04C11DB7 \r\n"
                   "width   : 32\r\n");
        
        os_kprintf("unsupport crc param:\r\n"
                   "last_val: 0x%08x\r\n"
                   "poly    : 0x%08x\r\n"
                   "width   : 0x%08x\r\n", 
                   ctx->crc_cfg.last_val, 
                   ctx->crc_cfg.poly, 
                   ctx->crc_cfg.width);
        return -1;
    }

    return 0;
}

static os_uint8_t reverse_byte_bit(os_uint8_t n)
{
    n = ((n >> 1) & 0x55) | ((n << 1) & 0xaa);
    n = ((n >> 2) & 0x33) | ((n << 2) & 0xcc);
    n = ((n >> 4) & 0x0f) | ((n << 4) & 0xf0);

    return n;
}

static os_uint32_t reverse_word_bit(os_uint32_t n)
{
    n = ((n >> 1) & 0x55555555) | ((n << 1) & 0xaaaaaaaa);
    n = ((n >> 2) & 0x33333333) | ((n << 2) & 0xcccccccc);
    n = ((n >> 4) & 0x0f0f0f0f) | ((n << 4) & 0xf0f0f0f0);
    n = ((n >> 8) & 0x00ff00ff) | ((n << 8) & 0xff00ff00);
    n = ((n >> 16) & 0x0000ffff) | ((n << 16) & 0xffff0000);

    return n;
}

static os_uint32_t stm32_crc_update(struct hwcrypto_crc *ctx, const os_uint8_t *in, os_size_t length)
{
    os_uint32_t result = 0;
    struct stm32_hwcrypto_device *stm32_hw_dev = (struct stm32_hwcrypto_device *)ctx->parent.device->user_data;
    CRC_HandleTypeDef *hcrc = (CRC_HandleTypeDef *)(ctx->parent.contex);

    if (length % 4 != 0)
    {
        os_kprintf("crc length must be 4 bytes align.\r\n");
        return 0;
    }

    os_mutex_lock(&stm32_hw_dev->mutex, OS_WAIT_FOREVER);

    if (HAL_CRC_STATE_READY != HAL_CRC_GetState(hcrc))
        goto _exit;

    int i;
    os_uint32_t tmp;
    for (i = 0; i < length / 4; i++, in += 4)
    {
        if (ctx->crc_cfg.flags & CRC_FLAG_REFIN)
            tmp = (reverse_byte_bit(in[0]) << 24) | (reverse_byte_bit(in[1]) << 16) 
                | (reverse_byte_bit(in[2]) <<  8) | (reverse_byte_bit(in[3]) << 0);
        else
            tmp = (in[0] << 24) | (in[1] << 16) | (in[2] << 8) | (in[3] << 0);
        
        result = HAL_CRC_Accumulate(hcrc, &tmp, 1);
    }

    if (ctx->crc_cfg.flags & CRC_FLAG_REFOUT)
        result = reverse_word_bit(result);

    ctx->crc_cfg.last_val = result;
    
    result = result ^ ctx->crc_cfg.xorout;

_exit:
    os_mutex_unlock(&stm32_hw_dev->mutex);

    return result;
}

#else
static os_int32_t stm32_crc_config(struct hwcrypto_crc *ctx)
{
    struct stm32_hwcrypto_device *stm32_hw_dev = (struct stm32_hwcrypto_device *)ctx->parent.device->user_data;
    CRC_HandleTypeDef *hcrc = (CRC_HandleTypeDef *)(ctx->parent.contex);

    hcrc->Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc->Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE;
    hcrc->InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;
    
    hcrc->Init.GeneratingPolynomial = ctx->crc_cfg.poly;
    hcrc->Init.InitValue = ctx->crc_cfg.last_val;

    switch (ctx->crc_cfg.width)
    {
    case 32:
        hcrc->Init.CRCLength = CRC_POLYLENGTH_32B;
        break;
    case 16:
        hcrc->Init.CRCLength = CRC_POLYLENGTH_16B;
        break;
    case 8:
        hcrc->Init.CRCLength = CRC_POLYLENGTH_8B;
        break;
    case 7:
        hcrc->Init.CRCLength = CRC_POLYLENGTH_7B;
        break;
    default:
        os_kprintf("invalide crc width %d, support 7, 8, 16, 32.\r\n", ctx->crc_cfg.width);
        return -1;
    }

    if (ctx->crc_cfg.flags & CRC_FLAG_REFIN)
    {
        hcrc->Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
    }
    else
    {
        hcrc->Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
    }

    if (ctx->crc_cfg.flags & CRC_FLAG_REFOUT)
    {
        hcrc->Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    }
    else
    {
        hcrc->Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    }

    os_mutex_lock(&stm32_hw_dev->mutex, OS_WAIT_FOREVER);
    HAL_CRC_Init(hcrc);
    os_mutex_unlock(&stm32_hw_dev->mutex);

    return 0;
}

static os_uint32_t stm32_crc_update(struct hwcrypto_crc *ctx, const os_uint8_t *in, os_size_t length)
{
    os_uint32_t result = 0;
    struct stm32_hwcrypto_device *stm32_hw_dev = (struct stm32_hwcrypto_device *)ctx->parent.device->user_data;
    CRC_HandleTypeDef *hcrc = (CRC_HandleTypeDef *)(ctx->parent.contex);

    os_mutex_lock(&stm32_hw_dev->mutex, OS_WAIT_FOREVER);

    if (HAL_CRC_STATE_READY != HAL_CRC_GetState(hcrc))
        goto _exit;

    result = HAL_CRC_Accumulate(hcrc, (os_uint32_t *)in, length);


    ctx->crc_cfg.last_val = result;
    
    result = result ^ ctx->crc_cfg.xorout;

_exit:
    os_mutex_unlock(&stm32_hw_dev->mutex);

    return result;
}

#endif

static const struct hwcrypto_crc_ops crc_ops = {
    .config = stm32_crc_config,
    .update = stm32_crc_update,
};

static os_err_t stm32_crc_crypto_create(struct os_hwcrypto_ctx *ctx)
{
    struct stm32_hwcrypto_device *stm32_hw_dev = (struct stm32_hwcrypto_device *)ctx->device->user_data;

    CRC_HandleTypeDef *hcrc = stm32_hw_dev->handle;
    OS_ASSERT(hcrc != OS_NULL);

    HAL_CRC_Init(hcrc);

    ctx->contex = hcrc;
    ((struct hwcrypto_crc *)ctx)->ops = &crc_ops;

    return OS_EOK;
}

static void stm32_crc_crypto_destroy(struct os_hwcrypto_ctx *ctx)
{
    __HAL_CRC_DR_RESET((CRC_HandleTypeDef *)ctx->contex);
    HAL_CRC_DeInit((CRC_HandleTypeDef *)(ctx->contex));
}

static os_err_t stm32_crc_crypto_clone(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    if (des->contex && src->contex)
    {
        memcpy(des->contex, src->contex, sizeof(CRC_HandleTypeDef));
    }

    return OS_EOK;
}

static void stm32_crc_crypto_reset(struct os_hwcrypto_ctx *ctx)
{
    __HAL_CRC_DR_RESET((CRC_HandleTypeDef *)ctx->contex);
}

static const struct os_hwcrypto_ops _ops = {
    .create  = stm32_crc_crypto_create,
    .destroy = stm32_crc_crypto_destroy,
    .copy    = stm32_crc_crypto_clone,
    .reset   = stm32_crc_crypto_reset,
};

static int stm32_crc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_hwcrypto_device *st_hwcrypto = os_calloc(1, sizeof(struct stm32_hwcrypto_device));

    OS_ASSERT(st_hwcrypto);

    struct os_hwcrypto_device *hwcrypto = &st_hwcrypto->hwcrypto;

    st_hwcrypto->handle = (void *)dev->info;

    hwcrypto->ops = &_ops;

    hwcrypto->id = stm32_hwcrypto_uid();

    hwcrypto->user_data = hwcrypto;

    if (os_hwcrypto_crc_register(hwcrypto, dev->name) != OS_EOK)
    {
        os_kprintf("stm32 crc probe failed %s.\r\n", dev->name);
        os_free(st_hwcrypto);
        return -1;
    }
    
    os_mutex_init(&st_hwcrypto->mutex, OS_HWCRYPTO_DEFAULT_NAME, OS_FALSE);
    return 0;
}

OS_DRIVER_INFO stm32_crc_driver = {
    .name   = "CRC_HandleTypeDef",
    .probe  = stm32_crc_probe,
};

OS_DRIVER_DEFINE(stm32_crc_driver,DEVICE,OS_INIT_SUBLEVEL_MIDDLE);


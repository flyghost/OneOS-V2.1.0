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

static os_uint32_t stm32_rng_rand(struct hwcrypto_rng *ctx)
{
    os_uint32_t gen_random = 0;

    RNG_HandleTypeDef *HW_TypeDef = (RNG_HandleTypeDef *)(ctx->parent.contex);

    struct stm32_hwcrypto_device *stm32_hw_dev = (struct stm32_hwcrypto_device *)ctx->parent.device->user_data;

    os_mutex_lock(&stm32_hw_dev->mutex, OS_WAIT_FOREVER);

    if (HAL_RNG_GenerateRandomNumber(HW_TypeDef, &gen_random) != HAL_OK)
    {
        gen_random = 0;
    }

    os_mutex_unlock(&stm32_hw_dev->mutex);

    return gen_random;
}

static const struct hwcrypto_rng_ops rng_ops = {
    .update = stm32_rng_rand,
};

static os_err_t stm32_rng_crypto_create(struct os_hwcrypto_ctx *ctx)
{
    struct stm32_hwcrypto_device *stm32_hw_dev = (struct stm32_hwcrypto_device *)ctx->device->user_data;

    RNG_HandleTypeDef *hrng = (RNG_HandleTypeDef *)stm32_hw_dev->handle;
    OS_ASSERT(hrng != OS_NULL);

    if (HAL_RNG_Init(hrng) != HAL_OK)
        return OS_ERROR;
    
    ctx->contex = hrng;
    ((struct hwcrypto_rng *)ctx)->ops = &rng_ops;

    return OS_EOK;
}

static void stm32_rng_crypto_destroy(struct os_hwcrypto_ctx *ctx)
{
    
}

static os_err_t stm32_rng_crypto_clone(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    if (des->contex && src->contex)
    {
        memcpy(des->contex, src->contex, sizeof(RNG_HandleTypeDef));
    }

    return OS_EOK;
}

static void stm32_rng_crypto_reset(struct os_hwcrypto_ctx *ctx)
{

}

static const struct os_hwcrypto_ops _ops = {
    .create  = stm32_rng_crypto_create,
    .destroy = stm32_rng_crypto_destroy,
    .copy    = stm32_rng_crypto_clone,
    .reset   = stm32_rng_crypto_reset,
};

static int stm32_rng_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    struct stm32_hwcrypto_device *st_hwcrypto = os_calloc(1, sizeof(struct stm32_hwcrypto_device));

    OS_ASSERT(st_hwcrypto);

    struct os_hwcrypto_device *hwcrypto = &st_hwcrypto->hwcrypto;

    st_hwcrypto->handle = (void *)dev->info;

    hwcrypto->ops = &_ops;

    hwcrypto->id = stm32_hwcrypto_uid();

    hwcrypto->user_data = hwcrypto;

    if (os_hwcrypto_rng_register(hwcrypto, dev->name) != OS_EOK)
    {
        os_kprintf("stm32 rng probe failed %s.\r\n", dev->name);
        os_free(st_hwcrypto);
        return -1;
    }
    
    os_mutex_init(&st_hwcrypto->mutex, OS_HWCRYPTO_DEFAULT_NAME, OS_FALSE);
    return 0;
}

OS_DRIVER_INFO stm32_rng_driver = {
    .name   = "RNG_HandleTypeDef",
    .probe  = stm32_rng_probe,
};

OS_DRIVER_DEFINE(stm32_rng_driver,DEVICE,OS_INIT_SUBLEVEL_MIDDLE);


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
 * @file        hw_rng.c
 *
 * @brief       This file provides interfaces for hardware RNG.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <hwcrypto/hw_rng.h>
#include <os_errno.h>
#include <driver.h>

static struct os_hwcrypto_device *hwcrypto_rng_device = OS_NULL;

static struct os_hwcrypto_ctx *ctx_default = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           Get random numbers.
 *
 * @param[in]       ctx             Crypto context.
 *
 * @return
 * @retval          0               Fail.
 * @retval          Others          Succeed.
 ***********************************************************************************************************************
 */
os_uint32_t os_hwcrypto_rng_update_ctx(struct os_hwcrypto_ctx *ctx)
{
    if (ctx)
    {
        return ((struct hwcrypto_rng *)ctx)->ops->update((struct hwcrypto_rng *)ctx);
    }
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Get random numbers.
 *
 * @return
 * @retval          0               Fail.
 * @retval          Others          Succeed.
 ***********************************************************************************************************************
 */
os_uint32_t os_hwcrypto_rng_update(void)
{
    if (ctx_default == OS_NULL)
    {
        os_kprintf("rng contex invalid.\r\n");
        return 0;
    }

    return os_hwcrypto_rng_update_ctx(ctx_default);
}

/**
***********************************************************************************************************************
* @brief           Register crypto device.
*
* @param[in]       device          Crypto device.
* @param[in]       name            Device name.
*
* @return
* @retval          OS_EOK          Succeed.
* @retval          Others          Fail.
***********************************************************************************************************************
*/
os_err_t os_hwcrypto_rng_register(struct os_hwcrypto_device *device, const char *name)
{
    if (hwcrypto_rng_device != OS_NULL)
    {
        os_kprintf("hwcrypto rng dev %s exist, %s register failed.\r\n",
                   device_name(&hwcrypto_rng_device->parent), name);
        return OS_EFULL;
    }

    hwcrypto_rng_device = device;

    os_hwcrypto_register(device, name);

    ctx_default = os_hwcrypto_ctx_create(hwcrypto_rng_device, HWCRYPTO_TYPE_RNG, sizeof(struct hwcrypto_rng));
    if (ctx_default == OS_NULL)
        return OS_ERROR;

    return OS_EOK;
}


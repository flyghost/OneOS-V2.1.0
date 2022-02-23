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
 * @file        hw_hash.c
 *
 * @brief       This file provides interfaces for hardware hash crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <hw_hash.h>

static struct os_hwcrypto_device *hwcrypto_hash_device = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           Create hash context.
 *
 * @param[in]       device          Crypto device.
 * @param[in]       type            Type of hash context.
 *
 * @return          Pointer to crypto context.
 ***********************************************************************************************************************
 */
struct os_hwcrypto_ctx *os_hwcrypto_hash_create(hwcrypto_type type)
{
    struct os_hwcrypto_ctx *ctx;

    if (hwcrypto_hash_device == OS_NULL)
        return OS_NULL;

    ctx = os_hwcrypto_ctx_create(hwcrypto_hash_device, type, sizeof(struct hwcrypto_hash));
    return ctx;
}

/**
 ***********************************************************************************************************************
 * @brief           Detroy hash context.
 *
 * @param[in]       ctx             Crypto context.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_hash_destroy(struct os_hwcrypto_ctx *ctx)
{
    os_hwcrypto_ctx_destroy(ctx);
}

/**
 ***********************************************************************************************************************
 * @brief           Get the final hash value.
 *
 * @param[in]       ctx             Crypto context.
 * @param[out]      output          Hash value buffer.
 * @param[in]       length          Hash value buffer length.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_hash_finish(struct os_hwcrypto_ctx *ctx, os_uint8_t *output, os_size_t length)
{
    if (ctx && ((struct hwcrypto_hash *)ctx)->ops->finish)
    {
        return ((struct hwcrypto_hash *)ctx)->ops->finish((struct hwcrypto_hash *)ctx, output, length);
    }
    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           Process data.
 *
 * @param[in]       ctx             Crypto context.
 * @param[in]       input           Data buffer to be processed.
 * @param[in]       length          Data buffer length.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_hash_update(struct os_hwcrypto_ctx *ctx, const os_uint8_t *input, os_size_t length)
{
    if (ctx && ((struct hwcrypto_hash *)ctx)->ops->update)
    {
        return ((struct hwcrypto_hash *)ctx)->ops->update((struct hwcrypto_hash *)ctx, input, length);
    }
    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           Copy hash crypto context.
 *
 * @param[out]      des             Destination of crypto context.
 * @param[in]       src             Source of crypto context.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_hash_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    return os_hwcrypto_ctx_cpy(des, src);
}

/**
 ***********************************************************************************************************************
 * @brief           Reset hash crypto context.
 *
 * @param[in]       ctx             Crypto context.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_hash_reset(struct os_hwcrypto_ctx *ctx)
{
    os_hwcrypto_ctx_reset(ctx);
}

/**
 ***********************************************************************************************************************
 * @brief           Set hash crypto context type.
 *
 * @param[in]       ctx             Crypto context.
 * @param[in]       type            Type.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_hash_set_type(struct os_hwcrypto_ctx *ctx, hwcrypto_type type)
{
    return os_hwcrypto_set_type(ctx, type);
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
os_err_t os_hwcrypto_hash_register(struct os_hwcrypto_device *device, const char *name)
{
    if (hwcrypto_hash_device != OS_NULL)
    {
        os_kprintf("hwcrypto hash dev %s exist, %s register failed.\r\n",
                   device_name(&hwcrypto_hash_device->parent), name);
        return OS_EFULL;
    }

    hwcrypto_hash_device = device;

    return os_hwcrypto_register(device, name);
}


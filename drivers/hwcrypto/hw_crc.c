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
 * @file        hw_crc.c
 *
 * @brief       This file provide interfaces for hardware CRC crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_util.h>
#include <os_errno.h>
#include <hwcrypto/hw_crc.h>

static struct os_hwcrypto_device *hwcrypto_crc_device = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           Create CRC context.
 *
 * @param[in]       device          Crypto device.
 * @param[in]       mode            Custom CRC mode.
 *
 * @return
 * @retval          OS_NULL         Fail.
 * @retval          Ohters          Pointer to crypto context
 ***********************************************************************************************************************
 */
struct os_hwcrypto_ctx *os_hwcrypto_crc_create(hwcrypto_crc_mode mode)
{
    struct hwcrypto_crc *crc_ctx;

    if (hwcrypto_crc_device == OS_NULL)
        return OS_NULL;

    crc_ctx = (struct hwcrypto_crc *)os_hwcrypto_ctx_create(hwcrypto_crc_device, 
                                                            HWCRYPTO_TYPE_CRC, 
                                                            sizeof(struct hwcrypto_crc));
    if (crc_ctx == OS_NULL)
    {
        return OS_NULL;
    }

    switch (mode)
    {
    case HWCRYPTO_CRC_CRC8:
    {
        struct hwcrypto_crc_cfg temp = HWCRYPTO_CRC8_CFG;
        crc_ctx->crc_cfg             = temp;
        break;
    }
    case HWCRYPTO_CRC_CRC16:
    {
        struct hwcrypto_crc_cfg temp = HWCRYPTO_CRC16_CFG;
        crc_ctx->crc_cfg             = temp;
        break;
    }
    case HWCRYPTO_CRC_CRC32:
    {
        struct hwcrypto_crc_cfg temp = HWCRYPTO_CRC32_CFG;
        crc_ctx->crc_cfg             = temp;
        break;
    }
    case HWCRYPTO_CRC_CCITT:
    {
        struct hwcrypto_crc_cfg temp = HWCRYPTO_CRC_CCITT_CFG;
        crc_ctx->crc_cfg             = temp;
        break;
    }
    case HWCRYPTO_CRC_DNP:
    {
        struct hwcrypto_crc_cfg temp = HWCRYPTO_CRC_DNP_CFG;
        crc_ctx->crc_cfg             = temp;
        break;
    }
    default:
        break;
    }

    return &crc_ctx->parent;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy CRC context.
 *
 * @param[in]       ctx             Crypto context.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_crc_destroy(struct os_hwcrypto_ctx *ctx)
{
    os_hwcrypto_ctx_destroy(ctx);
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
 * @retval          OS_OK           Succeed.
 * @retval          Ohters          Fail.
 ***********************************************************************************************************************
 */
os_uint32_t os_hwcrypto_crc_update(struct os_hwcrypto_ctx *ctx, const os_uint8_t *input, os_size_t length)
{
    struct hwcrypto_crc *crc_ctx = (struct hwcrypto_crc *)ctx;
    if (ctx && crc_ctx->ops->update)
    {
        return crc_ctx->ops->update(crc_ctx, input, length);
    }
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Configration for CRC crypto context.
 *
 * @param[in,out]   ctx             Crypto context.
 * @param[in]       cfg             CRC configration.
 *
 * @return          None.
 ***********************************************************************************************************************
 */

void os_hwcrypto_crc_cfg(struct os_hwcrypto_ctx *ctx, const struct hwcrypto_crc_cfg *cfg)
{
    struct hwcrypto_crc *crc_ctx = (struct hwcrypto_crc *)ctx;

    if (cfg)
    {
        crc_ctx->crc_cfg = *cfg;
    }

    if (crc_ctx->ops->config)
    {
        crc_ctx->ops->config(crc_ctx);
    }
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
os_err_t os_hwcrypto_crc_register(struct os_hwcrypto_device *device, const char *name)
{
    if (hwcrypto_crc_device != OS_NULL)
    {
        os_kprintf("hwcrypto crc dev %s exist, %s register failed.\r\n",
                   device_name(&hwcrypto_crc_device->parent), name);
        return OS_EFULL;
    }

    hwcrypto_crc_device = device;

    return os_hwcrypto_register(device, name);
}


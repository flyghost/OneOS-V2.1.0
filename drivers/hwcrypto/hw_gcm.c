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
 * @file        hw_gcm.c
 *
 * @brief       This file provide interfaces for hardware GCM crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <hw_gcm.h>

static struct os_hwcrypto_device *hwcrypto_gcm_device = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           Create GCM context.
 *
 * @param[in]       device          Crypto device.
 * @param[in]       type            Type of GCM context.
 *
 * @return          Pointer to crypto context.
 ***********************************************************************************************************************
 */
struct os_hwcrypto_ctx *os_hwcrypto_gcm_create(hwcrypto_type crypt_type)
{
    struct os_hwcrypto_ctx *ctx;

    if (hwcrypto_gcm_device == OS_NULL)
        return OS_NULL;

    ctx = os_hwcrypto_ctx_create(hwcrypto_gcm_device, HWCRYPTO_TYPE_GCM, sizeof(struct hwcrypto_gcm));
    if (ctx)
    {
        ((struct hwcrypto_gcm *)ctx)->crypt_type = crypt_type;
    }
    return ctx;
}

/**
 ***********************************************************************************************************************
 * @brief           Detroy GCM context.
 *
 * @param[in]       ctx             Crypto context.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_gcm_destroy(struct os_hwcrypto_ctx *ctx)
{
    os_hwcrypto_ctx_destroy(ctx);
}

/**
 ***********************************************************************************************************************
 * @brief           Start GCM crypto.
 *
 * @param[in]       ctx             Crypto context.
 * @param[in]       add             The buffer holding the additional data.
 * @param[in]       add_len         The length of the additional data.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_start(struct os_hwcrypto_ctx *ctx, const os_uint8_t *add, os_size_t add_len)
{
    struct hwcrypto_gcm *gcm_ctx = (struct hwcrypto_gcm *)ctx;

    if (gcm_ctx && gcm_ctx->ops->start)
    {
        return gcm_ctx->ops->start(gcm_ctx, add, add_len);
    }
    return OS_EINVAL;
}

/**
 ***********************************************************************************************************************
 * @brief           Finish GCM crypto and generate the authentication tag.
 *
 * @param[in]       ctx             Crypto context.
 * @param[out]      tag             The buffer holding tag.
 * @param[in]       tag_len         The length of tag.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_finish(struct os_hwcrypto_ctx *ctx, const os_uint8_t *tag, os_size_t tag_len)
{
    struct hwcrypto_gcm *gcm_ctx = (struct hwcrypto_gcm *)ctx;

    if (gcm_ctx && gcm_ctx->ops->finish)
    {
        return gcm_ctx->ops->finish(gcm_ctx, tag, tag_len);
    }
    return OS_EINVAL;
}

/**
 ***********************************************************************************************************************
 * @brief           Perform a symmetric encryption or decryption operation.
 *
 * @param[in]       ctx             Crypto context.
 * @param[in]       mode            Crypto mode.
 * @param[in]       length          The length of the input data in Bytes. This must be a multiple of the block size.
 * @param[in]       in              The buffer holding the input data.
 * @param[out]      length          The buffer holding the output data.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_crypt(struct os_hwcrypto_ctx *ctx,
                               hwcrypto_mode           mode,
                               os_size_t               length,
                               const os_uint8_t *      in,
                               os_uint8_t *            out)
{
    return os_hwcrypto_symmetric_crypt(ctx, mode, length, in, out);
}

/**
 ***********************************************************************************************************************
 * @brief		   Set Symmetric encryption and decryption key.
 *
 * @param[in]	   ctx			   Crypto context.
 * @param[in]	   key    	       Crypto key.
 * @param[in]	   bitlen		   Crypto key bit length.
 *
 * @return
 * @retval		   OS_EOK		   Succeed.
 * @retval		   Others		   Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_setkey(struct os_hwcrypto_ctx *ctx, const os_uint8_t *key, os_uint32_t bitlen)
{
    return os_hwcrypto_symmetric_setkey(ctx, key, bitlen);
}

/**
 ***********************************************************************************************************************
 * @brief		   Get Symmetric encryption and decryption key.
 *
 * @param[in]	   ctx             Crypto context.
 * @param[out]     key             Crypto key.
 * @param[in]	   bitlen		   Crypto key bit length.
 *
 * @return
 * @retval		   OS_EOK		   Succeed.
 * @retval		   Others		   Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_getkey(struct os_hwcrypto_ctx *ctx, os_uint8_t *key, os_uint32_t bitlen)
{
    return os_hwcrypto_symmetric_getkey(ctx, key, bitlen);
}

/**
 ***********************************************************************************************************************
 * @brief           Set symmetric encryption and decryption initialization vector.
 *
 * @param[in]       ctx             Crypto context.
 * @param[in]       iv              Crypto initialization vector.
 * @param[in]       len             Crypto initialization vector length.
 *
 * @return
 * @retval		    OS_EOK		    Succeed.
 * @retval		    Others		    Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_setiv(struct os_hwcrypto_ctx *ctx, const os_uint8_t *iv, os_size_t len)
{
    return os_hwcrypto_symmetric_setiv(ctx, iv, len);
}

/**
 ***********************************************************************************************************************
 * @brief           Get symmetric encryption and decryption initialization vector.
 *
 * @param[in]       ctx             Crypto context.
 * @param[out]      iv              Crypto initialization vector.
 * @param[in]       len             Crypto initialization vector length.
 *
 * @return          Length of copied vector.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_getiv(struct os_hwcrypto_ctx *ctx, os_uint8_t *iv, os_size_t len)
{
    return os_hwcrypto_symmetric_getiv(ctx, iv, len);
}

/**
 ***********************************************************************************************************************
 * @brief           Set offset in initialization vector.
 *
 * @param[in]       ctx             Crypto context.
 * @param[in]       iv_off          The offset in initialization vector.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_gcm_set_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t iv_off)
{
    os_hwcrypto_symmetric_set_ivoff(ctx, iv_off);
}

/**
 ***********************************************************************************************************************
 * @brief           Get offset in initialization vector.
 *
 * @param[in]       ctx             Crypto context.
 * @param[out]      iv_off          The offset in initialization vector.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_gcm_get_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t *iv_off)
{
    os_hwcrypto_symmetric_get_ivoff(ctx, iv_off);
}

/**
 ***********************************************************************************************************************
 * @brief           Copy GCM crypto context.
 *
 * @param[out]      des             Destination of crypto context.
 * @param[in]       src             Source of crypto context.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_gcm_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    struct hwcrypto_gcm *gcm_des = (struct hwcrypto_gcm *)des;
    struct hwcrypto_gcm *gcm_src = (struct hwcrypto_gcm *)src;

    if (des != OS_NULL && src != OS_NULL)
    {
        gcm_des->crypt_type = gcm_src->crypt_type;
        /* symmetric crypto context copy */
        return os_hwcrypto_symmetric_cpy(des, src);
    }
    return OS_EINVAL;
}

/**
 ***********************************************************************************************************************
 * @brief           Reset GCM crypto context.
 *
 * @param[in]       ctx             Crypto context.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_gcm_reset(struct os_hwcrypto_ctx *ctx)
{
    os_hwcrypto_symmetric_reset(ctx);
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
os_err_t os_hwcrypto_gcm_register(struct os_hwcrypto_device *device, const char *name)
{
    if (hwcrypto_gcm_device != OS_NULL)
    {
        os_kprintf("hwcrypto gcm dev %s exist, %s register failed.\r\n",
                   device_name(&hwcrypto_gcm_device->parent), name);
        return OS_EFULL;
    }

    hwcrypto_gcm_device = device;

    return os_hwcrypto_register(device, name);
}


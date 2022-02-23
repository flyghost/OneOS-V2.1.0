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
 * @file        hw_symmetric.c
 *
 * @brief       This file provide interfaces for hardware symmetric crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <hw_symmetric.h>
#include <os_memory.h>
#include <string.h>

static struct os_hwcrypto_device *hwcrypto_symmetric_device = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           Create symmetric crypto context.
 *
 * @param[in]       device          Crypto device.
 * @param[in]       type            Type of crypto context.
 *
 * @return          Pointer to crypto context.
 ***********************************************************************************************************************
 */
struct os_hwcrypto_ctx *os_hwcrypto_symmetric_create(hwcrypto_type type)
{
    if (hwcrypto_symmetric_device == OS_NULL)
        return OS_NULL;

    return os_hwcrypto_ctx_create(device, type, sizeof(struct hwcrypto_symmetric));
}

/**
 ***********************************************************************************************************************
 * @brief           Detroy symmetric crypto context.
 *
 * @param[in]       ctx             Crypto context.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_symmetric_destroy(struct os_hwcrypto_ctx *ctx)
{
    os_hwcrypto_ctx_destroy(ctx);
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
os_err_t os_hwcrypto_symmetric_crypt(struct os_hwcrypto_ctx *ctx,
                                     hwcrypto_mode           mode,
                                     os_size_t               length,
                                     const os_uint8_t *      in,
                                     os_uint8_t *            out)
{
    struct hwcrypto_symmetric *    symmetric_ctx;
    struct hwcrypto_symmetric_info symmetric_info;
    os_err_t                       err;

    if (ctx == OS_NULL)
    {
        return OS_EINVAL;
    }
    symmetric_ctx = (struct hwcrypto_symmetric *)ctx;
    if (symmetric_ctx->ops->crypt == OS_NULL)
    {
        return OS_ERROR;
    }
    if (mode != HWCRYPTO_MODE_ENCRYPT && mode != HWCRYPTO_MODE_DECRYPT)
    {
        return OS_EINVAL;
    }

    /* Input information packaging */
    symmetric_info.mode   = mode;
    symmetric_info.in     = in;
    symmetric_info.out    = out;
    symmetric_info.length = length;

    /* Calling Hardware Encryption and Decryption Function */
    err = symmetric_ctx->ops->crypt(symmetric_ctx, &symmetric_info);

    /* clean up flags */
    symmetric_ctx->flags &= ~(SYMMTRIC_MODIFY_KEY | SYMMTRIC_MODIFY_IV | SYMMTRIC_MODIFY_IVOFF);
    return err;
}

/**
 ***********************************************************************************************************************
 * @brief          Set Symmetric encryption and decryption key.
 *
 * @param[in]      ctx             Crypto context.
 * @param[in]      key             Crypto key.
 * @param[in]      bitlen          Crypto key bit length.
 *
 * @return
 * @retval         OS_EOK          Succeed.
 * @retval         Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_symmetric_setkey(struct os_hwcrypto_ctx *ctx, const os_uint8_t *key, os_uint32_t bitlen)
{
    struct hwcrypto_symmetric *symmetric_ctx;

    if (ctx && bitlen <= OS_HWCRYPTO_KEYBIT_MAX_SIZE)
    {
        symmetric_ctx = (struct hwcrypto_symmetric *)ctx;
        memcpy(symmetric_ctx->key, key, bitlen >> 3);
        /* Record key length */
        symmetric_ctx->key_bitlen = bitlen;
        /* Key change flag set up */
        symmetric_ctx->flags |= SYMMTRIC_MODIFY_KEY;
        return OS_EOK;
    }

    return OS_EINVAL;
}

/**
 ***********************************************************************************************************************
 * @brief          Get Symmetric encryption and decryption key.
 *
 * @param[in]      ctx             Crypto context.
 * @param[out]     key             Crypto key.
 * @param[in]      bitlen          Crypto key bit length.
 *
 * @return
 * @retval         OS_EOK          Succeed.
 * @retval         Others          Fail.
 ***********************************************************************************************************************
 */
int os_hwcrypto_symmetric_getkey(struct os_hwcrypto_ctx *ctx, os_uint8_t *key, os_uint32_t bitlen)
{
    struct hwcrypto_symmetric *symmetric_ctx = (struct hwcrypto_symmetric *)ctx;

    if (ctx && bitlen >= symmetric_ctx->key_bitlen)
    {
        memcpy(key, symmetric_ctx->key, symmetric_ctx->key_bitlen >> 3);
        return symmetric_ctx->key_bitlen;
    }

    return 0;
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
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_symmetric_setiv(struct os_hwcrypto_ctx *ctx, const os_uint8_t *iv, os_size_t len)
{
    struct hwcrypto_symmetric *symmetric_ctx;

    if (ctx && len <= OS_HWCRYPTO_IV_MAX_SIZE)
    {
        symmetric_ctx = (struct hwcrypto_symmetric *)ctx;
        memcpy(symmetric_ctx->iv, iv, len);
        symmetric_ctx->iv_len = len;
        /* IV change flag set up */
        symmetric_ctx->flags |= SYMMTRIC_MODIFY_IV;
        return OS_EOK;
    }

    return OS_EINVAL;
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
int os_hwcrypto_symmetric_getiv(struct os_hwcrypto_ctx *ctx, os_uint8_t *iv, os_size_t len)
{
    struct hwcrypto_symmetric *symmetric_ctx = (struct hwcrypto_symmetric *)ctx;
    ;

    if (ctx && len >= symmetric_ctx->iv_len)
    {
        memcpy(iv, symmetric_ctx->iv, symmetric_ctx->iv_len);
        return symmetric_ctx->iv_len;
    }

    return 0;
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
void os_hwcrypto_symmetric_set_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t iv_off)
{
    if (ctx)
    {
        ((struct hwcrypto_symmetric *)ctx)->iv_off = iv_off;
        /* iv_off change flag set up */
        ((struct hwcrypto_symmetric *)ctx)->flags |= SYMMTRIC_MODIFY_IVOFF;
    }
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
void os_hwcrypto_symmetric_get_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t *iv_off)
{
    if (ctx && iv_off)
    {
        *iv_off = ((struct hwcrypto_symmetric *)ctx)->iv_off;
    }
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
os_err_t os_hwcrypto_symmetric_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    struct hwcrypto_symmetric *symmetric_des = (struct hwcrypto_symmetric *)des;
    struct hwcrypto_symmetric *symmetric_src = (struct hwcrypto_symmetric *)src;

    if (des != OS_NULL && src != OS_NULL)
    {
        /* Copy Symmetric Encryption and Decryption Context Information */
        symmetric_des->flags      = symmetric_src->flags;
        symmetric_des->iv_len     = symmetric_src->iv_len;
        symmetric_des->iv_off     = symmetric_src->iv_off;
        symmetric_des->key_bitlen = symmetric_src->key_bitlen;
        memcpy(symmetric_des->iv, symmetric_src->iv, symmetric_src->iv_len);
        memcpy(symmetric_des->key, symmetric_src->key, symmetric_src->key_bitlen >> 3);

        /* Hardware context copy */
        return os_hwcrypto_ctx_cpy(des, src);
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
void os_hwcrypto_symmetric_reset(struct os_hwcrypto_ctx *ctx)
{
    struct hwcrypto_symmetric *symmetric_ctx = (struct hwcrypto_symmetric *)ctx;
    if (ctx != OS_NULL)
    {
        /* Copy Symmetric Encryption and Decryption Context Information */
        symmetric_ctx->flags      = 0x00;
        symmetric_ctx->iv_len     = 0x00;
        symmetric_ctx->iv_off     = 0x00;
        symmetric_ctx->key_bitlen = 0x00;
        memset(symmetric_ctx->iv, 0, OS_HWCRYPTO_IV_MAX_SIZE);
        memset(symmetric_ctx->key, 0, OS_HWCRYPTO_KEYBIT_MAX_SIZE >> 3);

        /* Hardware context reset */
        os_hwcrypto_ctx_reset(ctx);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Set symmetric crypto context type.
 *
 * @param[in]       ctx             Crypto context.
 * @param[in]       type            Type of crypto context.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          Others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_symmetric_set_type(struct os_hwcrypto_ctx *ctx, hwcrypto_type type)
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
os_err_t os_hwcrypto_symmetric_register(struct os_hwcrypto_device *device, const char *name)
{
    if (hwcrypto_symmetric_device != OS_NULL)
    {
        os_kprintf("hwcrypto symmetric dev %s exist, %s register failed.\r\n",
                   device_name(&hwcrypto_symmetric_device->parent), name);
        return OS_EFULL;
    }

    hwcrypto_symmetric_device = device;

    return os_hwcrypto_register(device, name);
}


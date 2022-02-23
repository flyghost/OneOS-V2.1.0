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
 * @file        hw_bignum.c
 *
 * @brief       This file provide operations for big number.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <hw_bignum.h>
#include <os_memory.h>
#include <string.h>

static struct os_hwcrypto_device *hwcrypto_bignum_device = OS_NULL;

static struct os_hwcrypto_ctx *bignum_default = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           Initialise a bignum obj.
 *
 * @param[in]       n               Bignum obj.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_bignum_init(struct hw_bignum_mpi *n)
{
    if (n == OS_NULL)
        return;

    n->sign  = 1;
    n->total = 0;
    n->p     = OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Free a bignum obj.
 *
 * @param[in]       n               Bignum obj.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_bignum_free(struct hw_bignum_mpi *n)
{
    if (n)
    {
        os_free(n->p);
        n->sign  = 0;
        n->total = 0;
        n->p     = OS_NULL;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Get length of bignum as an unsigned binary buffer.
 *
 * @param[in]       n               Bignum obj.
 *
 * @return          Binary buffer length.
 ***********************************************************************************************************************
 */
int os_hwcrypto_bignum_get_len(const struct hw_bignum_mpi *n)
{
    int tmp_len, total;

    if (n == OS_NULL || n->p == OS_NULL)
    {
        return 0;
    }
    tmp_len = 0;
    total   = n->total;
    while ((total > 0) && (n->p[total - 1] == 0))
    {
        tmp_len++;
        total--;
    }
    return n->total - tmp_len;
}

/**
 ***********************************************************************************************************************
 * @brief           Export n into unsigned binary data, big endian.
 *
 * @param[in]       n               Bignum obj.
 * @param[out]      buf             Buffer of the binary number.
 * @param[in]       len             Length of the buffer.
 *
 * @return          Export bin length.
 ***********************************************************************************************************************
 */
int os_hwcrypto_bignum_export_bin(struct hw_bignum_mpi *n, os_uint8_t *buf, int len)
{
    int cp_len, i, j;

    if (n == OS_NULL || buf == OS_NULL)
    {
        return 0;
    }
    memset(buf, 0, len);
    cp_len = n->total > len ? len : n->total;
    for (i = cp_len, j = 0; i > 0; i--, j++)
    {
        buf[i - 1] = n->p[j];
    }

    return cp_len;
}

/**
 ***********************************************************************************************************************
 * @brief           Import n from unsigned binary data, big endian.
 *
 * @param[out]      n               Bignum obj.
 * @param[in]       buf             Buffer of the binary number.
 * @param[in]       len             Length of the buffer.
 *
 * @return
 * @retval          0               Succeed.
 * @retval          Others          Length of data copied.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_bignum_import_bin(struct hw_bignum_mpi *n, os_uint8_t *buf, int len)
{
    int   cp_len, i, j;
    void *temp_p;

    if (n == OS_NULL || buf == OS_NULL)
    {
        return 0;
    }
    if (n->total < len)
    {
        temp_p = os_calloc(1, len);
        if (temp_p == OS_NULL)
        {
            return 0;
        }
        memset(temp_p, 0, len);
        os_free(n->p);
        n->p     = temp_p;
        n->total = len;
    }
    cp_len = n->total > len ? len : n->total;

    for (i = cp_len, j = 0; i > 0; i--, j++)
    {
        n->p[j] = buf[i - 1];
    }

    return cp_len;
}

/**
 ***********************************************************************************************************************
 * @brief           x = a + b.
 *
 * @param[out]      x               Bignum obj.
 * @param[in]       a               Bignum obj.
 * @param[in]       b               Bignum obj.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_bignum_add(struct hw_bignum_mpi *x, const struct hw_bignum_mpi *a, const struct hw_bignum_mpi *b)
{
    struct hwcrypto_bignum *bignum_ctx;

    if (bignum_default == OS_NULL)
        return OS_ERROR;
    
    bignum_ctx = (struct hwcrypto_bignum *)bignum_default;
    if (bignum_ctx->ops->add)
    {
        return bignum_ctx->ops->add(bignum_ctx, x, a, b);
    }
    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           x = a - b.
 *
 * @param[out]      x               Bignum obj.
 * @param[in]       a               Bignum obj.
 * @param[in]       b               Bignum obj.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_bignum_sub(struct hw_bignum_mpi *x, const struct hw_bignum_mpi *a, const struct hw_bignum_mpi *b)
{
    struct hwcrypto_bignum *bignum_ctx;

    if (bignum_default == OS_NULL)
        return OS_ERROR;
    
    bignum_ctx = (struct hwcrypto_bignum *)bignum_default;
    if (bignum_ctx->ops->sub)
    {
        return bignum_ctx->ops->sub(bignum_ctx, x, a, b);
    }
    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           x = a * b.
 *
 * @param[out]      x               Bignum obj.
 * @param[in]       a               Bignum obj.
 * @param[in]       b               Bignum obj.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_bignum_mul(struct hw_bignum_mpi *x, const struct hw_bignum_mpi *a, const struct hw_bignum_mpi *b)
{
    struct hwcrypto_bignum *bignum_ctx;

    if (bignum_default == OS_NULL)
        return OS_ERROR;
    
    bignum_ctx = (struct hwcrypto_bignum *)bignum_default;
    if (bignum_ctx->ops->mul)
    {
        return bignum_ctx->ops->mul(bignum_ctx, x, a, b);
    }
    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           x = a * b (mod c).
 *
 * @param[out]      x               Bignum obj.
 * @param[in]       a               Bignum obj.
 * @param[in]       b               Bignum obj.
 * @param[in]       c               Bignum obj.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_bignum_mulmod(struct hw_bignum_mpi *      x,
                                   const struct hw_bignum_mpi *a,
                                   const struct hw_bignum_mpi *b,
                                   const struct hw_bignum_mpi *c)
{
    struct hwcrypto_bignum *bignum_ctx;

    if (bignum_default == OS_NULL)
        return OS_ERROR;
    
    bignum_ctx = (struct hwcrypto_bignum *)bignum_default;
    if (bignum_ctx->ops->mulmod)
    {
        return bignum_ctx->ops->mulmod(bignum_ctx, x, a, b, c);
    }
    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           x = a ^ b (mod c).
 *
 * @param[out]      x               Bignum obj.
 * @param[in]       a               Bignum obj.
 * @param[in]       b               Bignum obj.
 * @param[in]       c               Bignum obj.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_bignum_exptmod(struct hw_bignum_mpi *      x,
                                    const struct hw_bignum_mpi *a,
                                    const struct hw_bignum_mpi *b,
                                    const struct hw_bignum_mpi *c)
{
    struct hwcrypto_bignum *bignum_ctx;

    if (bignum_default == OS_NULL)
        return OS_ERROR;
    
    bignum_ctx = (struct hwcrypto_bignum *)bignum_default;
    if (bignum_ctx->ops->exptmod)
    {
        return bignum_ctx->ops->exptmod(bignum_ctx, x, a, b, c);
    }
    return OS_ERROR;
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
os_err_t os_hwcrypto_bignum_register(struct os_hwcrypto_device *device, const char *name)
{
    if (hwcrypto_bignum_device != OS_NULL)
    {
        os_kprintf("hwcrypto bignum dev %s exist, %s register failed.\r\n",
                   device_name(&hwcrypto_bignum_device->parent), name);
        return OS_EFULL;
    }

    hwcrypto_bignum_device = device;

    os_hwcrypto_register(device, name);

    bignum_default = os_hwcrypto_ctx_create(hwcrypto_bignum_device, 
                                            HWCRYPTO_TYPE_BIGNUM, 
                                            sizeof(struct hwcrypto_bignum));
    if (bignum_default == OS_NULL)
        return OS_ERROR;
    
    return OS_EOK;
}
                                    

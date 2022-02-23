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
 * @file        hw_symmetric.h
 *
 * @brief       This file provide interfaces for hardware symmetric crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HW_SYMMETRIC_H__
#define __HW_SYMMETRIC_H__

#include <hwcrypto/hwcrypto.h>

#ifndef OS_HWCRYPTO_IV_MAX_SIZE
#define OS_HWCRYPTO_IV_MAX_SIZE (16)
#endif
#ifndef OS_HWCRYPTO_KEYBIT_MAX_SIZE
#define OS_HWCRYPTO_KEYBIT_MAX_SIZE (256)
#endif

#define SYMMTRIC_MODIFY_KEY   (0x1 << 0)
#define SYMMTRIC_MODIFY_IV    (0x1 << 1)
#define SYMMTRIC_MODIFY_IVOFF (0x1 << 2)

#ifdef __cplusplus
extern "C" {
#endif

struct hwcrypto_symmetric;
struct hwcrypto_symmetric_info;

struct hwcrypto_symmetric_ops
{
    os_err_t (*crypt)(
        struct hwcrypto_symmetric *     symmetric_ctx,
        struct hwcrypto_symmetric_info *symmetric_info); /* Hardware Symmetric Encryption and Decryption Callback */
};

struct hwcrypto_symmetric_info
{
    hwcrypto_mode     mode;   /* crypto mode. HWCRYPTO_MODE_ENCRYPT or HWCRYPTO_MODE_DECRYPT */
    const os_uint8_t *in;     /* Input data */
    os_uint8_t *      out;    /* Output data will be written */
    os_size_t         length; /* The length of the input data in Bytes. It's a multiple of block size. */
};

struct hwcrypto_symmetric
{
    struct os_hwcrypto_ctx               parent; /* Inheritance from hardware crypto context */
    os_uint32_t                          flags;  /* key or iv or ivoff has been changed. The flag will be set up */
    os_int32_t                           iv_len; /* initialization vector effective length */
    os_int32_t                           iv_off; /* The offset in IV */
    os_uint8_t                           iv[OS_HWCRYPTO_IV_MAX_SIZE];           /* The initialization vector */
    os_uint8_t                           key[OS_HWCRYPTO_KEYBIT_MAX_SIZE >> 3]; /* The crypto key */
    os_int32_t                           key_bitlen;                            /* The crypto key bit length */
    const struct hwcrypto_symmetric_ops *ops; /* Hardware initializes this value when creating context */
};

struct os_hwcrypto_ctx *os_hwcrypto_symmetric_create(hwcrypto_type type);

void os_hwcrypto_symmetric_destroy(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_symmetric_crypt(struct os_hwcrypto_ctx *ctx,
                                     hwcrypto_mode           mode,
                                     os_size_t               length,
                                     const os_uint8_t *      in,
                                     os_uint8_t *            out);

os_err_t os_hwcrypto_symmetric_setkey(struct os_hwcrypto_ctx *ctx, const os_uint8_t *key, os_uint32_t bitlen);

int os_hwcrypto_symmetric_getkey(struct os_hwcrypto_ctx *ctx, os_uint8_t *key, os_uint32_t bitlen);

os_err_t os_hwcrypto_symmetric_setiv(struct os_hwcrypto_ctx *ctx, const os_uint8_t *iv, os_size_t len);

int os_hwcrypto_symmetric_getiv(struct os_hwcrypto_ctx *ctx, os_uint8_t *iv, os_size_t len);

void os_hwcrypto_symmetric_set_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t iv_off);

void os_hwcrypto_symmetric_get_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t *iv_off);

os_err_t os_hwcrypto_symmetric_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src);

void os_hwcrypto_symmetric_reset(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_symmetric_set_type(struct os_hwcrypto_ctx *ctx, hwcrypto_type type);

os_err_t os_hwcrypto_symmetric_register(struct os_hwcrypto_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif

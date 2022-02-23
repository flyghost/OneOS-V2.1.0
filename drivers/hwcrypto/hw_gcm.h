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
 * @file        hw_gcm.h
 *
 * @brief       This file provide interfaces for hardware GCM crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HW_GCM_H__
#define __HW_GCM_H__

#include <string.h>
#include "hw_symmetric.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hwcrypto_gcm;

struct hwcrypto_gcm_ops
{
    os_err_t (*start)(struct hwcrypto_gcm *gcm_ctx,
                      const unsigned char *add,
                      size_t               add_len); /* Set additional data. start GCM operation */
    os_err_t (*finish)(struct hwcrypto_gcm *gcm_ctx,
                       const unsigned char *tag,
                       size_t               tag_len); /* finish GCM operation. get tag */
};

struct hwcrypto_gcm
{
    struct hwcrypto_symmetric      parent;     /* Inheritance from hardware symmetric crypto context */
    hwcrypto_type                  crypt_type; /* symmetric crypto type. eg: AES/DES */
    const struct hwcrypto_gcm_ops *ops;        /* Hardware initializes this value when creating context */
};

struct os_hwcrypto_ctx *os_hwcrypto_gcm_create(hwcrypto_type crypt_type);

void os_hwcrypto_gcm_destroy(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_gcm_start(struct os_hwcrypto_ctx *ctx, const os_uint8_t *add, os_size_t add_len);

os_err_t os_hwcrypto_gcm_finish(struct os_hwcrypto_ctx *ctx, const os_uint8_t *tag, os_size_t tag_len);

os_err_t os_hwcrypto_gcm_crypt(struct os_hwcrypto_ctx *ctx,
                               hwcrypto_mode           mode,
                               os_size_t               length,
                               const os_uint8_t *      in,
                               os_uint8_t *            out);

os_err_t os_hwcrypto_gcm_setkey(struct os_hwcrypto_ctx *ctx, const os_uint8_t *key, os_uint32_t bitlen);

os_err_t os_hwcrypto_gcm_getkey(struct os_hwcrypto_ctx *ctx, os_uint8_t *key, os_uint32_t bitlen);

os_err_t os_hwcrypto_gcm_setiv(struct os_hwcrypto_ctx *ctx, const os_uint8_t *iv, os_size_t len);

os_err_t os_hwcrypto_gcm_getiv(struct os_hwcrypto_ctx *ctx, os_uint8_t *iv, os_size_t len);

void os_hwcrypto_gcm_set_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t iv_off);

void os_hwcrypto_gcm_get_ivoff(struct os_hwcrypto_ctx *ctx, os_int32_t *iv_off);

os_err_t os_hwcrypto_gcm_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src);

void os_hwcrypto_gcm_reset(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_gcm_register(struct os_hwcrypto_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif

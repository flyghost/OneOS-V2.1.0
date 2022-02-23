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
 * @file        hw_hash.h
 *
 * @brief       This file provides interfaces for hardware hash crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HW_HASH_H__
#define __HW_HASH_H__

#include <hwcrypto/hwcrypto.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hwcrypto_hash;

struct hwcrypto_hash_ops
{
    os_err_t (*update)(struct hwcrypto_hash *hash_ctx, const os_uint8_t *in, os_size_t length); /* Process data */
    os_err_t (*finish)(struct hwcrypto_hash *hash_ctx,
                       os_uint8_t *          out,
                       os_size_t             length); /* Get the final hash value */
};

struct hwcrypto_hash
{
    struct os_hwcrypto_ctx          parent; /* Inherit from hardware crypto context */
    const struct hwcrypto_hash_ops *ops;    /* Operations for hash crypto context */
};

struct os_hwcrypto_ctx *os_hwcrypto_hash_create(hwcrypto_type type);

void os_hwcrypto_hash_destroy(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_hash_finish(struct os_hwcrypto_ctx *ctx, os_uint8_t *output, os_size_t length);

os_err_t os_hwcrypto_hash_update(struct os_hwcrypto_ctx *ctx, const os_uint8_t *input, os_size_t length);

os_err_t os_hwcrypto_hash_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src);

void os_hwcrypto_hash_reset(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_hash_set_type(struct os_hwcrypto_ctx *ctx, hwcrypto_type type);

os_err_t os_hwcrypto_hash_register(struct os_hwcrypto_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif

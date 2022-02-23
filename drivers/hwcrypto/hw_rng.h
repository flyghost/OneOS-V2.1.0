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
 * @file        hw_rng.h
 *
 * @brief       This file provides interfaces for hardware RNG.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HW_RNG_H__
#define __HW_RNG_H__

#include <hwcrypto/hwcrypto.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hwcrypto_rng;

struct hwcrypto_rng_ops
{
    os_uint32_t (*update)(struct hwcrypto_rng *ctx); /* Return a random number */
};

struct hwcrypto_rng
{
    struct os_hwcrypto_ctx         parent; /* Inheritance from hardware crypto context */
    const struct hwcrypto_rng_ops *ops;    /* Hardware initializes this value when creating context */
};

os_uint32_t os_hwcrypto_rng_update_ctx(struct os_hwcrypto_ctx *ctx);

os_uint32_t os_hwcrypto_rng_update(void);

os_err_t os_hwcrypto_rng_register(struct os_hwcrypto_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif

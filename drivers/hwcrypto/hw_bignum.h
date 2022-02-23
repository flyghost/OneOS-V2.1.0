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
 * @file        hw_bignum.h
 *
 * @brief       This file provide operations for big number.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HW_BIGNUM_H__
#define __HW_BIGNUM_H__

#include <hwcrypto/hwcrypto.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hwcrypto_bignum;

/* bignum obj */
struct hw_bignum_mpi
{
    int         sign;  /* integer sign */
    os_size_t   total; /* total of limbs */
    os_uint8_t *p;     /* pointer to limbs */
};

struct hwcrypto_bignum_ops
{
    os_err_t (*add)(struct hwcrypto_bignum *    bignum_ctx,
                    struct hw_bignum_mpi *      x,
                    const struct hw_bignum_mpi *a,
                    const struct hw_bignum_mpi *b); /* x = a + b */
    os_err_t (*sub)(struct hwcrypto_bignum *    bignum_ctx,
                    struct hw_bignum_mpi *      x,
                    const struct hw_bignum_mpi *a,
                    const struct hw_bignum_mpi *b); /* x = a - b */
    os_err_t (*mul)(struct hwcrypto_bignum *    bignum_ctx,
                    struct hw_bignum_mpi *      x,
                    const struct hw_bignum_mpi *a,
                    const struct hw_bignum_mpi *b); /* x = a * b */
    os_err_t (*mulmod)(struct hwcrypto_bignum *    bignum_ctx,
                       struct hw_bignum_mpi *      x,
                       const struct hw_bignum_mpi *a,
                       const struct hw_bignum_mpi *b,
                       const struct hw_bignum_mpi *c); /* x = a * b (mod c) */
    os_err_t (*exptmod)(struct hwcrypto_bignum *    bignum_ctx,
                        struct hw_bignum_mpi *      x,
                        const struct hw_bignum_mpi *a,
                        const struct hw_bignum_mpi *b,
                        const struct hw_bignum_mpi *c); /* x = a ^ b (mod c) */
};

struct hwcrypto_bignum
{
    struct os_hwcrypto_ctx            parent; /* Inheritance from hardware crypto context */
    const struct hwcrypto_bignum_ops *ops;    /* Hardware initializes this value when creating context */
};

void os_hwcrypto_bignum_init(struct hw_bignum_mpi *n);

void os_hwcrypto_bignum_free(struct hw_bignum_mpi *n);

int os_hwcrypto_bignum_get_len(const struct hw_bignum_mpi *n);

int os_hwcrypto_bignum_export_bin(struct hw_bignum_mpi *n, os_uint8_t *buf, int len);

os_err_t os_hwcrypto_bignum_import_bin(struct hw_bignum_mpi *n, os_uint8_t *buf, int len);

os_err_t os_hwcrypto_bignum_add(struct hw_bignum_mpi *x, const struct hw_bignum_mpi *a, const struct hw_bignum_mpi *b);

os_err_t os_hwcrypto_bignum_sub(struct hw_bignum_mpi *x, const struct hw_bignum_mpi *a, const struct hw_bignum_mpi *b);

os_err_t os_hwcrypto_bignum_mul(struct hw_bignum_mpi *x, const struct hw_bignum_mpi *a, const struct hw_bignum_mpi *b);

os_err_t os_hwcrypto_bignum_mulmod(struct hw_bignum_mpi *      x,
                                   const struct hw_bignum_mpi *a,
                                   const struct hw_bignum_mpi *b,
                                   const struct hw_bignum_mpi *c);

os_err_t os_hwcrypto_bignum_exptmod(struct hw_bignum_mpi *      x,
                                    const struct hw_bignum_mpi *a,
                                    const struct hw_bignum_mpi *b,
                                    const struct hw_bignum_mpi *c);

os_err_t os_hwcrypto_bignum_register(struct os_hwcrypto_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif

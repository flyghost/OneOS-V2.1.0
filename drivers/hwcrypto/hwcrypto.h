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
 * @file        hwcrypto.h
 *
 * @brief       This file provides hardware crypto interfaces.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HWCRYPTO_H__
#define __HWCRYPTO_H__

#include <os_task.h>
#include <device.h>

#ifndef OS_HWCRYPTO_DEFAULT_NAME
#define OS_HWCRYPTO_DEFAULT_NAME ("hwcryto")
#endif

#define HWCRYPTO_MAIN_TYPE_MASK (0xffffUL << 16)
#define HWCRYPTO_SUB_TYPE_MASK  (0xffUL << 8)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    HWCRYPTO_TYPE_NULL = 0x00000000,

    /* Main Type */
    /* symmetric Type */
    HWCRYPTO_TYPE_HEAD = __LINE__,
    HWCRYPTO_TYPE_AES  = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* AES */
    HWCRYPTO_TYPE_DES  = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* DES */
    HWCRYPTO_TYPE_3DES = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* 3DES */
    HWCRYPTO_TYPE_RC4  = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* RC4 */
    HWCRYPTO_TYPE_GCM  = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* GCM */
    /* HASH Type */
    HWCRYPTO_TYPE_MD5  = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* MD5 */
    HWCRYPTO_TYPE_SHA1 = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* SHA1 */
    HWCRYPTO_TYPE_SHA2 = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* SHA2 */
    /* Other Type */
    HWCRYPTO_TYPE_RNG    = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* RNG */
    HWCRYPTO_TYPE_CRC    = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* CRC */
    HWCRYPTO_TYPE_BIGNUM = ((__LINE__ - HWCRYPTO_TYPE_HEAD) & 0xffff) << 16, /* BIGNUM */

    /* AES Subtype */
    HWCRYPTO_TYPE_AES_ECB = HWCRYPTO_TYPE_AES | (0x01 << 8),
    HWCRYPTO_TYPE_AES_CBC = HWCRYPTO_TYPE_AES | (0x02 << 8),
    HWCRYPTO_TYPE_AES_CFB = HWCRYPTO_TYPE_AES | (0x03 << 8),
    HWCRYPTO_TYPE_AES_CTR = HWCRYPTO_TYPE_AES | (0x04 << 8),
    HWCRYPTO_TYPE_AES_OFB = HWCRYPTO_TYPE_AES | (0x05 << 8),

    /* DES Subtype */
    HWCRYPTO_TYPE_DES_ECB = HWCRYPTO_TYPE_DES | (0x01 << 8),
    HWCRYPTO_TYPE_DES_CBC = HWCRYPTO_TYPE_DES | (0x02 << 8),

    /* 3DES Subtype */
    HWCRYPTO_TYPE_3DES_ECB = HWCRYPTO_TYPE_3DES | (0x01 << 8),
    HWCRYPTO_TYPE_3DES_CBC = HWCRYPTO_TYPE_3DES | (0x02 << 8),

    /* SHA2 Subtype */
    HWCRYPTO_TYPE_SHA224 = HWCRYPTO_TYPE_SHA2 | (0x01 << 8),
    HWCRYPTO_TYPE_SHA256 = HWCRYPTO_TYPE_SHA2 | (0x02 << 8),
    HWCRYPTO_TYPE_SHA384 = HWCRYPTO_TYPE_SHA2 | (0x03 << 8),
    HWCRYPTO_TYPE_SHA512 = HWCRYPTO_TYPE_SHA2 | (0x04 << 8),
} hwcrypto_type;

typedef enum
{
    HWCRYPTO_MODE_ENCRYPT = 0x1,        /* Encryption operations */
    HWCRYPTO_MODE_DECRYPT = 0x2,        /* Decryption operations */
    HWCRYPTO_MODE_UNKNOWN = 0x7fffffff, /* Unknown */
} hwcrypto_mode;

struct os_hwcrypto_ctx;

struct os_hwcrypto_ops
{
    os_err_t (*create)(struct os_hwcrypto_ctx *ctx);                                  /* Creating hardware context */
    void (*destroy)(struct os_hwcrypto_ctx *ctx);                                     /* Delete hardware context */
    os_err_t (*copy)(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src); /* Cpoy hardware context */
    void (*reset)(struct os_hwcrypto_ctx *ctx);                                       /* Reset hardware context */
};

struct os_hwcrypto_device
{
    struct os_device              parent;    /* Inherited from the standard device */
    const struct os_hwcrypto_ops *ops;       /* Hardware crypto ops */
    os_uint64_t                   id;        /* Unique id */
    void                         *user_data; /* Device user data */
};

struct os_hwcrypto_ctx
{
    struct os_hwcrypto_device *device; /* Binding device */
    hwcrypto_type              type;   /* Encryption and decryption types */
    void *                     contex; /* Hardware context */
};

os_err_t os_hwcrypto_set_type(struct os_hwcrypto_ctx *ctx, hwcrypto_type type);

void os_hwcrypto_ctx_reset(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_ctx_init(struct os_hwcrypto_ctx *ctx, struct os_hwcrypto_device *device, hwcrypto_type type);

struct os_hwcrypto_ctx *
os_hwcrypto_ctx_create(struct os_hwcrypto_device *device, hwcrypto_type type, os_uint32_t obj_size);

void os_hwcrypto_ctx_destroy(struct os_hwcrypto_ctx *ctx);

os_err_t os_hwcrypto_ctx_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src);

os_err_t os_hwcrypto_register(struct os_hwcrypto_device *device, const char *name);

os_uint64_t os_hwcrypto_id(struct os_hwcrypto_device *device);

#ifdef __cplusplus
}
#endif

#endif

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
 * @file        hw_crc.h
 *
 * @brief       This file provide interfaces for hardware CRC crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __HW_CRC_H__
#define __HW_CRC_H__

#include <hwcrypto/hwcrypto.h>

#define CRC_FLAG_REFIN  (0x1 << 0)
#define CRC_FLAG_REFOUT (0x1 << 1)

#define HWCRYPTO_CRC8_CFG       \
{                               \
    .last_val = 0x00,           \
    .poly = 0x07,               \
    .width = 8,                 \
    .xorout = 0x00,             \
    .flags = 0,                 \
}

#define HWCRYPTO_CRC16_CFG      \
{                               \
    .last_val = 0x0000,         \
    .poly = 0x8005,             \
    .width = 16,                \
    .xorout = 0x0000,           \
    .flags = 0,                 \
}

#define HWCRYPTO_CRC32_CFG     \
{                              \
    .last_val = 0x00000000,    \
    .poly = 0x04C11DB7,        \
    .width = 32,               \
    .xorout = 0x00000000,      \
    .flags = 0,                \
}

#define HWCRYPTO_CRC_CCITT_CFG  \
{                               \
    .last_val = 0x0000,         \
    .poly = 0x1021,             \
    .width = 16,                \
    .xorout = 0x0000,           \
    .flags = CRC_FLAG_REFIN | CRC_FLAG_REFOUT, \
}

#define HWCRYPTO_CRC_DNP_CFG   \
{                              \
    .last_val = 0x0000,        \
    .poly = 0x3D65,            \
    .width = 16,               \
    .xorout = 0xffff,          \
    .flags = CRC_FLAG_REFIN | CRC_FLAG_REFOUT, \
}

#ifdef __cplusplus
extern "C" {
#endif

struct hwcrypto_crc;

typedef enum
{
    HWCRYPTO_CRC_CUSTOM, /* Custom CRC mode */
    HWCRYPTO_CRC_CRC8,   /* poly : 0x07 */
    HWCRYPTO_CRC_CRC16,  /* poly : 0x8005 */
    HWCRYPTO_CRC_CRC32,  /* poly : 0x04C11DB7 */
    HWCRYPTO_CRC_CCITT,  /* poly : 0x1021 */
    HWCRYPTO_CRC_DNP,    /* poly : 0x3D65 */
} hwcrypto_crc_mode;

struct hwcrypto_crc_cfg
{
    os_uint32_t last_val; /* Last CRC value cache */
    os_uint32_t poly;     /* CRC polynomial */
    os_uint16_t width;    /* CRC value width */
    os_uint32_t xorout;   /* Result XOR Value */
    os_uint16_t flags;    /* Input or output data reverse. CRC_FLAG_REFIN or CRC_FLAG_REFOUT */
};

struct hwcrypto_crc_ops
{
    os_int32_t  (*config)(struct hwcrypto_crc *ctx);
    os_uint32_t (*update)(struct hwcrypto_crc *ctx, const os_uint8_t *in, os_size_t length);
};

struct hwcrypto_crc
{
    struct os_hwcrypto_ctx         parent;  /* Inherit from hardware crypto context */
    struct hwcrypto_crc_cfg        crc_cfg; /* CRC configuration */
    const struct hwcrypto_crc_ops *ops;     /* Operations for CRC crypto context */
};

struct os_hwcrypto_ctx *os_hwcrypto_crc_create(hwcrypto_crc_mode mode);

void os_hwcrypto_crc_destroy(struct os_hwcrypto_ctx *ctx);

os_uint32_t os_hwcrypto_crc_update(struct os_hwcrypto_ctx *ctx, const os_uint8_t *input, os_size_t length);

void os_hwcrypto_crc_cfg(struct os_hwcrypto_ctx *ctx, const struct hwcrypto_crc_cfg *cfg);

os_err_t os_hwcrypto_crc_register(struct os_hwcrypto_device *device, const char *name);

#ifdef __cplusplus
}
#endif

#endif

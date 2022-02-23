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
 * @file        cmiot_md5.h
 *
 * @brief       The md5 header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_MD5_H__
#define __CMIOT_MD5_H__

#include "cmiot_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MD5_BLOCKSIZE
#define MD5_BLOCKSIZE 64
#endif

typedef struct
{
    cmiot_uint32 count[2];
    cmiot_uint32 state[4];
    cmiot_uint8  buffer[MD5_BLOCKSIZE];
} cmiot_md5_ctx_t;

typedef struct
{
    cmiot_uint8  content1;
    cmiot_uint8  content2;
    cmiot_uint32 content3;
} cmiot_md5_data_t;

#define F(x, y, z)        (((x) & (y)) | (~(x) & (z)))
#define G(x, y, z)        (((x) & (z)) | ((y) & ~(z)))
#define H(x, y, z)        ((x) ^ (y) ^ (z))
#define I(x, y, z)        ((y) ^ ((x) | ~(z)))
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define FF(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += F((b), (c), (d)) + (x) + (ac);                                                                          \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }

#define GG(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += G((b), (c), (d)) + (x) + (ac);                                                                          \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }

#define HH(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += H((b), (c), (d)) + (x) + (ac);                                                                          \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }

#define II(a, b, c, d, x, s, ac)                                                                                       \
    {                                                                                                                  \
        (a) += I((b), (c), (d)) + (x) + (ac);                                                                          \
        (a) = ROTATE_LEFT((a), (s));                                                                                   \
        (a) += (b);                                                                                                    \
    }

void cmiot_md5_init(cmiot_md5_ctx_t *context);
void cmiot_md5_update(cmiot_md5_ctx_t *context, cmiot_uint8 *input, cmiot_uint32 inputlen);
void cmiot_md5_final(cmiot_md5_ctx_t *context, cmiot_uint8 *digest);
void cmiot_md5_transform(cmiot_uint32 *state, cmiot_uint8 *block);
void cmiot_md5_encode(cmiot_uint8 *output, cmiot_uint32 *input, cmiot_uint32 len);
void cmiot_md5_decode(cmiot_uint32 *output, cmiot_uint8 *input, cmiot_uint32 len);

#ifdef __cplusplus
}
#endif

#endif

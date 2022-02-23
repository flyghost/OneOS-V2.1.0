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
 * @file        onenet_base64.h
 *
 * @brief       This function supply a Base64 encode and decode method.
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _ONENET_BASE64_H
#define _ONENET_BASE64_H

#include <stddef.h>

#define MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL  -0x002A /**< Output buffer too small. */
#define MBEDTLS_ERR_BASE64_INVALID_CHARACTER -0x002C /**< Invalid character in input. */

#ifdef __cplusplus
extern "C" {
#endif

extern int BASE64_Encode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);
extern int BASE64_Decode(unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen);

#ifdef __cplusplus
}
#endif

#endif /* onenet_base64.h */

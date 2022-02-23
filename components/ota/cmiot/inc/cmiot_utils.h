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
 * @file        cmiot_stdlib.h
 *
 * @brief       The stdlib header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_UTILS_H__
#define __CMIOT_UTILS_H__

#include "cmiot_type.h"

#ifdef __cplusplus
extern "C" {
#endif

cmiot_char *cmiot_get_32bit_dec_tmp(cmiot_int32 number, cmiot_bool unsignedint);
void        cmiot_byte2hexstr(const cmiot_uint8 *source, cmiot_char *dest, cmiot_uint32 source_len);
void        cmiot_hexstr2byte(cmiot_char *dest, const cmiot_uint8 *source, cmiot_uint32 source_len);
cmiot_int32 cmiot_hex2dec(cmiot_char *hex);
cmiot_int32 cmiot_c2i(cmiot_char ch);
cmiot_bool  cmiot_str_is_same_char(cmiot_char *str, cmiot_uint32 len, cmiot_char ch);

#ifdef __cplusplus
}
#endif

#endif

/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        ope_types.h
 * 
 * @brief       Date type definition 
 * 
 * @details     Define the data types supported by the system
 * 
 * @revision
 * Date         Author          Notes
 * 2021-04-29   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __OPE_TYPES_H__
#define __OPE_TYPES_H__

#include <stdbool.h>
#include "ope_operating_sys.h"

#ifdef _OPE_WINDOWS_ /* Windows platform */

/* primary data type */
typedef char           ope_char_t;
typedef char           ope_int8_t;
typedef unsigned char  ope_uint8_t;
typedef short          ope_int16_t;
typedef unsigned short ope_uint16_t;
typedef int            ope_int32_t;
typedef unsigned int   ope_uint32_t;
typedef float          ope_float_t;
typedef double         ope_double_t;
typedef void           ope_void_t;
typedef bool           ope_bool_t;
typedef ope_int32_t    ope_err_t;

#define ope_true       true
#define ope_false      false
#define OPE_NULL       NULL

#elif defined _OPE_LINUX_ /* Linux platform */

/* primary data type */
typedef char           ope_char_t;
typedef char           ope_int8_t;
typedef unsigned char  ope_uint8_t;
typedef short          ope_int16_t;
typedef unsigned short ope_uint16_t;
typedef int            ope_int32_t;
typedef unsigned int   ope_uint32_t;
typedef float          ope_float_t;
typedef double         ope_double_t;
typedef void           ope_void_t;
typedef bool           ope_bool_t;
typedef ope_int32_t    ope_err_t;

#define ope_true       true
#define ope_false      false
#define OPE_NULL       NULL

#elif defined _OPE_ONEOS_ /* OneOS platform */
#include <os_types.h>
#include <os_stddef.h>

/* primary data type */
typedef os_int8_t      ope_char_t;
typedef os_int8_t      ope_int8_t;
typedef os_uint8_t     ope_uint8_t;
typedef os_int16_t     ope_int16_t;
typedef os_uint16_t    ope_uint16_t;
typedef os_int32_t     ope_int32_t;
typedef os_uint32_t    ope_uint32_t;
typedef float          ope_float_t;
typedef double         ope_double_t;
typedef void           ope_void_t;
typedef os_bool_t      ope_bool_t;
typedef os_err_t       ope_err_t;

#define ope_true       true
#define ope_false      false
#define OPE_NULL       OS_NULL

#else
#error Undefined platform
#endif 


#endif /* __OPE_TYPES_H__ */

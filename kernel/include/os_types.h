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
 * @file        os_types.h
 *
 * @brief       OneOS most common data type definitions
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-12   OneOS Team      First version
 ***********************************************************************************************************************
 */

#ifndef __OS_TYPES_H__
#define __OS_TYPES_H__

#include <oneos_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum value of base type */
#define OS_UINT8_MAX                    0xffU                   /* Maxium value of UINT8 */
#define OS_UINT16_MAX                   0xffffU                 /* Maxium value of UINT16 */
#define OS_UINT32_MAX                   0xffffffffU             /* Maxium value of UINT32 */
#define OS_UINT64_MAX                   0xffffffffffffffffULL   /* Maxium value of UINT64 */

#ifdef OS_ARCH_CPU_64BIT
#define OS_TICK_MAX                     OS_UINT64_MAX           /* Maxium value of tick */
#else
#define OS_TICK_MAX                     OS_UINT32_MAX           /* Maxium value of tick */
#endif

/* Alignment size for CPU architecture data access */
#define OS_ALIGN_SIZE                   sizeof(os_base_t)

/* Basic data type definitions */
typedef signed char                     os_int8_t;              /* 8bit integer type */
typedef signed short                    os_int16_t;             /* 16bit integer type */
typedef signed int                      os_int32_t;             /* 32bit integer type */
typedef signed long long                os_int64_t;             /* 64bit integer type */
typedef unsigned char                   os_uint8_t;             /* 8bit unsigned integer type */
typedef unsigned short                  os_uint16_t;            /* 16bit unsigned integer type */
typedef unsigned int                    os_uint32_t;            /* 32bit unsigned integer type */
typedef unsigned long long              os_uint64_t;            /* 64bit unsigned integer type */
typedef signed int                      os_bool_t;              /* Boolean type */

/* Represent Nbit CPU */
typedef signed long                     os_base_t;              /* Nbit CPU related date type */
typedef unsigned long                   os_ubase_t;             /* Nbit unsigned CPU related data type */

/* Else data type definitions */
typedef os_int32_t                      os_err_t;               /* Type for error number */
typedef os_ubase_t                      os_tick_t;              /* Type for tick count */ 
typedef os_ubase_t                      os_size_t;              /* Type for size */
typedef os_base_t                       os_off_t;               /* Type for offset */

#ifdef __cplusplus
}
#endif

#endif /* __OS_TYPES_H__ */


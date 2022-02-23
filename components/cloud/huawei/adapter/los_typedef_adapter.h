/*
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
 * @file        los_typedef_adapter.c
 *
 * @brief       huawei cloud sdk file "los_typedef.c" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __LOS_TYPEDEF_ADAPTER__
#define __LOS_TYPEDEF_ADAPTER__

#include <arpa/inet.h>
#include "os_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OS_STRING(x) #x
#define X_STRING(x)  OS_STRING(x)

/* type definitions */
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef signed char    INT8;
typedef signed short   INT16;
typedef signed int     INT32;
typedef float          FLOAT;
typedef double         DOUBLE;
typedef char           CHAR;

#ifdef __LP64__
typedef long unsigned int UINT64;
typedef long signed int   INT64;
typedef unsigned long     UINTPTR;
typedef signed long       INTPTR;
#else
typedef unsigned long long UINT64;
typedef signed long long   INT64;
typedef unsigned int       UINTPTR;
typedef signed int         INTPTR;
#endif

#ifdef __LP64__
typedef __uint128_t UINT128;
typedef INT64       ssize_t;
typedef UINT64      size_t;
#define LOSCFG_AARCH64
#else
// typedef INT32              ssize_t;
typedef UINT32 size_t;
#endif

typedef UINTPTR AARCHPTR;
typedef size_t  BOOL;

#define VOID   void
#define STATIC static

#ifndef FALSE
#define FALSE 0U
#endif

#ifndef TRUE
#define TRUE 1U
#endif

#ifndef NULL
#define NULL ((VOID *)0)
#endif

#ifdef YES
#undef YES
#endif
#define YES 1

#ifdef NO
#undef NO
#endif
#define NO 0

#define OS_NULL_BYTE  ((UINT8)0xFF)
#define OS_NULL_SHORT ((UINT16)0xFFFF)
#define OS_NULL_INT   ((UINT32)0xFFFFFFFF)

#ifndef LOS_OK
#define LOS_OK 0
#endif

#ifndef LOS_NOK
#define LOS_NOK 1
#endif

#define OS_FAIL 1
//#define OS_ERROR           (UINT32)(-1)
#define OS_INVALID (UINT32)(-1)

#define asm __asm
#ifdef typeof
#undef typeof
#endif
#define typeof __typeof__

#ifndef LOS_LABEL_DEFN
#define LOS_LABEL_DEFN(label) label
#endif

#ifndef LOSARC_ALIGNMENT
#define LOSARC_ALIGNMENT 8
#endif
/* And corresponding power of two alignment */
#ifndef LOSARC_P2ALIGNMENT
#ifdef LOSCFG_AARCH64
#define LOSARC_P2ALIGNMENT 3
#else
#define LOSARC_P2ALIGNMENT 2
#endif
#endif

/* Give a type or object explicit minimum alignment */
#if !defined(LOSBLD_ATTRIB_ALIGN)
#define LOSBLD_ATTRIB_ALIGN(__align__) __attribute__((aligned(__align__)))
#endif

/* Assign a defined variable to a specific section */
#if !defined(LOSBLD_ATTRIB_SECTION)
#define LOSBLD_ATTRIB_SECTION(__sect__) __attribute__((section(__sect__)))
#endif

/*
 * Tell the compiler not to throw away a variable or function. Only known
 * available on 3.3.2 or above. Old version's didn't throw them away,
 * but using the unused attribute should stop warnings.
 */
#define LOSBLD_ATTRIB_USED __attribute__((used))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LOS_TYPEDEF_ADAPTER__ */

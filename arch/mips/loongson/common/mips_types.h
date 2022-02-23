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
 * @file        mips_types.h
 *
 * @brief       mips types define
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _MIPS_TYPES_H_
#define _MIPS_TYPES_H_

#ifndef __ASSEMBLY__

typedef unsigned short umode_t;

/*
 * __xx is ok: it doesn't pollute the POSIX namespace. Use these in the
 * header files exported to user space
 */

typedef __signed__ char     __s8;
typedef unsigned char       __u8;

typedef __signed__ short    __s16;
typedef unsigned short      __u16;

typedef __signed__ int      __s32;
typedef unsigned int        __u32;

#if (_MIPS_SZLONG == 64)

typedef __signed__ long     __s64;
typedef unsigned long       __u64;

#else

#if defined(__GNUC__)
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#endif

#endif

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */

#define BITS_PER_LONG _MIPS_SZLONG


typedef __signed char s8;
typedef unsigned char u8;

typedef __signed short s16;
typedef unsigned short u16;

typedef __signed int s32;
typedef unsigned int u32;

#if (_MIPS_SZLONG == 64)

typedef __signed__ long s64;
typedef unsigned long u64;

#else

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef __signed__ long long s64;
typedef unsigned long long u64;
#endif

#endif

typedef u32 dma_addr_t;

typedef u32 phys_addr_t;
typedef u32 phys_size_t;


/*
 * Don't use phys_t.  You've been warned.
 */

typedef unsigned long phys_t;

#endif /* __ASSEMBLY__ */


#endif /* _MIPS_TYPES_H_ */


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
 * @file        os_stddef.h
 *
 * @brief       OneOS standard macro definitions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-13   OneOS Team      First version
 * 2020-11-01   OneOS Team      1. Modify macros difinition of automatic initialization.
 *                              2. Add new macros difinition. OS_WAIT_FOREVER, OS_NO_WAIT, OS_UNREFERENCE.
 ***********************************************************************************************************************
 */

#ifndef __OS_STDDEF_H__
#define __OS_STDDEF_H__

#include <os_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define __CLANG_ARM
#endif

/* Compiler related definitions */
#if defined(__CC_ARM) || defined(__CLANG_ARM)   /* For ARM compiler */
    #include <stdarg.h>
    
    #define OS_SECTION(x)               __attribute__((section(x)))
    #define OS_ALIGN(n)                 __attribute__((aligned(n)))
    #define OS_UNUSED                   __attribute__((unused))
    #define OS_USED                     __attribute__((used))
    #define OS_WEAK                     __attribute__((weak))
    #define OS_INLINE                   static __inline
#elif defined (__IAR_SYSTEMS_ICC__)             /* For IAR compiler */
    #include <stdarg.h>
    
    #define OS_SECTION(x)               @ x
    #define OS_PRAGMA(x)                _Pragma(#x)
    #define OS_ALIGN(n)                 OS_PRAGMA(data_alignment=n)
    #define OS_UNUSED
    #define OS_USED                     __root
    #define OS_WEAK                     __weak
    #define OS_INLINE                   static inline
#elif defined (__GNUC__)                        /* For GNU GCC compiler */
    #include <stdarg.h>

    #define OS_SECTION(x)               __attribute__((section(x)))
    #define OS_ALIGN(n)                 __attribute__((aligned(n)))
    #define OS_UNUSED                   __attribute__((unused))
    #define OS_USED                     __attribute__((used))
    
    #define OS_WEAK                     __attribute__((weak))
    #define OS_INLINE                   static __inline
#else
    #error "Not supported the tool chain."
#endif

/******************************************** Begin: Auto initialization **********************************************/
typedef os_err_t (*os_init_fn_t)(void);

#define OS_INIT_SUBLEVEL_HIGH          "1"
#define OS_INIT_SUBLEVEL_MIDDLE        "2"
#define OS_INIT_SUBLEVEL_LOW           "3"

#define OS_INIT_EXPORT(fn, level, sublevel)                                                           \
    OS_USED const os_init_fn_t  _os_call_##fn OS_SECTION(".init_call." level sublevel) = fn

/* Core init routines(hardware initialization required for kernel starting) should be called before the kernel starts */
#define OS_CORE_INIT(fn, sublevel)      OS_INIT_EXPORT(fn, "1.", sublevel)

/* postcore/prev/device/component/env/app init routines will be called before main() function */
#define OS_POSTCORE_INIT(fn, sublevel)  OS_INIT_EXPORT(fn, "2.", sublevel)  /* hardware initialization after kernel starting */
#define OS_PREV_INIT(fn, sublevel)      OS_INIT_EXPORT(fn, "3.", sublevel)  /* Pre-initialization(pure software initilization) */
#define OS_DEVICE_INIT(fn, sublevel)    OS_INIT_EXPORT(fn, "4.", sublevel)  /* Device initialization */
#define OS_CMPOENT_INIT(fn, sublevel)   OS_INIT_EXPORT(fn, "5.", sublevel)  /* Components initialization (vfs, lwip, ...) */
#define OS_ENV_INIT(fn, sublevel)       OS_INIT_EXPORT(fn, "6.", sublevel)  /* Environment initialization (mount disk, ...) */
#define OS_APP_INIT(fn, sublevel)       OS_INIT_EXPORT(fn, "7.", sublevel)  /* Appliation initialization */
/******************************************** End: Auto initialization ************************************************/


/* Similar as the NULL in C library */
#ifdef __cplusplus
#define OS_NULL                         0
#else
#define OS_NULL                         ((void *)0)
#endif

/* Boolean value definitions */
#define OS_FALSE                        0
#define OS_TRUE                         1


/* Return the most contiguous size aligned at specified width. OS_ALIGN_UP(13, 4) would return 16. */
#define OS_ALIGN_UP(size, align)        (((size) + (align) - 1) & ~((align) - 1))

/* Return the down number of aligned at specified width. OS_ALIGN_DOWN(13, 4) would return 12. */
#define OS_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))

/* Calculate array size */
#define OS_ARRAY_SIZE(x)                (sizeof(x) / sizeof(x[0]))

#define OS_WAIT_FOREVER                 OS_TICK_MAX
#define OS_NO_WAIT                      0U

#define OS_UNREFERENCE(x)               ((void)(x))

#define OS_SET_BIT(value, bit)          ((value) |= (1 << (bit)))
#define OS_CLEAR_BIT(value, bit)        ((value) &= ~(1 << (bit)))

/**
 ***********************************************************************************************************************
 * @def         os_container_of
 *
 * @brief       Cast a member of a structure out to the containing structure.
 *
 * @param       ptr             The pointer to the member.
 * @param       type            The type of the container struct this is embedded in.
 * @param       member          The name of the member within the struct. 
 ***********************************************************************************************************************
 */
#define os_container_of(ptr, type, member)      \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 ***********************************************************************************************************************
 * @def         os_offsetof
 *
 * @brief       This macro will return a byte offset of a member to the beginning of the struct.
 *
 * @param       type            The type of the struct
 * @param       member          The name of the member within the struct.
 ***********************************************************************************************************************
 */
#define os_offsetof(type, member)       ((os_size_t) &((type *)0)->member)

#ifdef __cplusplus
}
#endif

#endif /* __OS_STDDEF_H__ */


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
 * @file        nimble_npl_os.h
 *
 * @brief       Define the type used for stack.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _NIMBLE_NPL_OS_H_
#define _NIMBLE_NPL_OS_H_

#include <assert.h>
#include <stdint.h>
#include <string.h>

/**
 * 'OS_ALIGN' is defined in a different implementation at Nimble stack 
 * 'components\ble\mynewt-nimble\porting\nimble\include\os\os.h' and Oneos 'kernel\include\os_stddef.h' when compiler is keil. 
 * We need to include Macro defined in 'libc\include\extension\errno_ext.h' but not include Macro defined in 'kernel\include\os_stddef.h' 
 * when this header file is used in Nimble.
 */
#if defined(OS_ALIGN)           // Whether OS_ALIGN is already defined.
#if defined(__OS_STDDEF_H__)    // Whether this header is used in Nimble.
#define NIMBLE_STACK_NOTUSE
#else
#define NIMBLE_STACK_USE
#endif
#undef OS_ALIGN                 // undefine OS_ALIGN to void compile warning.
#endif
#include <errno_ext.h>
#undef OS_ALIGN                 // OS_ALIGN is undefined there!!!
#if defined(NIMBLE_STACK_NOTUSE)
    /* Compiler related definitions */
    #if defined(__CC_ARM) || defined(__CLANG_ARM)   /* For ARM compiler */
        #define OS_ALIGN(n)                 __attribute__((aligned(n)))
    #elif defined (__IAR_SYSTEMS_ICC__)             /* For IAR compiler */
        #define OS_ALIGN(n)                 OS_PRAGMA(data_alignment=n)
    #elif defined (__GNUC__)                        /* For GNU GCC compiler */
        #define OS_ALIGN(n)                 __attribute__((aligned(n)))
    #else
        #error "Not supported the tool chain."
    #endif
#else
    #define OS_ALIGN(__n, __a) (                             \
            (((__n) & ((__a) - 1)) == 0)                   ? \
                (__n)                                      : \
                ((__n) + ((__a) - ((__n) & ((__a) - 1))))    \
            )
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_NPL_OS_ALIGNMENT    4

// typedef unsigned int                    os_uint32_t;            /* 32bit unsigned integer type */
// typedef os_uint32_t                     os_tick_t;              /* Type for tick count */ 
#ifndef OS_IPC_WAITING_FOREVER
#define OS_IPC_WAITING_FOREVER    ((unsigned int)0xFFFFFFFF)
#endif

#ifndef OS_IPC_WAITING_NO
#define OS_IPC_WAITING_NO         ((unsigned int)0)
#endif

#define BLE_NPL_TIME_FOREVER    (OS_IPC_WAITING_FOREVER)

/* ble_npl_time_t have same definition with os_tick_t. */
typedef uint32_t ble_npl_time_t;
typedef int32_t ble_npl_stime_t;

struct ble_npl_task
{
    void *t;
};

struct ble_npl_event
{
    bool queued;
    ble_npl_event_fn *fn;
    void *arg;
};

struct ble_npl_eventq
{
    void *q;
};

struct ble_npl_callout
{
    void *handle;
    struct ble_npl_eventq *evq;
    struct ble_npl_event ev;
};

struct ble_npl_mutex
{
    void *handle;
};

struct ble_npl_sem
{
    void  *handle;
};

static inline 
bool ble_npl_os_started(void)
{
    return true;
}

void nimble_port_oneos_init(void);

void ble_hs_task_startup(void);

void nimble_task_init(struct ble_npl_task *ble_npl_task,
                      const char *name,
                      void (*entry)(void *parameter),
                      void *parameter,
                      uint8_t priority_invaild,
                      uint32_t sanity_itvl_invaild,
                      void *stack_start,
                      uint32_t stack_size);

void ble_npl_eventq_run(void *evq);

#ifdef __cplusplus
}
#endif

#endif

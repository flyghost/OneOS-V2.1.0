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
 * @file        os_startup.c
 *
 * @brief       The starting process of OneOS.
 *
 * @details     During starting process, do something as follows:
 *                  1) Board initialization before scheduler is started (OS_CORE_INIT).
 *                  2) Tick queue initialization.
 *                  3) Scheduler system initialization.
 *                  4) Timer system initialization.
 *                  5) Recycle task initialization.
 *                  6) Idle task initialization.
 *                  7) Application initialization. The important job is to create a main task.               
 *                  8) Start scheduler
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-01   OneOS Team      First version
 * 2020-11-05   OneOS Team      Refactor starting process.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_errno.h>

#include "os_kernel_internal.h"

#define STARTUP_TAG                 "STARTUP"

#ifdef OS_USING_SYS_HEAP
static os_task_t *gs_os_main_task = OS_NULL;
#else
static OS_TASK_STACK_DEFINE(gs_os_main_stack, OS_MAIN_TASK_STACK_SIZE);
static os_task_t  gs_os_main_task = {0};
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * Auto Initialization will initialize some driver and components as following order:
 * 
 * os_core_init_start       --> 1.
 
 * OS_CORE_INIT             --> 1.x
 
 * os_postcore_init_start   --> 1.end
 
 * OS_POSTCORE_INIT         --> 2.x
 * OS_PREV_INIT             --> 3.x
 * OS_DEVICE_INIT           --> 4.x
 * OS_CMPOENT_INIT          --> 5.x
 * OS_ENV_INIT              --> 6.x
 * OS_APP_INIT              --> 7.x
 *
 * os_init_end              --> 7.end
 *
 * If you want to use the automatic initialization mechanism, initial function must be defined with:
 * OS_CORE_INIT(fn, sublevel);
 * OS_POSTCORE_INIT(fn, sublevel);
 * OS_PREV_INIT(fn, sublevel);
 * OS_DEVICE_INIT(fn, sublevel)
 * ...
 * OS_APP_INIT(fn, sublevel);
 * 
 * 'sublevel' is OS_INIT_SUBLEVEL_HIGH, OS_INIT_SUBLEVEL_MIDDLE or OS_INIT_SUBLEVEL_LOW.
 ***********************************************************************************************************************
 */

static os_err_t os_core_init_start(void)
{
    return OS_EOK;
}
OS_INIT_EXPORT(os_core_init_start, "1.", "");

static os_err_t os_postcore_init_start(void)
{
    return OS_EOK;
}
OS_INIT_EXPORT(os_postcore_init_start, "1.end", "");

static os_err_t os_init_end(void)
{
    return OS_EOK;
}
OS_INIT_EXPORT(os_init_end, "7.end", "");


/**
 ***********************************************************************************************************************
 * @brief           Automatic initialization before scheduler starts.
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
static os_err_t _k_core_auto_init(void)
{
    const os_init_fn_t *fn_ptr_core_init_start;
    const os_init_fn_t *fn_ptr_core_init_end;
    const os_init_fn_t *fn_ptr;
    os_err_t            ret;

    fn_ptr_core_init_start = &_os_call_os_core_init_start + 1;
    fn_ptr_core_init_end   = &_os_call_os_postcore_init_start - 1;
    
    for (fn_ptr = fn_ptr_core_init_start; fn_ptr <= fn_ptr_core_init_end; fn_ptr++)
    {
        ret = (*fn_ptr)();
        if (ret != OS_EOK)
        {
            return ret; 
        }
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Automatic initialization after scheduler starts (postcore/prev/device/component/env/app).
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
static os_err_t _k_other_auto_init(void)
{
    const os_init_fn_t *fn_ptr_other_init_start;
    const os_init_fn_t *fn_ptr_other_init_end;
    const os_init_fn_t *fn_ptr;
    os_err_t            ret;

    fn_ptr_other_init_start = &_os_call_os_postcore_init_start + 1;
    fn_ptr_other_init_end   = &_os_call_os_init_end - 1;

    for (fn_ptr = fn_ptr_other_init_start; fn_ptr <= fn_ptr_other_init_end; fn_ptr++)
    {
        ret = (*fn_ptr)();
        if (ret != OS_EOK)
        {
            return ret; 
        }
    }

    return OS_EOK;
}

static void _k_main_task_entry(void *arg)
{
    os_err_t ret;

    OS_UNREFERENCE(arg);

    /* Auto initialization for pre/device/component/env/app */
    ret = _k_other_auto_init();
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Automatic initialization after kernel startup failed");
    }

/* Invoke system main function */
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern int $Super$$main(void);
    $Super$$main();
#elif defined(__ICCARM__) || defined(__GNUC__)
    extern int main(void);
    main();
#endif

    return;
}

static void _k_application_init(void)
{
    os_err_t ret;

#ifdef OS_USING_SYS_HEAP
    gs_os_main_task = os_task_create("main",
                                     _k_main_task_entry,
                                     OS_NULL,
                                     OS_MAIN_TASK_STACK_SIZE,
                                     0U);
    if (OS_NULL == gs_os_main_task)
    {
        OS_ASSERT_EX(0, "Why create main task failed?");
    }

    ret = os_task_startup(gs_os_main_task);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why startup main task failed?");
    }  

#else 
    ret = os_task_init(&gs_os_main_task,
                       "main", 
                       _k_main_task_entry, 
                       OS_NULL,
                       OS_TASK_STACK_BEGIN_ADDR(gs_os_main_stack),
                       OS_TASK_STACK_SIZE(gs_os_main_stack),
                       0U);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why initialize main task failed?");
    }

    ret = os_task_startup(&gs_os_main_task);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why startup main task failed?");
    }
#endif /* OS_USING_SYS_HEAP */

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Startup OneOS.
 *
 * @param           No parameter.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
static void _k_startup(void)
{
    os_err_t ret;
    
    /* Board initialization */
    ret = _k_core_auto_init();
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Automatic initialization before kernel startup failed");
    }

    OS_KERN_LOG(KERN_WARNING, STARTUP_TAG, "OneOS kernel start, version: %s", OS_KERNEL_VERSION);

    k_tickq_init();

    k_sched_init();

#ifdef OS_USING_TIMER
    k_timer_module_init();
#endif

    k_recycle_task_init();

    k_idle_task_init();

    _k_application_init();

    k_start();

    return;
}

#if defined(__CC_ARM) || defined(__CLANG_ARM)
/* Re-define main function */
int $Sub$$main(void)
{
    /* Start up the operating system */
    _k_startup();
    
    return OS_EOK;
}

#elif defined(__ICCARM__)
extern void __iar_data_init3(void);

/* __low_level_init will auto called by IAR c-startup */
int __low_level_init(void)
{
    /* Call IAR table copy function. */
    __iar_data_init3();

    /* Start up the operating system */
    _k_startup();
    
    return OS_EOK;
}

#elif defined(__GNUC__)
/* Add -eentry to arm-none-eabi-gcc argument */
int entry(void)
{
    /* Start up the operating system */
    _k_startup();
    
    return OS_EOK;
}
#endif


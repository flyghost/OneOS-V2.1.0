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
 * @file        os_safety.c
 *
 * @brief       This file implements the safety functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-08   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <arch_interrupt.h>

#ifdef OS_USING_SAFETY_MECHANISM
static void (*gs_os_safety_task_stack_overflow_hook)(void) = OS_NULL;
static void (*gs_os_safety_assert_hook)(void)              = OS_NULL;
static void (*gs_os_safety_exception_hook)(void)           = OS_NULL;

void os_safety_task_stack_overflow_process(void)
{
    if (OS_NULL != gs_os_safety_task_stack_overflow_hook)
    {
        gs_os_safety_task_stack_overflow_hook();
    }
    else
    {
        os_irq_disable();
        while(1);
    }
    
    return;
}

void os_safety_assert_process(void)
{
    if (OS_NULL != gs_os_safety_assert_hook)
    {
        gs_os_safety_assert_hook();
    }
    else
    {
        os_irq_disable();
        while(1);
    }
    
    return;
}

void os_safety_exception_process(void)
{
    if (OS_NULL != gs_os_safety_exception_hook)
    {
        gs_os_safety_exception_hook();
    }
    else
    {
        os_irq_disable();
        while(1);
    }

    return;
}

void os_safety_task_stack_overflow_set_hook(void (*hook)(void))
{
    gs_os_safety_task_stack_overflow_hook = hook;
}

void os_safety_assert_set_hook(void (*hook)(void))
{
    gs_os_safety_assert_hook = hook;
}

void os_safety_exception_set_hook(void (*hook)(void))
{
    gs_os_safety_exception_hook = hook;
}
#endif /* OS_USING_SAFETY_MECHANISM */


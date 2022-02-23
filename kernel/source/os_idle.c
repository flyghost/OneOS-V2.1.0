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
 * @file        os_idle.c
 *
 * @brief       This file implements the idle task.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-24   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <arch_interrupt.h>

#include "os_kernel_internal.h"

#define IDLE_TAG                    "IDLE"

static OS_TASK_STACK_DEFINE(gs_os_idle_stack, OS_IDLE_TASK_STACK_SIZE);
static os_task_t gs_os_idle_task;
#ifdef OS_USING_TICKLESS_LPMGR
extern void os_low_power_manager(void);
#endif

static void _k_idle_task_entry(void *arg)
{
    OS_UNREFERENCE(arg);

    while (1)
    {
        /* TODO: */
        ;
#ifdef OS_USING_TICKLESS_LPMGR
        os_low_power_manager();
#endif
    }
}

void k_idle_task_init(void)
{
    os_err_t ret;
    
    /* Initialize idle task */
    ret = os_task_init(&gs_os_idle_task,
                       OS_IDLE_TASK_NAME,
                       _k_idle_task_entry,
                       OS_NULL,
                       OS_TASK_STACK_BEGIN_ADDR(gs_os_idle_stack),
                       OS_TASK_STACK_SIZE(gs_os_idle_stack),
                       OS_TASK_PRIORITY_MAX - 1);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why initialize idle task failed?");
    }

    /* Startup */
    ret = os_task_startup(&gs_os_idle_task);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why startup idle task failed?");
    }
    
    return;
}


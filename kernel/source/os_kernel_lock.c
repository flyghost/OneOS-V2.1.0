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
 * @file        os_readyq.c
 *
 * @brief       This file implements the sched functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_spinlock.h>
#include <arch_interrupt.h>
#include "os_kernel_internal.h"


#ifdef OS_CONFIG_SMP

/* TODO: */

#else

os_int32_t g_os_kernel_lock_cnt = 0;

void k_kernel_enter_check(void)
{
#ifdef OS_USING_KERNEL_LOCK_CHECK
    os_bool_t fault_active;

    fault_active = os_is_fault_active();
    
    if (OS_NULL != g_os_current_task)
    {
        /* Exception context. */
        if (OS_TRUE == fault_active)
        {
            /* Do nothing. */
            ;
        }
        else
        {
            OS_ASSERT_EX(0 == g_os_kernel_lock_cnt, "Kernel enter check failed, task(%s), irq(%u), ref_cnt(%d)",
                         g_os_current_task->name,
                         os_irq_num(),
                         g_os_kernel_lock_cnt);

            g_os_kernel_lock_cnt++;
        }
    }
#endif

    return;
}

void k_kernel_exit_check(void)
{
#ifdef OS_USING_KERNEL_LOCK_CHECK
    os_bool_t fault_active;
    
    fault_active = os_is_fault_active();

    if (OS_NULL != g_os_current_task)
    {
        /* Exception context. */
        if (OS_TRUE == fault_active)
        {
            /* Do nothing. */
            ;
        }
        else
        {
            OS_ASSERT_EX(1 == g_os_kernel_lock_cnt, "Kernel exit check failed, task(%s), irq(%u), ref_cnt(%d)",
                         g_os_current_task->name,
                         os_irq_num(),
                         g_os_kernel_lock_cnt);

            g_os_kernel_lock_cnt--;
        }
    }
#endif

    return;
}
#endif /* OS_CONFIG_SMP */


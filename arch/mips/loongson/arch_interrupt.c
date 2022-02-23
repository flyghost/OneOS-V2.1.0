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
 * @file        arch_interrupt.c
 *
 * @brief       This file provides interrupt related functions under the mips architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-16   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <arch_hw.h>
#include <mips_regs.h>

volatile os_uint32_t g_os_interrupt_nest = 0;

os_ubase_t os_irq_lock(void)
{
    return os_hw_interrupt_disable();
}

void os_irq_unlock(os_ubase_t irq_save)
{
    os_hw_interrupt_enable(irq_save);
}

void os_irq_disable(void)
{
    clear_c0_status(ST0_IE);
}
void os_irq_enable(void)
{
    set_c0_status(ST0_IE);
}

os_bool_t os_is_irq_active(void)
{
    return ((g_os_interrupt_nest > 0) ? OS_TRUE : OS_FALSE);
}

os_bool_t os_is_irq_disabled(void)
{
    os_base_t status = read_c0_status();
    return (((status & ST0_IE) ==  ST0_IE)? OS_FALSE : OS_TRUE);
}

os_uint32_t os_irq_num(void)
{
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           Determine whether the current context is an exception context.
 *
 * @detail          All exception vector numbers are as follows: Hard Fault:3 MemManage Fault:4 Bus Fault:5
 *                  Usage Fault:6.Therefore, in the exception context, the range of interrupt vector number is [3,6].
 *
 * @param           None.
 *
 * @return          Return 1 in exception context, otherwise return 0.
 * @retval          1               In exception context.
 * @retval          0               In other context.
 ***********************************************************************************************************************
 */
OS_WEAK os_bool_t os_is_fault_active(void)
{
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function increase the interrupt nesting level by 1.
 *
 * @attention       This function is supposed to be called every time we enter an interrupt.
 *
 * @param           None
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_interrupt_enter(void)
{
    os_base_t level;

    level = os_irq_lock();

    g_os_interrupt_nest++;

    os_irq_unlock(level);
}

/**
 ***********************************************************************************************************************
 * @brief           This function decrease the interrupt nesting level by 1.
 *
 * @attention       This function is supposed to be called every time we leave an interrupt.
 *
 * @param           None
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void os_interrupt_leave(void)
{
    os_base_t level;

    level = os_irq_lock();

    g_os_interrupt_nest--;

    os_irq_unlock(level);
}

/**
 ***********************************************************************************************************************
 * @brief           This function get the interrupt nesting level.
 *
 * @param           None
 *
 * @return          Return the interrupt nesting level.
 ***********************************************************************************************************************
 */
os_uint32_t os_interrupt_get_nest(void)
{
    return g_os_interrupt_nest;
}


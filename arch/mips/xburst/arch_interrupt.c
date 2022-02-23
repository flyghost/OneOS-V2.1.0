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

volatile os_uint32_t g_os_interrupt_nest = 0;


#if defined(__GNUC__)
OS_WEAK os_ubase_t os_irq_lock(void)
{
    /*Implemented in mips_context_gcc.S file.*/
    return 0;
}

OS_WEAK void os_irq_unlock(os_ubase_t irq_save)
{
    /*Implemented in mips_context_gcc.S file.*/
    return;

}

OS_WEAK void os_irq_disable(void)
{
    /*Implemented in mips_context_gcc.S file.*/
    return;
}

OS_WEAK void os_irq_enable(void)
{
    /*Implemented in mips_context_gcc.S file.*/
    return;
}

OS_WEAK os_bool_t os_is_irq_active(void)
{
    if(g_os_interrupt_nest > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

OS_WEAK os_bool_t os_is_irq_disabled(void)
{

    /*Implemented in mips_context_gcc.S file.*/
    return 0;
}

OS_WEAK os_uint32_t os_irq_num(void)
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
    if(g_os_interrupt_nest > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#endif

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




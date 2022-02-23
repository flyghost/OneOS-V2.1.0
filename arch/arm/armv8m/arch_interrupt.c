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
 * @brief       This file provides interrupt related functions under the ARMv8-M architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>

#if defined(__GNUC__) || defined(__CLANG_ARM)
/**
 ***********************************************************************************************************************
 * @brief           Get irq status and disable irq.
 *
 * @param           None
 *
 * @return          0:    irq enable
 *                  1:    irq disable
 ***********************************************************************************************************************
 */
os_ubase_t os_irq_lock(void)
{
    os_ubase_t primask;

    __asm__ __volatile__(
        "MRS     %0, PRIMASK\n"
        "CPSID   I"
        : "=r"(primask)
        : 
        : "memory");

    return primask;
}

/**
 ***********************************************************************************************************************
 * @brief           Restore irq enable state.
 *
 * @param[in]       irq_save          irq status
 *
 * @return          None
 ***********************************************************************************************************************
 */
void os_irq_unlock(os_ubase_t irq_save)
{
    __asm__ __volatile__(
        "MSR     PRIMASK, %0"
        : 
        : "r"(irq_save)
        : "memory");

    return;
}

void os_irq_disable(void)
{
    __asm__ __volatile__(
        "CPSID   I"
        :
        : 
        :);

    return;
}

void os_irq_enable(void)
{
    __asm__ __volatile__(
        "CPSIE   I"
        :
        : 
        :);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the current context state,not 0: irq context  0: task context.
 *
 * @param           None
 *
 * @return          0:        irq context.
 *                  not 0:    task context.
 ***********************************************************************************************************************
 */
os_bool_t os_is_irq_active(void)
{
    os_bool_t active;

    __asm__ __volatile__(
        "   MRS     %0, IPSR\n"
        "   CBZ     %0, in_task_context\n"
        "   MOV     %0, #1\n"
        "in_task_context:"
        :   "=r"(active)
        :
        :   "memory");

    return active;
}

/**
 ***********************************************************************************************************************
 * @brief           Get irq status.
 *
 * @param           None
 *
 * @return          0:    irq enable
 *                  1:    irq disable
 ***********************************************************************************************************************
 */
os_bool_t os_is_irq_disabled(void)
{
    os_bool_t disabled;

    __asm__ __volatile__(
        "   MRS     %0, PRIMASK\n"
        :   "=r"(disabled)
        :
        :   "memory");

    return disabled;
}

/**
 ***********************************************************************************************************************
 * @brief           Get irq num.
 *
 * @param           None
 *
 * @return          irq num
 ***********************************************************************************************************************
 */
os_uint32_t os_irq_num(void)
{
    os_uint32_t irq_num;

    __asm__ __volatile__(
        "   MRS     %0, IPSR\n"
        :   "=r"(irq_num)
        :
        :);

    return irq_num;
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
os_bool_t os_is_fault_active(void)
{
    os_bool_t active;
    os_bool_t fault_low;
    os_bool_t fault_high;

    __asm__ __volatile__(
        "   MRS     %0, IPSR\n"
        "   MOV     %1, #3\n"
        "   MOV     %2, #6\n"
        "   CMP     %0, %1\n"
        "   BLT     noactive\n"
        "   CMP     %0, %2\n"
        "   BGT     noactive\n"
        "   MOV     %0, #1\n"
        "   B       end\n"
        "noactive:         \n"
        "   MOV     %0, #0\n"
        "end:         \n"
        :   "=r"(active),"=r"(fault_low),"=r"(fault_high)
        :
        :   "cc","memory");

    return active;
}

#elif defined(__CC_ARM)

/**
 ***********************************************************************************************************************
 * @brief           Get irq status and disable irq.
 *
 * @param           None
 *
 * @return          0:    irq enable
 *                  1:    irq disable
 ***********************************************************************************************************************
 */
__asm os_ubase_t os_irq_lock(void)
{
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR
}

/**
 ***********************************************************************************************************************
 * @brief           Restore irq enable state.
 *
 * @param[in]       irq_save          irq status
 *
 * @return          None
 ***********************************************************************************************************************
 */
__asm void os_irq_unlock(os_ubase_t irq_save)
{
    MSR     PRIMASK, R0
    BX      LR
}

__asm void os_irq_disable(void)
{
    CPSID   I
    BX      LR
}

__asm void os_irq_enable(void)
{
    CPSIE   I
    BX      LR
}

/**
 ***********************************************************************************************************************
 * @brief           Get the current context state,not 0: irq context  0: task context.
 *
 * @param           None
 *
 * @return          0:        irq context.
 *                  not 0:    task context.
 ***********************************************************************************************************************
 */
__asm os_bool_t os_is_irq_active(void)
{
    MRS     R0, IPSR
    CBZ     R0, in_task_context
    MOV     R0, #1

in_task_context
    BX      LR
}

/**
 ***********************************************************************************************************************
 * @brief           Get irq status.
 *
 * @param           None
 *
 * @return          0:    irq enable
 *                  1:    irq disable
 ***********************************************************************************************************************
 */
__asm os_bool_t os_is_irq_disabled(void)
{
    MRS     R0, PRIMASK
    BX      LR
}

/**
 ***********************************************************************************************************************
 * @brief           Get irq num.
 *
 * @param           None
 *
 * @return          irq num
 ***********************************************************************************************************************
 */
__asm os_uint32_t os_irq_num(void)
{
    MRS     R0, IPSR
    BX      LR
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
__asm os_bool_t os_is_fault_active(void)
{
    MRS     R0, IPSR
    MOV     R1, #3
    MOV     R2, #6
    CMP     R0, R1
    BLT     noactive
    CMP     R0, R2
    BGT     noactive
    MOV     R0, #1
    BX      LR
noactive
    MOV     R0, #0
    BX      LR
}

#endif


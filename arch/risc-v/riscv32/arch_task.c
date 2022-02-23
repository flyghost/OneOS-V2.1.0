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
 * @file        cpuport.c
 *
 * @brief       This file provides functions related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_types.h>
#include <string.h>
#include <os_stddef.h>
#include "arch_task.h"
#include "riscv_bits.h"
#include <os_assert.h>

volatile os_uint32_t os_task_switch_interrupt_flag = 0;

/**
 ***********************************************************************************************************************
 * @brief           This function initializes the task stack space.
 *
 * @param[in]       task_entry      The entry of task.
 * @param[in]       parameter       The parameter of task.
 * @param[in]       stack_addr      Stack start address.
 * @param[in]       task_exit       The function will be called when task exit.
 *
 * @return          Task's current stack address.
 ***********************************************************************************************************************
 */
void *os_hw_stack_init(void *tentry, void *parameter, void *stack_begin, os_uint32_t stack_size, void *texit)
{
    os_int32_t                i;
    os_uint8_t               *stk;
    struct os_hw_stack_frame *frame;

    memset(stack_begin, '$', stack_size);

    stk  = (os_uint8_t *)stack_begin + stack_size;
    stk  = (os_uint8_t *)OS_ALIGN_DOWN((os_ubase_t)stk, OS_ARCH_STACK_ALIGN_SIZE);
    stk -= sizeof(struct os_hw_stack_frame);

    frame = (struct os_hw_stack_frame *)stk;

    for (i = 0; i < sizeof(struct os_hw_stack_frame) / sizeof(os_ubase_t); i++)
    {
        ((os_ubase_t *)frame)[i] = 0xdeadbeef;
    }

    frame->ra      = (os_ubase_t)texit;
    frame->a0      = (os_ubase_t)parameter;
    frame->epc     = (os_ubase_t)tentry;

    /* force to machine mode(MPP=11) and set MPIE to 1 */
    frame->mstatus = 0x00001880;

    return (void *)stk;
}

/**
 ***********************************************************************************************************************
 * @brief           This function gets the maximum usage of the task stack.
 *
 * @param[in]       stack_begin     Stack start address.
 * @param[in]       stack_size      Stack size.
 *
 * @return          Maximum used value of task stack.
 ***********************************************************************************************************************
 */
os_uint32_t os_hw_stack_max_used(void *stack_begin, os_uint32_t stack_size)
{
    os_uint8_t *addr;
    os_uint32_t max_used;

    addr = (os_uint8_t *)stack_begin;
    while (*addr == '$')
    {
        addr++;
    }

    max_used = (os_uint32_t)((os_ubase_t)stack_begin + stack_size - (os_ubase_t)addr);

    return max_used;
}

#ifdef OS_USING_OVERFLOW_CHECK
/**
 ***********************************************************************************************************************
 * @brief           This function is used to check the stack of task is overflow or not.
 *
 * @param[in]       task            The descriptor of task control block
 * 
 * @return          Whether the stack of this task overflows.
 * @retval          OS_TRUE         The stack of this task is overflow.
 * @retval          OS_FALSE        The stack of this task is not overflow.
 ***********************************************************************************************************************
 */
os_bool_t os_task_stack_is_overflow(void *stack_top, void *stack_begin, void *stack_end)
{
    os_bool_t  is_overflow;
    
    OS_ASSERT(OS_NULL != stack_top);
    OS_ASSERT(OS_NULL != stack_begin);
    OS_ASSERT(OS_NULL != stack_end);

    if ((*((os_uint8_t *)stack_begin) != '$')
        || ((os_ubase_t)stack_top < (os_ubase_t)stack_begin)
        || ((os_ubase_t)stack_top >= (os_ubase_t)stack_end))
    {
        is_overflow = OS_TRUE;
    }
    else
    {
        is_overflow = OS_FALSE;
    }

    return is_overflow;
}
#endif /* OS_USING_OVERFLOW_CHECK */


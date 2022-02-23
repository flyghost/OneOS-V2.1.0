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
 * @file        arch_task.c
 *
 * @brief       This file provides functions related to the ARMv8-M architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_types.h>
#include <os_task.h>
#include <string.h>
#include <os_assert.h>
#include <arch_task.h>

/**
 ***********************************************************************************************************************
 * @brief           This function initializes the task stack space.
 *
 * @param[in]       task_entry      The entry of task.
 * @param[in]       arg             The parameter of task.
 * @param[in]       stack_begin     Stack start address.
 * @param[in]       stack_size      Stack size.
 * @param[in]       task_exit       The function will be called when task exit.
 *
 * @return          Task's current stack address.
 ***********************************************************************************************************************
 */
void *os_hw_stack_init(void (*task_entry)(void *arg), void *arg, void *stack_begin, os_uint32_t stack_size, void (*task_exit)(void))
{
    struct stack_frame    *stack_frame;
    os_uint8_t            *stack_top;
    os_uint32_t            index;

    memset(stack_begin, '$', stack_size);

    stack_top  = (os_uint8_t *)stack_begin + stack_size;
    /*Compatible with old interfaces*/
    stack_top  = (os_uint8_t *)OS_ALIGN_DOWN((os_ubase_t)stack_top, OS_ARCH_STACK_ALIGN_SIZE);
    stack_top -= sizeof(struct stack_frame);

    /* TODO: Need to check stack size */

    stack_frame = (struct stack_frame *)stack_top;

    /* Initialize all registers */
    for (index = 0; index < sizeof(struct stack_frame) / sizeof(os_uint32_t); index++)
    {
        ((os_uint32_t *)stack_frame)[index] = 0xDEADBEEF;
    }

    stack_frame->cpu_frame.r0  = (os_uint32_t)arg;            /* r0 : argument */
    stack_frame->cpu_frame.r1  = 0;                           /* r1 */
    stack_frame->cpu_frame.r2  = 0;                           /* r2 */
    stack_frame->cpu_frame.r3  = 0;                           /* r3 */
    stack_frame->cpu_frame.r12 = 0;                           /* r12 */
    stack_frame->cpu_frame.lr  = (os_uint32_t)task_exit;      /* lr */
    stack_frame->cpu_frame.pc  = (os_uint32_t)task_entry;     /* entry point, pc */
    stack_frame->cpu_frame.psr = 0x01000000UL;                /* PSR */

    /*
     *Do not save floating point to stack. Thread mode + PSP stack. 
     *Once hardware floating is used:CONTROL.FPCA = 1 EXC_RETURN(LR):4bit=0.
     *This behavior is automatically executed by the hardware.
     */
    stack_frame->frame_common.tz = 0x00;
    stack_frame->frame_common.exc_return = 0xFFFFFFBC;
    stack_frame->frame_common.psplim = (os_uint32_t)stack_begin;

    return (void *)stack_top;
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


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
 * @brief       This file provides functions related to the ARM M4 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_task.h>
#include <string.h>
#include <arch_task.h>
#include <arch_interrupt.h>
#include <os_assert.h>
#include <arch_hw.h>

extern os_task_t        *g_os_current_task;
extern os_task_t        *g_os_current1_task;
extern os_task_t        *g_os_next_task;

void *os_hw_stack_init(void        (*task_entry)(void *arg),
        void         *arg,
        void         *stack_begin,
        os_uint32_t   stack_size,
        void        (*task_exit)(void))
{
    os_uint8_t          *stack_top;

    memset(stack_begin, '$', stack_size);

    stack_top  = (os_uint8_t *)stack_begin + stack_size;
    /* Since mips uses an empty stack, point the top of the stack to an available memory address. */
    stack_top -= sizeof(os_size_t);

    extern os_uint8_t *k_os_hw_stack_init(void *tentry, void *parameter, os_uint8_t *stack_addr, void *texit);
    return k_os_hw_stack_init(task_entry, arg, stack_top, task_exit);
}

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

void os_task_switch(void)
{
    os_task_t *from;
    from = g_os_current_task;
    g_os_current_task = g_os_next_task;
    if(os_interrupt_get_nest() == 0)
    {
        from->state &= ~OS_TASK_STATE_RUNNING;
        g_os_next_task->state |= OS_TASK_STATE_RUNNING;
        os_hw_context_switch((os_ubase_t)(&(from->stack_top)), (os_ubase_t)(&(g_os_next_task->stack_top)));
    }
    else
    {
        from->state &= ~OS_TASK_STATE_RUNNING;
        g_os_next_task->state |= OS_TASK_STATE_RUNNING;
        os_hw_context_switch_interrupt((os_ubase_t)(&(from->stack_top)), (os_ubase_t)(&(g_os_next_task->stack_top)));
    }
}

void os_first_task_start(void)
{

    g_os_next_task->state |= OS_TASK_STATE_RUNNING;

    g_os_current_task = g_os_next_task;

    os_hw_context_switch_to((os_ubase_t)(&(g_os_next_task->stack_top)));
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

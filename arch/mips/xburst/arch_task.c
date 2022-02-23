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
#include "../common/mips.h"
#include <arch_task.h>
#include <arch_interrupt.h>
#include <os_assert.h>

/* Stack down */
register U32 $GP __asm__ ("$28");

void *os_hw_stack_init(void        (*task_entry)(void *arg),
                       void         *arg,
                       void         *stack_begin,
                       os_uint32_t   stack_size,
                       void        (*task_exit)(void))
{
    static os_uint32_t wSR=0;
    static os_uint32_t wGP;

    mips_reg_ctx        *regCtx;
    mips_arg_ctx        *argCtx;
    os_uint32_t          i;
    os_uint8_t          *stack_top;

    memset(stack_begin, '$', stack_size);

    stack_top  = (os_uint8_t *)stack_begin + stack_size;
    /* Since mips uses an empty stack, point the top of the stack to an available memory address. */
    stack_top -= sizeof(os_size_t);
    /*Compatible with old interfaces*/
    stack_top  = (os_uint8_t *)OS_ALIGN_DOWN((os_ubase_t)stack_top, OS_ARCH_STACK_ALIGN_SIZE);

    if (wSR == 0)
    {
        wSR = read_c0_status();
        wSR &= 0xfffffffe;
        wSR |= 0x0403;

        wGP = $GP;
    }    

    argCtx = (mips_arg_ctx *)((os_uint32_t)stack_top - sizeof(mips_arg_ctx));
    regCtx = (mips_reg_ctx *)((os_uint32_t)stack_top - sizeof(mips_arg_ctx) - sizeof(mips_reg_ctx));


    for (i = 0; i < 4; ++i)
    {
        argCtx->args[i] = i;
    }

    for (i = 0; i < 32; ++i)
    {
        regCtx->regs[i] = i;
    }

    regCtx->regs[REG_SP] = (os_uint32_t)stack_top;
    regCtx->regs[REG_A0] = (os_uint32_t)arg;
    regCtx->regs[REG_GP] = (os_uint32_t)wGP;
    regCtx->regs[REG_FP] = (os_uint32_t)0x0;
    regCtx->regs[REG_RA] = (os_uint32_t)task_exit;

    regCtx->CP0DataLO   = 0x00;
    regCtx->CP0DataHI   = 0x00;
    regCtx->CP0Cause    = read_c0_cause();
    regCtx->CP0Status   = wSR;
    regCtx->CP0EPC      = (os_uint32_t)task_entry;
    regCtx->CP0BadVAddr= 0x00;

    return (os_uint8_t *)(regCtx);
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
    if(os_interrupt_get_nest() == 0)
    {
        extern void os_hw_context_switch_from_task(void);
        os_hw_context_switch_from_task();
    }
    else
    {
        extern os_uint32_t g_os_task_switch_interrupt_flag;
        g_os_task_switch_interrupt_flag = 1;
    }
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


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
 * @file        stack.c
 *
 * @brief       stack init
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_stddef.h>
#include "mips.h"
#include <arch_task.h>

register os_uint32_t $GP __asm__ ("$28");

os_uint8_t *k_os_hw_stack_init(void *tentry, void *parameter, os_uint8_t *stack_addr, void *texit)
{
    static os_ubase_t wSR=0;
    static os_ubase_t wGP;
    os_uint8_t *stk;

    struct pt_regs *pt;

    os_uint32_t i;

    /* Get stack aligned */
    stk = (os_uint8_t *)OS_ALIGN_DOWN((os_ubase_t)stack_addr, OS_ARCH_STACK_ALIGN_SIZE);
    stk -= sizeof(struct pt_regs);
    pt =  (struct pt_regs*)stk;

#ifndef ARCH_MIPS64
    for (i = 0; i < 8; ++i)
    {
        pt->pad0[i] = 0xdeadbeef;
    }
#endif
    /* Fill Stack register numbers */
    for (i = 0; i < 32; ++i)
    {
        pt->regs[i] = 0xdeadbeef;
    }

    pt->regs[REG_SP] = (os_ubase_t)stk;
    pt->regs[REG_A0] = (os_ubase_t)parameter;
    pt->regs[REG_GP] = (os_ubase_t)$GP;
    pt->regs[REG_FP] = (os_ubase_t)0x0;
    pt->regs[REG_RA] = (os_ubase_t)texit;

    pt->hi  = 0x0;
    pt->lo  = 0x0;
    pt->cp0_status = (ST0_IE | ST0_CU0 | ST0_IM);
#ifdef OS_USING_FPU
    pt->cp0_status |= (ST0_CU1 | ST0_FR);
#endif
    pt->cp0_cause   = read_c0_cause();
    pt->cp0_epc = (os_ubase_t)tentry;
    pt->cp0_badvaddr    = 0x0;

    return stk;
}

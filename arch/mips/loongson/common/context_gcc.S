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
 * @file        contex_gcc.s
 *
 * @brief       context switch asm file
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif

#include "stackframe.h"

    .section ".text", "ax"
    .set noreorder

    .global uart_send_char_debug

/*
 * void os_hw_context_switch(os_uint32 from, os_uint32 to)
 * a0 --> from
 * a1 --> to
 */
    .globl os_hw_context_switch
os_hw_context_switch:
    MTC0    ra, CP0_EPC
    SAVE_ALL

    REG_S      sp, 0(a0)       /* store sp in preempted tasks TCB */
    REG_L      sp, 0(a1)       /* get new task stack pointer */

    RESTORE_ALL_AND_RET

/*
 * void os_hw_context_switch_to(os_uint32 to)/*
 * a0 --> to
 */
    .globl os_hw_context_switch_to
os_hw_context_switch_to:

    REG_L      sp, 0(a0)       /* get new task stack pointer */
    RESTORE_ALL_AND_RET

/*
 * void os_hw_context_switch_interrupt(os_uint32 from, os_uint32 to)/*
 */
    .globl os_thread_switch_interrupt_flag
    .globl os_interrupt_from_thread
    .globl os_interrupt_to_thread
    .globl os_hw_context_switch_interrupt
os_hw_context_switch_interrupt:
    PTR_LA      t0, os_thread_switch_interrupt_flag
    REG_L       t1, 0(t0)
    nop
    bnez    t1, _reswitch
    nop
    li      t1, 0x01                       /* set os_thread_switch_interrupt_flag to 1 */
    LONG_S      t1, 0(t0)
    PTR_LA      t0, os_interrupt_from_thread   /* set os_interrupt_from_thread */
    LONG_S      a0, 0(t0)
_reswitch:
    PTR_LA      t0, os_interrupt_to_thread     /* set os_interrupt_to_thread */
    LONG_S      a1, 0(t0)
    jr      ra
    nop

/*
 * void os_hw_context_switch_interrupt_do(os_base_t flag)
 */
    .globl os_interrupt_enter
    .globl os_interrupt_leave
    .globl os_general_exc_dispatch
    .globl mips_irq_handle
mips_irq_handle:
    SAVE_ALL

    /* let k0 keep the current context sp */
    move    k0, sp
    /* switch to kernel stack */
    lw      sp, interrupt_stack_addr

    jal     os_interrupt_enter
    nop
    /* Get Old SP from k0 as paremeter in a0 */
    move	a0, k0
    jal     os_general_exc_dispatch
    nop
    jal     os_interrupt_leave
    nop

    /* switch sp back to thread context */
    move    sp, k0

    /*
    * if os_thread_switch_interrupt_flag set, jump to
    * os_hw_context_switch_interrupt_do and do not return
    */
    PTR_LA  k0, os_thread_switch_interrupt_flag
    LONG_L  k1, 0(k0)
    beqz    k1, spurious_interrupt
    nop
    LONG_S  zero, 0(k0)                     /* clear flag */
    nop

    /*
    * switch to the new thread
    */
    PTR_LA  k0, os_interrupt_from_thread
    LONG_L  k1, 0(k0)
    nop
    LONG_S  sp, 0(k1)                       /* store sp in preempted task TCB */

    PTR_LA  k0, os_interrupt_to_thread
    LONG_L  k1, 0(k0)
    nop
    LONG_L  sp, 0(k1)                       /* get new task stack pointer */
    j       spurious_interrupt
    nop

spurious_interrupt:
    RESTORE_ALL_AND_RET
    .set reorder


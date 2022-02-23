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
 * @file        interrupt_gcc.S
 *
 * @brief       This file provides context interrupt functions related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "riscv_encoding.h"
#include "riscv_bits.h"

.equ    MSUBM_PTYP,    (0x3 << 8)
.equ    OS_TASK_STATE_RUNNING,   0x0004              /* Task Status Flag (RUNNING) */

.extern g_os_current_task
.extern g_os_next_task
.extern os_task_switch_interrupt_flag
.extern os_task_switch_notify

/* save necessary registers */
.macro IRQ_SAVE_CONTEXT
    addi  sp, sp, -32 * REGBYTES
    STORE x1,   2 * REGBYTES(sp)

    STORE x4,   4 * REGBYTES(sp)
    STORE x5,   5 * REGBYTES(sp)
    STORE x6,   6 * REGBYTES(sp)
    STORE x7,   7 * REGBYTES(sp)
    STORE x10, 10 * REGBYTES(sp)
    STORE x11, 11 * REGBYTES(sp)
    STORE x12, 12 * REGBYTES(sp)
    STORE x13, 13 * REGBYTES(sp)
    STORE x14, 14 * REGBYTES(sp)
    STORE x15, 15 * REGBYTES(sp)
    STORE x16, 16 * REGBYTES(sp)
    STORE x17, 17 * REGBYTES(sp)
    STORE x28, 28 * REGBYTES(sp)
    STORE x29, 29 * REGBYTES(sp)
    STORE x30, 30 * REGBYTES(sp)
    STORE x31, 31 * REGBYTES(sp)
.endm

/* save preserved registers */
.macro IRQ_SWITCH_SAVE_REG    
    STORE x8,   8 * REGBYTES(t1)
    STORE x9,   9 * REGBYTES(t1)
    STORE x18, 18 * REGBYTES(t1)
    STORE x19, 19 * REGBYTES(t1)
    STORE x20, 20 * REGBYTES(t1)
    STORE x21, 21 * REGBYTES(t1)
    STORE x22, 22 * REGBYTES(t1)
    STORE x23, 23 * REGBYTES(t1)
    STORE x24, 24 * REGBYTES(t1)
    STORE x25, 25 * REGBYTES(t1)
    STORE x26, 26 * REGBYTES(t1)
    STORE x27, 27 * REGBYTES(t1)
.endm

/* restore necessary registers */
.macro IRQ_RESTORE_CONTEXT
    LOAD  x1,   2 * REGBYTES(sp)

    LOAD  x4,   4 * REGBYTES(sp)
    LOAD  x5,   5 * REGBYTES(sp)
    LOAD  x6,   6 * REGBYTES(sp)
    LOAD  x7,   7 * REGBYTES(sp)
    LOAD  x10, 10 * REGBYTES(sp)
    LOAD  x11, 11 * REGBYTES(sp)
    LOAD  x12, 12 * REGBYTES(sp)
    LOAD  x13, 13 * REGBYTES(sp)
    LOAD  x14, 14 * REGBYTES(sp)
    LOAD  x15, 15 * REGBYTES(sp)
    LOAD  x16, 16 * REGBYTES(sp)
    LOAD  x17, 17 * REGBYTES(sp)
    LOAD  x28, 28 * REGBYTES(sp)
    LOAD  x29, 29 * REGBYTES(sp)
    LOAD  x30, 30 * REGBYTES(sp)
    LOAD  x31, 31 * REGBYTES(sp)
    addi  sp, sp, 32 * REGBYTES
.endm

/* restore preserved registers */
.macro IRQ_SWITCH_RESTORE_REG
    LOAD x8,   8 * REGBYTES(t2)
    LOAD x9,   9 * REGBYTES(t2)
    LOAD x18, 18 * REGBYTES(t2)
    LOAD x19, 19 * REGBYTES(t2)
    LOAD x20, 20 * REGBYTES(t2)
    LOAD x21, 21 * REGBYTES(t2)
    LOAD x22, 22 * REGBYTES(t2)
    LOAD x23, 23 * REGBYTES(t2)
    LOAD x24, 24 * REGBYTES(t2)
    LOAD x25, 25 * REGBYTES(t2)
    LOAD x26, 26 * REGBYTES(t2)
    LOAD x27, 27 * REGBYTES(t2)
.endm

/* save csr registers */
.macro IRQ_SAVE_CSR
    addi  sp,  sp, -3 * REGBYTES
    csrrwi  x0, CSR_PUSHMCAUSE,0
    csrrwi  x0, CSR_PUSHMEPC,  1
    csrrwi  x0, CSR_PUSHMSUBM, 2
.endm

/* restore csr registers */
.macro IRQ_RESTORE_CSR
    LOAD  t0,  0 * REGBYTES(sp)
    csrw  CSR_MCAUSE, t0
    LOAD  t0,  1 * REGBYTES(sp)
    csrw  CSR_MEPC, t0
    LOAD  t0,  2 * REGBYTES(sp)
    csrw  CSR_MSUBM, t0
    addi  sp,  sp, 3 * REGBYTES
.endm

/* switch task sp to irq sp */
.macro TASK_SP_TO_IRQ_SP
    /* save mstatus,mepc,sp */
    csrr  t0,  CSR_MSTATUS
    STORE t0,  0 * REGBYTES(sp)

    csrr  t0,  CSR_MEPC
    STORE t0,  1 * REGBYTES(sp)

    la    t0,  g_os_current_task                /* t0 = &g_os_current_task */
    LOAD  t0,  0(t0)                            /* t0 = g_os_current_task */
    STORE sp,  0(t0)                            /* g_os_current_task->stack_top = sp */

    /* task stack to irq stack. */
    la    sp,  _sp
.endm

.macro IRQ_SP_TO_TASK_SP
    /* restore mstatus,mepc,sp */
    la    t0,  g_os_current_task                /* t0 = &g_os_current_task */
    LOAD  t0,  0(t0)                            /* t0 = g_os_current_task */
    LOAD  sp,  0(t0)                            /* sp = g_os_next_task->stack_top; Pickup task stack pointer */

    LOAD  t0,  0 * REGBYTES(sp)                 /* t0 = CSR_MSTATUS */
    li    t1,  MSTATUS_MPP                      /* set the privilege as machine mode. */
    or    t0,  t0, t1
    csrw  CSR_MSTATUS, t0

    LOAD  t0,  1 * REGBYTES(sp)
    csrw  CSR_MEPC, t0
.endm

.section      .text.entry
.align 6

/* irq entry */
.global irq_entry
irq_entry:

    IRQ_SAVE_CONTEXT
    
    /* check if the nesting is interrupted */
    /* irq nesting ==> save special CSR registers,do not need switch sp */
    /* non-irq nesting ==> save mstatus,switch to irq sp */
    csrr  a0, CSR_MSUBM
    andi  a0, a0, 0x00000300
    bnez  a0, irq_save_csr

switch_irq_sp:
    TASK_SP_TO_IRQ_SP

irq_save_csr:
    IRQ_SAVE_CSR

irq_process:
    /* irq loop */
    csrrw ra, CSR_JALMNXTI, ra

    /* disable irq */
    csrci CSR_MSTATUS, MSTATUS_MIE

irq_restore_csr:
    IRQ_RESTORE_CSR

scheduler:
    /* check if the nesting is irq */
    /* irq nesting ==> restore context,do not need switch sp */
    /* non-irq nesting ==> restore mstatus,switch to task sp */
    csrr  a0, CSR_MSUBM
    andi  a0, a0, MSUBM_PTYP
    bnez  a0, restore_context

    /* Check if need to switch task context */
    la    t0, os_task_switch_interrupt_flag
    lw    t1, 0(t0)
    beqz  t1, switch_task_sp                    /* if flag = 0, don`t need task switch, return the current task. */
    sw    zero, 0(t0)                           /* clear flag. */
    
    /* get current task struct pointer*/    
    la    t0, g_os_current_task                 /* t0 = &g_os_current_task */
    LOAD  t2, 0(t0)                             /* t2 = g_os_current_task */

    /* save preserved registers */
    LOAD  t1, 0(t2)                             /* t1 = g_os_current_task->stack_top */
    IRQ_SWITCH_SAVE_REG

    /* clear current task running state */
    li    a0, OS_TASK_STATE_RUNNING
    not   a0, a0                                /* a0 = (~OS_TASK_STATE_RUNNING) */
    LOAD  t1, 3 * REGBYTES(t2)                  /* t1 = g_os_current_task->state */
    and   t1, t1, a0                            /* t1 &= (~OS_TASK_STATE_RUNNING) */
    STORE t1, 3 * REGBYTES(t2)                  /* g_os_current_task->state = t1 */

    /* Check either task stack during task switching. */
    #if defined (OS_TASK_SWITCH_NOTIFY)
    jal os_task_switch_notify
    #endif

    /* switch task: g_os_current_task = g_os_next_task */ 
    la    t1, g_os_next_task                    /* t1 = &g_os_next_task */
    LOAD  t1, 0(t1)                             /* t1 = g_os_next_task */
    la    t0, g_os_current_task                 /* t0 = &g_os_current_task */
    STORE t1, 0(t0)                             /* (*t0) = t1 = g_os_next_task */

    /* Set switch to task running state */
    LOAD  t0, 3 * REGBYTES(t1)                  /* t0 = g_os_next_task->state */
    ori   t0, t0, OS_TASK_STATE_RUNNING         /* t0 |= (OS_TASK_STATE_RUNNING) */
    STORE t0, 3 * REGBYTES(t1)                  /* g_os_next_task->state = t0 */

    /* restore preserved registers */
    LOAD  t2, 0(t1)                             /* t2 = g_os_next_task->stack_top */
    IRQ_SWITCH_RESTORE_REG

switch_task_sp:
    IRQ_SP_TO_TASK_SP

restore_context:
    IRQ_RESTORE_CONTEXT
    mret

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
 * @file        context_gcc.S
 *
 * @brief       This file provides context switch functions related to the RISC-V architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-05-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "riscv_encoding.h"
#include "riscv_bits.h"

.equ    OS_TASK_STATE_RUNNING,   0x0004              /* Task Status Flag (RUNNING) */

.extern g_os_current_task
.extern g_os_next_task
.extern os_task_switch_interrupt_flag
.extern os_task_switch_notify

.macro SWITCH_TO_TASK_PROCESS
    /* get switch to task struct */    
    la    t1, g_os_next_task                    /* t1 = &g_os_next_task */
    LOAD  t1, 0(t1)                             /* t1 = g_os_next_task */
    /* g_os_current_task = g_os_next_task */
    la    t0, g_os_current_task                 /* t0 = &g_os_current_task */
    STORE t1, 0(t0)                             /* (*t0) = t1 = g_os_next_task */

    /* begin: Restore next task context */
    LOAD  sp, 0(t1)                             /* sp = g_os_next_task->stack_top; Pickup task stack pointer */

    /* Set switch to task running state */
    LOAD  t0, 3 * REGBYTES(t1)                  /* t0 = g_os_current_task->state */
    ori   t0, t0, OS_TASK_STATE_RUNNING         /* t0 |= (OS_TASK_STATE_RUNNING) */
    STORE t0, 3 * REGBYTES(t1)                  /* g_os_current_task->state = t0 */

    /* set mpp, restore mstatus */
    LOAD  t0,  0 * REGBYTES(sp)
    li    t2,  MSTATUS_MPP                      /* set the privilege as machine mode. */
    or    t0,  t0, t2
    csrw  CSR_MSTATUS, t0

    /* load mepc */
    LOAD  t0,  1 * REGBYTES(sp)
    csrw  CSR_MEPC, t0
.endm

/**
 ***********************************************************************************************************************
 * @brief           This function is called when the scheduler starts the first task, Only used once.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
.globl os_first_task_start
os_first_task_start:
    SWITCH_TO_TASK_PROCESS

    LOAD  x1,   2 * REGBYTES(sp)                /* load ra */
    LOAD  x10, 10 * REGBYTES(sp)                /* load a0 */
    
    addi  sp,  sp, 32 * REGBYTES
    /* end: Restore next task stack */

    mret

 /**
 ***********************************************************************************************************************
 * @brief           Start the task swtich process.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
.globl os_task_switch
os_task_switch:
     /* begin: save current task context */
    addi  sp,  sp, -32 * REGBYTES
    /* store T0 T1 */
    STORE x5,   5 * REGBYTES(sp)
    STORE x6,   6 * REGBYTES(sp)

    /* Check if into irq context */
    addi t1, x0, 0x00000040
    csrr t0, 0x7c4
    andi t0, t0, 0x000000c0
    beq  t0, t1, irq_context_switch

    /* store mstatus */
    STORE x1,   1 * REGBYTES(sp)
    STORE x1,   2 * REGBYTES(sp)
    csrr  t0,   CSR_MSTATUS

store_from_task:
    STORE t0,   0 * REGBYTES(sp)
    STORE x4,   4 * REGBYTES(sp)
    STORE x7,   7 * REGBYTES(sp)
    STORE x8,   8 * REGBYTES(sp)
    STORE x9,   9 * REGBYTES(sp)
    STORE x10, 10 * REGBYTES(sp)
    STORE x11, 11 * REGBYTES(sp)
    STORE x12, 12 * REGBYTES(sp)
    STORE x13, 13 * REGBYTES(sp)
    STORE x14, 14 * REGBYTES(sp)
    STORE x15, 15 * REGBYTES(sp)
    STORE x16, 16 * REGBYTES(sp)
    STORE x17, 17 * REGBYTES(sp)
    STORE x18, 18 * REGBYTES(sp)
    STORE x19, 19 * REGBYTES(sp)
    STORE x20, 20 * REGBYTES(sp)
    STORE x21, 21 * REGBYTES(sp)
    STORE x22, 22 * REGBYTES(sp)
    STORE x23, 23 * REGBYTES(sp)
    STORE x24, 24 * REGBYTES(sp)
    STORE x25, 25 * REGBYTES(sp)
    STORE x26, 26 * REGBYTES(sp)
    STORE x27, 27 * REGBYTES(sp)
    STORE x28, 28 * REGBYTES(sp)
    STORE x29, 29 * REGBYTES(sp)
    STORE x30, 30 * REGBYTES(sp)
    STORE x31, 31 * REGBYTES(sp)

    /* get current task struct */
    la    t0, g_os_current_task                 /* t0 = &g_os_current_task */
    LOAD  t0, 0(t0)                             /* t0 = g_os_current_task */

    /* store current task sp */
    STORE sp, 0(t0)                             /* g_os_current_task->stack_top = sp */
    /* end: save current task mstatus and registers to stack */

    /* clear current task running state */
    li    t1, OS_TASK_STATE_RUNNING
    not   t1, t1                                /* ~OS_TASK_STATE_RUNNING */
    LOAD  t2, 3 * REGBYTES(t0)                  /* t2 = g_os_current_task->state */
    and   t2, t2, t1                            /* t2 &= (~OS_TASK_STATE_RUNNING) */
    STORE t2, 3 * REGBYTES(t0)                  /* g_os_current_task->state = t2 */
    
    /* Check either task stack during task switching. */
    #if defined (OS_TASK_SWITCH_NOTIFY)
    jal os_task_switch_notify
    #endif

switch_to_task:
    SWITCH_TO_TASK_PROCESS

restore_contex:
    LOAD x1,   2 * REGBYTES(sp)

    LOAD x4,   4 * REGBYTES(sp)
    LOAD x5,   5 * REGBYTES(sp)
    LOAD x6,   6 * REGBYTES(sp)
    LOAD x7,   7 * REGBYTES(sp)
    LOAD x8,   8 * REGBYTES(sp)
    LOAD x9,   9 * REGBYTES(sp)
    LOAD x10, 10 * REGBYTES(sp)
    LOAD x11, 11 * REGBYTES(sp)
    LOAD x12, 12 * REGBYTES(sp)
    LOAD x13, 13 * REGBYTES(sp)
    LOAD x14, 14 * REGBYTES(sp)
    LOAD x15, 15 * REGBYTES(sp)
    LOAD x16, 16 * REGBYTES(sp)
    LOAD x17, 17 * REGBYTES(sp)
    LOAD x18, 18 * REGBYTES(sp)
    LOAD x19, 19 * REGBYTES(sp)
    LOAD x20, 20 * REGBYTES(sp)
    LOAD x21, 21 * REGBYTES(sp)
    LOAD x22, 22 * REGBYTES(sp)
    LOAD x23, 23 * REGBYTES(sp)
    LOAD x24, 24 * REGBYTES(sp)
    LOAD x25, 25 * REGBYTES(sp)
    LOAD x26, 26 * REGBYTES(sp)
    LOAD x27, 27 * REGBYTES(sp)
    LOAD x28, 28 * REGBYTES(sp)
    LOAD x29, 29 * REGBYTES(sp)
    LOAD x30, 30 * REGBYTES(sp)
    LOAD x31, 31 * REGBYTES(sp)

    addi sp,  sp, 32 * REGBYTES
    /* end: Restore next task stack */

    mret

irq_context_switch:
    /* if switch task in irq context, only set switch_interrupt_flag then return */
    addi  t1, x0, 1
    la    t0, os_task_switch_interrupt_flag
    sw    t1, 0(t0)

    /* restore T0 T1 */
    LOAD x5,   5 * REGBYTES(sp)
    LOAD x6,   6 * REGBYTES(sp)
    addi sp,  sp, 32 * REGBYTES

    ret


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
 * @file        mips_context_gcc.S
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif

#include "../common/mips.h"

.section .text,"ax",@progbits
.set noreorder
.set noat

.equ OS_TASK_STATE_RUNNING,        0x0004               /* Task Status Flag (RUNNING) */
.equ OS_TASK_STATE_RUNNING_CLR,    -5                   /* Task Status Flag (RUNNING) Clear */
.equ OS_TASK_STATE_OFFSET,         12                   /* Byte offset of task status in os_task_t */

.global g_os_current_task
.global g_os_next_task
.global os_task_switch_notify

LEAF(os_irq_lock)
    mfc0    v0, CP0_STATUS                  /* v0 = status */
    addiu   v1, zero,   -2                  /* v1 = 0-2 = 0xFFFFFFFE */
    and     v1, v0, v1                      /* v1 = v0 & 0xFFFFFFFE */
    mtc0    v1, CP0_STATUS                  /* status = v1 */
    andi    v0, v0, 1                       /* v0 = v0 & 1 */
    jr      ra
    nop
END(os_irq_lock)

LEAF(os_irq_unlock)
    mfc0    v0,CP0_STATUS                   /* v0 = CP0_STATUS */
    addiu   v1, zero,   -2                  /* v1 = 0-2 = 0xFFFFFFFE */
    and     v0, v0, v1                      /* v0 = v0 & 0xFFFFFFFE */
    or      v0, v0, a0                      /* v0 = v0 | a0 */
    mtc0    v0, CP0_STATUS                  /* CP0_STATUS = v0 */
    jr      ra
    nop
END(os_irq_unlock)


LEAF(os_irq_disable)
    mfc0    v0, CP0_STATUS                  /* v0 = status */
    addiu   v1, zero,   -2                  /* v1 = 0-2 = 0xFFFFFFFE */
    and     v1, v0, v1                      /* v1 = v0 & 0xFFFFFFFE */
    mtc0    v1, CP0_STATUS                  /* status = v1 */
    jr      ra
    nop
END(os_irq_disable)

LEAF(os_irq_enable)
    mfc0    v0, CP0_STATUS                  /* v0 = status */
    ori     v0, v0, 1                       /* v0 = v0 | 0x01 */
    mtc0    v0, CP0_STATUS                  /* CP0_STATUS = v0 */
    jr      ra
    nop
END(os_irq_enable)

LEAF(os_is_irq_disabled)
    mfc0    v1, CP0_STATUS                  /* v1 = CP0_STATUS */
    andi    v0, v1, 1                       /* v0 = v1 & 0x01 */
    xori    v0, v0, 1                       /* v0 = v0 ^ 0x01 */
    jr      ra
    nop
END(os_is_irq_disabled)


/*
 * void os_first_task_start(void)
 */
LEAF(os_first_task_start)
    lw  t0 , g_os_next_task                 /* t0 = g_os_next_task */
    la  t1 , g_os_current_task              /* t1 = &g_os_current_task */
    sw  t0 , 0(t1)                          /* g_os_current_task = g_os_next_task */

    /* Set switch to task running state */
    lw  t1 , 0(t1)                          /* t1 = g_os_current_task */
    lw  t2 , (OS_TASK_STATE_OFFSET)(t1)     /* t2 = g_os_current_task->state */
    ori t2 , t2, OS_TASK_STATE_RUNNING      /* t2 |= (OS_TASK_STATE_RUNNING) */
    sw  t2 , (OS_TASK_STATE_OFFSET)(t1)     /* g_os_current_task->state = t2 */

    lw  sp ,   0(t0)                        /* switch to the new stack */
    RESTORE_CONTEXT
END(os_first_task_start)

/*
 * void os_hw_context_switch(void)
 */
LEAF(os_hw_context_switch_from_task)
    mtc0    ra, CP0_EPC
    SAVE_CONTEXT
    lw  t0 , g_os_current_task              /* t1 = g_os_current_task */
    sw  sp , 0(t0)                          /* store sp in preempted tasks TCB */

    li  t1 , OS_TASK_STATE_RUNNING_CLR      /* t1 = OS_TASK_STATE_RUNNING_CLR */
    lw  t2 , (OS_TASK_STATE_OFFSET)(t0)     /* t2 = g_os_current_task->state */
    and t2 , t2, t1                         /* t2 &= OS_TASK_STATE_RUNNING_CLR */
    sw  t2 , (OS_TASK_STATE_OFFSET)(t0)     /* g_os_current_task->state = t2 */

    /*task_switch_hook*/
    #if defined (OS_TASK_SWITCH_NOTIFY)
    subu    sp, sp, 4                       
    sw      ra, (0)(sp)                     /* push ra*/

    jal os_task_switch_notify
    nop

    lw      ra, (0)(sp)                     /* pop ra*/
    addu    sp, sp, 4
    #endif
    
    lw  t0 , g_os_next_task                 /* t0 = g_os_next_task */
    la  t1 , g_os_current_task              /* t1 = &g_os_current_task */
    sw  t0 , 0(t1)                          /* g_os_current_task = g_os_next_task */

    lw  t2 , 0(t1)                          /* t2 = g_os_current_task */
    lw  t3 , (OS_TASK_STATE_OFFSET)(t2)     /* t3 = g_os_current_task->state */
    ori t3 , t3, OS_TASK_STATE_RUNNING      /* t3 |= (OS_TASK_STATE_RUNNING) */
    sw  t3 , (OS_TASK_STATE_OFFSET)(t2)     /* g_os_current_task->state = t3 */

    lw  sp , 0(t2)                          /* get new task stack pointer */

    RESTORE_CONTEXT
END(os_hw_context_switch_from_task)


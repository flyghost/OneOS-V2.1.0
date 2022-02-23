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
 * @file        arch_task_switch_gcc.S
 *
 * @brief       This file provides context switch functions related to the ARMv8-M architecture, 
 *              and the compiler uses gcc.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
.syntax unified
.thumb
.text

.equ SCB_VTOR,                  0xE000ED08              /* Vector table offset register */
.equ NVIC_INT_CTRL,             0xE000ED04              /* Interrupt control state register */
.equ NVIC_SYSPRI2,              0xE000ED20              /* System priority register (2) */
.equ NVIC_PENDSV_PRI,           0x00FF0000              /* PendSV priority value (lowest) */
.equ NVIC_SYSTICK_PRI,          0xFF000000              /* SysTick priority value (lowest) */
.equ NVIC_PENDSV_SYSTICK_PRI,   0xFFFF0000              /* PendSV and SysTick priority value (lowest) */	
.equ NVIC_PENDSVSET,            0x10000000              /* Value to trigger PendSV exception */

.equ OS_TASK_STATE_RUNNING,     0x00000004              /* Task Status Flag (RUNNING) */

.extern g_os_current_task
.extern g_os_next_task
.extern interrupt_stack_addr
.extern os_task_switch_notify

.global os_task_switch
.type   os_task_switch, %function
.global os_first_task_start
.type   os_first_task_start, %function
.global PendSV_Handler	                              /* Override the  definition of PendSV_Handler */
.type   PendSV_Handler, %function

os_task_switch:
    /* Trigger pdneSV */
    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    BX      LR

PendSV_Handler: 
    /* Disable interrupt to prevent interruption of task switching process */
    MRS     R12, PRIMASK
    CPSID   I

    /* If g_os_current_task is OS_NULL, skip preservation */
    LDR     R1,  =g_os_current_task                  /* R1 = &gs_os_current_task */
    LDR     R0,  [R1]                                /* R0 = gs_os_current_task */         
    CBZ     R0,  switch_to_task                      /* If gs_os_current_task is OS_NULL, os_first_task_start trigger */

    /* begin: save current task registers to stack */
    MRS     R3,  PSP

    /* Save FPU registers */
    #if (defined (__VFP_FP__) && !defined(__SOFTFP__))
        TST      LR,  #0x10
        IT       EQ
        VSTMDBEQ R3!, {D8 - D15}                    /*Lazy Stacking:Triggers the hardware floating point (D0-D8,FPSCR) update stack at the same time*/
    #endif

    MRS     R2,  PSPLIM                             /* R2 = PSPLIM */
    STMFD   R3!, {R2}                               /* Push PSPLIM */
    STMFD   R3!, {R4 - R11, LR}                     /* Save R4 - R11 and LR*/
    STR     R3,  [R0, #0]                           /* Update g_os_current_task->stack_top */
    /* end: save current task registers to stack */

    /* Clear current task running state */
    LDR     R2,  =OS_TASK_STATE_RUNNING
    LDRH    R3,  [R0, #12]                          /* R4 = g_os_current_task->state */
    BIC     R3,  R3, R2                             /* g_os_current_task->state &= (~OS_TASK_STATE_RUNNING) */
    STRH    R3,  [R0, #12]

    /* Check either task stack during task switching. */
    #if defined (OS_TASK_SWITCH_NOTIFY)
    PUSH    {R1, R12}
    BL      os_task_switch_notify
    POP     {R1, R12}
    #endif

switch_to_task:
    /* Get g_os_next_task->stack_top */
    LDR     R3, =g_os_next_task                     /* R3 = &g_os_next_task */
    LDR     R2,  [R3]                               /* R2 = g_os_next_task */

    /* begin: Restore next task stack */
    LDR     R3,  [R2, #0]                           /* R3 = g_os_next_task->stack_top; Pickup task stack pointer */
    LDMFD   R3!, {R4 - R11, LR}                     /* Pop R4 - R11 and LR from stack */
    LDMFD   R3!, {R0}                               /* Pop PSPLIM */
    MSR     PSPLIM, R0                              /* PSPLIM = R0*/
    
    #if (defined (__VFP_FP__) && !defined(__SOFTFP__))
        TST      LR,  #0x10
        IT       EQ
        VLDMIAEQ R3!, {D8 - D15}
    #endif  

    MSR     PSP, R3                                 /* Restore PSP */
    /* end: Restore next task stack */

    /* g_os_current_task = g_os_next_task */
    STR     R2,  [R1]

    /* Set task running state */
    LDR     R0,  =OS_TASK_STATE_RUNNING
    LDRH    R3,  [R2, #12]                          /* R3 = g_os_next_task->state */
    ORR     R3,  R3, R0                             /* g_os_next_task->state |= OS_TASK_STATE_RUNNING */
    STRH    R3,  [R2, #12]

    /* Restore interrupt */
    MSR     PRIMASK, R12

    BX      LR

os_first_task_start:
    /* set the PendSV and Systick priority. */
    LDR     R0, =NVIC_SYSPRI2
    LDR     R1, =NVIC_PENDSV_SYSTICK_PRI
    LDR.W   R2, [R0, #0x00]                         /* Read */
    ORR     R1, R1, R2                              /* Modify */
    STR     R1, [R0]                                /* Write-back */

    #if (defined (__VFP_FP__) && !defined(__SOFTFP__))
        /* Clear CONTROL.FPCA */
        MRS     R2, CONTROL             /* Read */
        BIC     R2, #0x04               /* Modify */
        MSR     CONTROL, R2             /* Write-back */
    #endif

    /* Restore MSP */
    LDR     R0, =interrupt_stack_addr
    LDR     R0, [R0]
    MSR     MSP, R0

    /* Trigger the PendSV exception (causes context switch) */
    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    /* Enable interrupts at processor level */
    CPSIE   F
    CPSIE   I

    ; /* Never reach here! */
    B .


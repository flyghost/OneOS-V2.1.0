;/**
 ;***********************************************************************************************************************
 ;* Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 ;*
 ;* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 ;* the License. You may obtain a copy of the License at
 ;*
 ;*     http://www.apache.org/licenses/LICENSE-2.0
 ;*
 ;* Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 ;* an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 ;* specific language governing permissions and limitations under the License.
 ;*
 ;* @file        os_arch_task_switch_rvds.S
 ;*
 ;* @brief       This file provides context switch functions related to the ARMv7-M architecture, 
 ;*              and the compiler uses armcc.
 ;*
 ;* @revision
 ;* Date         Author          Notes
 ;* 2020-10-10   OneOS Team      First version.
 ;***********************************************************************************************************************
 ;*/

SCB_VTOR                EQU     0xE000ED08              ; /* Vector table offset register */
NVIC_INT_CTRL           EQU     0xE000ED04              ; /* Interrupt control state register */
NVIC_SYSPRI2            EQU     0xE000ED20              ; /* System priority register (2) */
NVIC_PENDSV_PRI         EQU     0x00FF0000              ; /* PendSV priority value (lowest) */
NVIC_SYSTICK_PRI        EQU     0xFF000000              ; /* SysTick priority value (lowest) */
NVIC_PENDSVSET          EQU     0x10000000              ; /* Value to trigger PendSV exception */

OS_TASK_STATE_RUNNING   EQU     0x0004                  ; /* Task Status Flag (RUNNING) */

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8
    IMPORT g_os_current_task
    IMPORT g_os_next_task
    IMPORT interrupt_stack_addr
    IF :DEF:OS_TASK_SWITCH_NOTIFY
    IMPORT os_task_switch_notify
    ENDIF

    EXPORT os_task_switch
    EXPORT os_first_task_start
    EXPORT PendSV_Handler                               ; /* Override the MDK definition of PendSV_Handler */

os_task_switch   PROC  
    ; /* Trigger pdneSV */
    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    BX      LR
    ENDP

PendSV_Handler   PROC
    ; /* Disable interrupt to prevent interruption of task switching process */
    MRS     R12, PRIMASK
    CPSID   I

    ; /* If g_os_current_task is OS_NULL, skip preservation */
    LDR     R1,  =g_os_current_task                 ; /* R1 = &gs_os_current_task */
    LDR     R0,  [R1]                               ; /* R0 = gs_os_current_task */
    CBZ     R0,  switch_to_task                     ; /* If gs_os_current_task is OS_NULL, os_first_task_start trigger */

    ; /* begin: save current task registers to stack */
    MRS     R3,  PSP

    ; /* Save FPU registers */
    IF {FPU} != "SoftVFP"
        TST      LR,  #0x10
        VSTMFDEQ R3!, {D8 - D15}                    ;/*Lazy Stacking:Triggers the hardware floating point (D0-D8,FPSCR) update stack at the same time*/
    ENDIF

    STMFD   R3!, {R4 - R11, LR}                     ; /* Save R4 - R11 and LR*/
    STR     R3,  [R0, #0]                           ; /* Update g_os_current_task->stack_top */
    ; /* end: save current task registers to stack */

    ; /* Clear current task running state */
    LDR     R2,  =OS_TASK_STATE_RUNNING
    LDRH    R3,  [R0, #12]                          ; /* R4 = g_os_current_task->state */
    BIC     R3,  R3, R2                             ; /* g_os_current_task->state &= (~OS_TASK_STATE_RUNNING) */
    STRH    R3,  [R0, #12]

    ; /* Check either task stack during task switching. */
    IF :DEF:OS_TASK_SWITCH_NOTIFY
    PUSH    {R1, R12}
    BL      os_task_switch_notify
    POP     {R1, R12}
    ENDIF

switch_to_task
    ; /* Get g_os_next_task->stack_top */
    LDR     R3, =g_os_next_task                     ; /* R3 = &g_os_next_task */
    LDR     R2,  [R3]                               ; /* R2 = g_os_next_task */

    ; /* begin: Restore next task stack */
    LDR     R3,  [R2, #0]                           ; /* R3 = g_os_next_task->stack_top; Pickup task stack pointer */
    LDMFD   R3!, {R4 - R11, LR}                     ; /* Pop R4 - R11 and LR from stack */

    IF {FPU} != "SoftVFP"
        TST      LR,  #0x10
        VLDMFDEQ R3!, {D8 - D15}
    ENDIF

    MSR     PSP, R3                                 ; /* Restore PSP */
    ; /* end: Restore next task stack */

    STR     R2,  [R1]                               ; /* g_os_current_task = g_os_next_task */

    ; /* Set task running state */
    LDR     R0,  =OS_TASK_STATE_RUNNING
    LDRH    R3,  [R2, #12]                          ; /* R3 = g_os_next_task->state */
    ORR     R3,  R3, R0                             ; /* g_os_next_task->state |= OS_TASK_STATE_RUNNING */
    STRH    R3,  [R2, #12]

    ; /* Restore interrupt */
    MSR     PRIMASK, R12

    BX      LR
    ENDP

os_first_task_start    PROC
    ; /* set the PendSV and Systick priority */
    LDR     R0, =NVIC_SYSPRI2
    LDR     R1, =NVIC_PENDSV_PRI :OR: NVIC_SYSTICK_PRI
    LDR.W   R2, [R0,#0x00]                          ; /* Read */
    ORR     R1, R1, R2                              ; /* Modify */
    STR     R1, [R0]                                ; /* Write-back */

    IF {FPU} != "SoftVFP"
        ; /* Clear CONTROL.FPCA */
        MRS     R2, CONTROL             ; /* Read */
        BIC     R2, #0x04               ; /* Modify */
        MSR     CONTROL, R2             ; /* Write-back */
    ENDIF

    ; /* Restore MSP */
    LDR     R0, =interrupt_stack_addr
    LDR     R0, [R0]
    MSR     MSP, R0

    ; /* Trigger the PendSV exception (causes context switch) */
    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    ; /* Enable interrupts at processor level */
    CPSIE   F
    CPSIE   I

    ; /* Never reach here! */
    B .
    ENDP

    ALIGN   4
    END


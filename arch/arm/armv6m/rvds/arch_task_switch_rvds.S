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
 ;* @file        os_arch_task_switch_keil.S
 ;*
 ;* @brief       This file provides context switch functions related to the ARM M4 architecture.
 ;*
 ;* @revision
 ;* Date         Author          Notes
 ;* 2020-10-10   OneOS Team      First version.
 ;***********************************************************************************************************************
 ;*/

SCB_VTOR                EQU     0xE000ED08              ; /* Vector table offset register */
NVIC_INT_CTRL           EQU     0xE000ED04              ; /* Interrupt control state register */
NVIC_SHPR3              EQU     0xE000ED20              ; /* System priority register (2) */
NVIC_PENDSV_PRI         EQU     0x00FF0000              ; /* PendSV priority value (lowest) */
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
    CMP     R0,  #0x00
    BEQ     switch_to_task                          ; /* If gs_os_current_task is OS_NULL, os_first_task_start trigger */

    ; /* begin: save current task registers to stack */
    MRS     R3,  PSP

    SUBS    R3, R3, #0x24                           ; /* Space for {r4 - r7} , {r8 - r11} and {LR} */
    STR     R3, [R0, #0]                            ; /* Update g_os_current_task->stack_top */

    STMIA   R3!, {R4 - R7}                          ; /* Push task {r4 - r7} register to task stack */

    MOV     R4, R8                                  ; /* Mov task {r8 - r11} to {r4 - r7} */
    MOV     R5, R9
    MOV     R6, R10
    MOV     R7, R11
    STMIA   R3!, {R4 - R7}                          ; /* Push task {r8 - r11} high register to task stack */ 

    MOV     R2, LR                                  ; /* Push EXC_RETURN */
    STMIA   R3!, {R2}
    ; /* end: save current task registers to stack */

    ; /* Clear current task running state */
    LDR     R2,  =OS_TASK_STATE_RUNNING
    LDRH    R3,  [R0, #12]                          ; /* R3 = g_os_current_task->state */
    BICS    R3,  R3, R2                             ; /* g_os_current_task->state &= (~OS_TASK_STATE_RUNNING) */
    STRH    R3,  [R0, #12]

    ; /* Check either task stack during task switching. */
    IF :DEF:OS_TASK_SWITCH_NOTIFY
    MOV     R2, R12
    PUSH    {R1, R2}
    BL      os_task_switch_notify
    POP     {R1, R2}
    MOV     R12, R2
    ENDIF

switch_to_task
    ; /* Get g_os_next_task->stack_top */
    LDR     R3, =g_os_next_task                     ; /* R3 = &g_os_next_task */
    LDR     R2,  [R3]                               ; /* R2 = g_os_next_task */

    ; /* begin: Restore next task stack */
    LDR     R3,  [R2, #0]                           ; /* R3 = g_os_next_task->stack_top; Pickup task stack pointer */
    LDMIA   R3!, {R4 - R7}                          ; /* Pop task {r4 - r7} register from task stack */
    PUSH    {R4 - R7}                               ; /* Push {r4 - r7} to MSP for copy {r8 - r11} */

    LDMIA   R3!, {R4 - R7}                          ; /* Pop task {r8 - r11} high register from task stack to {r4 - r7} */
    MOV     R8,  R4                                 ; /* Mov {r4 - r7} to {r8 - r11} */
    MOV     R9,  R5
    MOV     R10, R6
    MOV     R11, R7

    LDMIA   R3!, {R4}                               ; /* Pop EXC_RETURN */
    MOV     LR,  R4

    POP     {R4 - R7}                               ; /* Pop {r4 - r7} from MSP */

    MSR     PSP, R3                                 ; /* Restore PSP */
    ; /* end: Restore next task stack */

    ; /* g_os_current_task = g_os_next_task */
    STR     R2,  [R1]

    ; /* Set task running state */
    LDR     R0,  =OS_TASK_STATE_RUNNING
    LDRH    R3,  [R2, #12]                          ; /* R3 = g_os_next_task->state */
    ORRS    R3,  R3, R0                             ; /* g_os_next_task->state |= OS_TASK_STATE_RUNNING */
    STRH    R3,  [R2, #12]

    ; /* Restore interrupt */
    MSR     PRIMASK, R12

    BX      LR
    ENDP

os_first_task_start    PROC
    ; /*set the PendSV exception priority*/
    LDR     R0, =NVIC_SHPR3
    LDR     R1, =NVIC_PENDSV_PRI
    LDR     R2, [R0, #0x00]                         ; /* Read */
    ORRS    R1, R1, R2                              ; /* Modify */
    STR     R1, [R0]                                ; /* Write-back */

    ; /* Restore MSP */
    LDR     R0, =interrupt_stack_addr
    LDR     R0, [R0]
    MSR     MSP, R0

    ; /* Trigger the PendSV exception (causes context switch) */
    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    ; /* Enable interrupts at processor level */
    CPSIE   I

    ; /* Never reach here! */
    B .
    ENDP

    ALIGN   4
    END


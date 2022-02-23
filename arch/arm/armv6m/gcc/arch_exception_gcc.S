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
 * @file        arch_exception_gcc.S
 *
 * @brief       This file provides exception handlers related to the ARM-v6M architecture, and the compiler uses gcc.     
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-05   OneOS Team      First version.
 ***********************************************************************************************************************
 */
.syntax unified
.thumb
.text

.global HardFault_Handler

/**
 ***********************************************************************************************************************
 * @brief           HardFault interrupt handler.
 *
 * @param           None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
.type HardFault_Handler, %function

HardFault_Handler:
    /* Get current context */
    MOV     R2, LR                          /* Get EXC_RETURN */
    MOVS    R0, #0x04                       /* if(EXC_RETURN[2] == 1) */
    ANDS    R0, R0, R2
    CMP     R0, #0x04
    BNE     stack_sp_is_MSP
    
    MRS     R0, PSP                         /* EXC_RETURN[2] = 1, get fault context from task. */
    SUBS    R0, R0, #0x24                   /* Space for {r4 - r7} , {r8 - r11} , {EXC_RETURN} */
    MSR     PSP, R0                         /* EXC_RETURN[2] = 1, update stack pointer to PSP. */
    B       stack_push_regs

stack_sp_is_MSP:
    MRS     R0, MSP                         /* EXC_RETURN[2] = 0, get fault context from handler. */
    SUBS    R0, R0, #0x24                   /* Space for {r4 - r7} , {r8 - r11} , {EXC_RETURN} */
    MSR     MSP, R0                         /* EXC_RETURN[2] = 0, update stack pointer to MSP. */

stack_push_regs:
    ; /* begin: R4 - R11 and LR */
    MOV     R3, R0                          /* Reserved excption frame start addr */
    STMIA   R3!, {R4 - R7}                  /* Push task {r4 - r7} register to task stack */
    MOV     R4, R8                          /* Mov task {r8 - r11} to {r4 - r7} */
    MOV     R5, R9
    MOV     R6, R10
    MOV     R7, R11
    STMIA   R3!, {R4 - R7}                  /* Push task {r8 - r11} high register to task stack */ 
    STMIA   R3!, {R2}                       /* Push EXC_RETURN */
    ; /* end: R4 - R11 and LR */

    MRS     R1, MSP
    MRS     R2, PSP
    
    PUSH    {LR}    
    BL      os_arch_fault_exception
    POP     {PC}



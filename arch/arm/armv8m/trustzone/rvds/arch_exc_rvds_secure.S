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
 ;* @file        arch_exc_rvds_secure.S
 ;*
 ;* @brief       This file provides exception handlers related to the ARM-v8M architecture, and the compiler uses armcc.
 ;*
 ;* @revision
 ;* Date         Author          Notes
 ;* 2021-02-25   OneOS Team      First version.
 ;***********************************************************************************************************************
 ;*/

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT arch_fault_exception_secure
    IMPORT pHardFaultCallback

    EXPORT HardFault_Handler

;/**
; ***********************************************************************************************************************
; * @brief           HardFault interrupt handler.
; *
; * @param           None.
; *
; * @return          None.
; ***********************************************************************************************************************
; */
HardFault_Handler PROC
    ; /* Get current context */

    TST     LR, #0x40                               ; /* Bit[6] of EXC_RETURN is 1: trustzone stack was used 0: non-trustzone stack was used to store stack frame */
    BEQ     no_secure_call                          ; /* When the flag bit is 0, it means EQ. */

    TST     LR,  #0x04
    ITE     EQ
    MRSEQ   R0,  MSP                                ; /* EXC_RETURN[2] = 0, get fault context from handler. */
    MRSNE   R0,  PSP                                ; /* EXC_RETURN[2] = 1, get fault context from task. */

    ; /* Save FPU registers */
    IF {FPU} != "SoftVFP"
        TST       LR,  #0x10
        VSTMFDEQ  R0!, {D8 - D15}
    ENDIF

    STMFD   R0!, {R4 - R11, LR}                     ; /* Push r4 - r11, exc_return register */

    ; /* After pushing the stack frame, modify the used stack top pointer. */
    TST     LR,  #0x08
    ITE     EQ
    MSREQ   MSP, R0                                 ; /* EXC_RETURN[2] = 0, modify the used stack pointer MSP. */
    MSRNE   PSP, R0                                 ; /* EXC_RETURN[2] = 1, modify the used stack pointer PSP. */
    
    MRS     R1,  MSP
    MRS     R2,  PSP

    PUSH    {LR}
    BL      arch_fault_exception_secure
    POP     {LR}
    BX      LR 

no_secure_call
    TST     LR,  #0x08
    ITE     EQ
    MRSEQ   R0,  MSP_NS                             ; /* EXC_RETURN[3] = 0, get fault context from handler. */
    MRSNE   R0,  PSP_NS                             ; /* EXC_RETURN[3] = 1, get fault context from task. */

    ; /* Save FPU registers */
    IF {FPU} != "SoftVFP"
        TST       LR,  #0x10
        VSTMFDEQ  R0!, {D8 - D15}
    ENDIF

    STMFD   R0!, {R4 - R11}                         ; /* Push r4 - r11 register */

    ; /* Push g_arch_tz_context_memory_id lr psplim control to stack */
    MOV     R1,  #0                                 ; /* We don't care about g_arch_tz_context_memory_id, push 0 to the stack. */ 
    MRS     R2,  PSPLIM_NS                          ; /* R2 = PSPLIM_NS */
    STMFD   R0!, {R1-R2, LR}                        ; /* Push to task stack */

    ; /* After pushing the stack frame, modify the used stack top pointer. */
    TST     LR,  #0x08
    ITE     EQ
    MSREQ   MSP_NS,  R0                             ; /* EXC_RETURN[3] = 0, modify the used stack pointer MSP. */
    MSRNE   PSP_NS,  R0                             ; /* EXC_RETURN[3] = 1, modify the used stack pointer PSP. */

    MRS     R1,  MSP_NS
    MRS     R2,  PSP_NS

    PUSH    {LR}
    LDR     R3,  =pHardFaultCallback                ; /* R3 = &pHardFaultCallback */
    LDR     R3,  [R3,#0]                            ; /* R3 = pHardFaultCallback */
    BLXNS   R3
    POP     {LR}
 
    BX      LR 
    ENDP


    ALIGN   4
    END
    

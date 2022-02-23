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
 ;* @file        arch_exception_rvds.S
 ;*
 ;* @brief       This file provides exception handlers related to the ARM-v8M architecture, and the compiler uses armcc.
 ;*
 ;* @revision
 ;* Date         Author          Notes
 ;* 2021-01-23   OneOS Team      First version.
 ;***********************************************************************************************************************
 ;*/

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT os_arch_fault_exception

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
    TST     LR, #0x04
    ITE     EQ
    MRSEQ   R0, MSP                         ; /* EXC_RETURN[2] = 0, get fault context from handler. */
    MRSNE   R0, PSP                         ; /* EXC_RETURN[2] = 1, get fault context from task. */
 
    ; /* begin: Push FPU registers, R4 - R11 and LR */
    IF {FPU} != "SoftVFP"
        TST      LR,  #0x10
        VSTMFDEQ R0!, {D8 - D15}
    ENDIF

    MRS     R1 , PSPLIM
    STMFD   R0!, {R1}             
    STMFD   R0!, {R4 - R11, LR}
    ; /* end: Push FPU registers, R4 - R11 , LR and PSPLIM. */
 
    TST     LR,  #0x04
    ITE     EQ
    MSREQ   MSP, R0                         ; /* EXC_RETURN[2] = 0, update stack pointer to MSP. */
    MSRNE   PSP, R0                         ; /* EXC_RETURN[2] = 1, update stack pointer to PSP. */

    MRS     R1,  MSP
    MRS     R2,  PSP
    PUSH    {LR}
    BL      os_arch_fault_exception
    POP     {LR}
 
    BX      LR 
    ENDP


    ALIGN   4
    END


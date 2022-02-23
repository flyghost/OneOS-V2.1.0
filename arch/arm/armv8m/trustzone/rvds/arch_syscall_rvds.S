;/**
; ***********************************************************************************************************************
; * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
; *
; * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
; * the License. You may obtain a copy of the License at
; *
; *     http://www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
; * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
; * specific language governing permissions and limitations under the License.
; *
; * @file        arch_syscall_rvds.S
; *
; * @brief       This file provides syscall functions related to the ARMv8-M architecture.
; *
; * @revision
; * Date         Author          Notes
; * 2021-01-12   OneOS Team      First version.
; ***********************************************************************************************************************
; */

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

    IMPORT _arch_tz_svc_handle

    EXPORT arch_system_call
    EXPORT arch_trustzone_call
    EXPORT SVC_Handler

;/**
; ***********************************************************************************************************************
; * @brief           Trigger system call.
; *
; * @param           None
; *
; * @return          None.
; ***********************************************************************************************************************
; */
arch_system_call PROC
    SVC     0
    BX      LR

    ENDP

;/**
; ***********************************************************************************************************************
; * @brief           Trigger trustzone call.
; *
; * @param           None
; *
; * @return          trustzone call result.
; ***********************************************************************************************************************
; */
arch_trustzone_call PROC
    SVC     1
    BX      LR

    ENDP

;/**
; ***********************************************************************************************************************
; * @brief           SVC call process.
; *
; * @param           None
; *
; * @return          svc call result.
; ***********************************************************************************************************************
; */
SVC_Handler    PROC
    TST     LR, #4
    ITE     EQ
    ; /* Get msp if return stack is msp */
    MRSEQ   R1, MSP

    ; /* Get psp if return stack is psp */
    MRSNE   R1, PSP

    ; /* Get svc number */
    LDR     R0, [R1, #24]
    LDRB    R0, [R0, #-2]

    ; /* Handle trustzone call */
    CMP     R0, #0x1
    BEQ     trustzone_call

    ; /* Handle system call */
    CMP     R0, #0x0
    BEQ     system_call

    BX      LR

trustzone_call
    PUSH    {R1, R4, LR}

    ; /* Copy task SP to R4 */
    MOV     R4, R1

    ; /* Get input cmdid, arg0, arg1, arg2 */
    LDMFD   R4!, {R0 - R3}

    ; /* Call _arch_tz_svc_handle */
    BL      _arch_tz_svc_handle
    POP     {R1, R4, LR}

    ; /* Update return value */
    STR     R0, [R1]
    BX      LR

system_call
    PUSH    {LR}
    ;BL      xxxxxxx
    POP     {LR}
    BX      LR

    ENDP

    ALIGN
    END

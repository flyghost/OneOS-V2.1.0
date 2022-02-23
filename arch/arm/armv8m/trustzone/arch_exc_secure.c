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
 * @file        arch_exc_secure.c
 *
 * @brief       This file provides exception functions related to the Cortex-V8M secure zone.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-25   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "arch_exc_secure.h"
#include <stdio.h>

/**
 ***********************************************************************************************************************
 * @brief           This function handles hard fault exception.
 *
 * @param[in]       stack_frame     The start address of the stack frame when the exception occurs.
 * @param[in]       msp             Interrupt stack pointer.
 * @param[in]       psp             Currently thread stack pointer.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void arch_fault_exception_secure(void *stack_frame, unsigned long *msp, unsigned long *psp)
{
    struct stack_frame_common   *stack_common;
    struct stack_frame          *nofpu_frame;
    struct stack_frame_fpu      *fpu_frame;

    printf("Hard fault in secure zone\r\n");
    
    stack_common = (struct stack_frame_common *)stack_frame;

    /* Stack frame with floating point storage */
    if (!(stack_common->exc_return & EXCEPTION_STACK_FRAME_TYPE_MASK))
    {
        fpu_frame = (struct stack_frame_fpu *)stack_common;

        printf("FPU active! FPCSR is 0x%08x\r\n", fpu_frame->cpu_frame_fpu.FPSCR);

        printf("R0 : 0x%08x\r\n", fpu_frame->cpu_frame_fpu.r0);
        printf("R1 : 0x%08x\r\n", fpu_frame->cpu_frame_fpu.r1);
        printf("R2 : 0x%08x\r\n", fpu_frame->cpu_frame_fpu.r2);
        printf("R3 : 0x%08x\r\n", fpu_frame->cpu_frame_fpu.r3);
        printf("R4 : 0x%08x\r\n", fpu_frame->frame_common_fpu.r4);
        printf("R5 : 0x%08x\r\n", fpu_frame->frame_common_fpu.r5);
        printf("R6 : 0x%08x\r\n", fpu_frame->frame_common_fpu.r6);
        printf("R7 : 0x%08x\r\n", fpu_frame->frame_common_fpu.r7);
        printf("R8 : 0x%08x\r\n", fpu_frame->frame_common_fpu.r8);
        printf("R9 : 0x%08x\r\n", fpu_frame->frame_common_fpu.r9);
        printf("R10: 0x%08x\r\n", fpu_frame->frame_common_fpu.r10);
        printf("R11: 0x%08x\r\n", fpu_frame->frame_common_fpu.r11);
        printf("R12: 0x%08x\r\n", fpu_frame->cpu_frame_fpu.r12);
        printf("LR : 0x%08x\r\n", fpu_frame->cpu_frame_fpu.lr);
        printf("PC : 0x%08x\r\n", fpu_frame->cpu_frame_fpu.pc);
        printf("PSR: 0x%08x\r\n", fpu_frame->cpu_frame_fpu.psr);
    }
    /* Stack frame without floating point storage. */
    else
    {
        nofpu_frame = (struct stack_frame *)stack_common;

        printf("R0 : 0x%08x\r\n", nofpu_frame->cpu_frame.r0);
        printf("R1 : 0x%08x\r\n", nofpu_frame->cpu_frame.r1);
        printf("R2 : 0x%08x\r\n", nofpu_frame->cpu_frame.r2);
        printf("R3 : 0x%08x\r\n", nofpu_frame->cpu_frame.r3);
        printf("R4 : 0x%08x\r\n", nofpu_frame->frame_common.r4);
        printf("R5 : 0x%08x\r\n", nofpu_frame->frame_common.r5);
        printf("R6 : 0x%08x\r\n", nofpu_frame->frame_common.r6);
        printf("R7 : 0x%08x\r\n", nofpu_frame->frame_common.r7);
        printf("R8 : 0x%08x\r\n", nofpu_frame->frame_common.r8);
        printf("R9 : 0x%08x\r\n", nofpu_frame->frame_common.r9);
        printf("R10: 0x%08x\r\n", nofpu_frame->frame_common.r10);
        printf("R11: 0x%08x\r\n", nofpu_frame->frame_common.r11);
        printf("R12: 0x%08x\r\n", nofpu_frame->cpu_frame.r12);
        printf("LR : 0x%08x\r\n", nofpu_frame->cpu_frame.lr);
        printf("PC : 0x%08x\r\n", nofpu_frame->cpu_frame.pc);
        printf("PSR: 0x%08x\r\n", nofpu_frame->cpu_frame.psr);
    }

    /* Hard fault is generated in task context. */
    if (stack_common->exc_return & EXCEPTION_RETURN_MODE_MASK)
    {
        printf("Hard fault in thread mode \r\n");
    }
    /* Hard fault is generated in interrupt context. */
    else
    {
        printf("Hard fault in interrupt\r\n");
    }
    
    _arch_hard_fault_track();
    
    while (1);
}

static void _arch_usage_fault_track(void)
{
    printf("usage fault:\r\n");
    printf("SCB_CFSR_UFSR: 0x%02x\r\n", SCB_CFSR_UFSR);

    if (SCB_CFSR_UFSR & (1 << 0))
    {
        printf("UNDEFINSTR\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 1))
    {
        printf("INVSTATE\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 2))
    {
        printf("INVPC\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 3))
    {
        printf("NOCP\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 4))
    {
        printf("STKOF\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 8))
    {
        printf("UNALIGNED\r\n");
    }

    if (SCB_CFSR_UFSR & (1 << 9))
    {
        printf("DIVBYZERO\r\n");
    }
}

static void _arch_bus_fault_track(void)
{
    printf("bus fault:\r\n");
    printf("SCB_CFSR_BFSR: 0x%02x\r\n", SCB_CFSR_BFSR);

    if (SCB_CFSR_BFSR & (1 << 0))
    {
        printf("IBUSERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 1))
    {
        printf("PRECISERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 2))
    {
        printf("IMPRECISERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 3))
    {
        printf("UNSTKERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 4))
    {
        printf("STKERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 5))
    {
        printf("LSPERR\r\n");
    }

    if (SCB_CFSR_BFSR & (1 << 7))
    {
        printf("BFARVALID\r\n");
        printf("SCB->BFAR:%08x\r\n", SCB_BFAR);
    }

    return;
}

static void _arch_mem_manage_fault_track(void)
{
    printf("mem manage fault:\r\n");
    printf("SCB_CFSR_MFSR: 0x%02x\r\n", SCB_CFSR_MFSR);

    if (SCB_CFSR_MFSR & (1 << 0))
    {
        printf("IACCVIOL\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 1))
    {
        printf("DACCVIOL\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 3))
    {
        printf("MUNSTKERR\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 4))
    {
        printf("MSTKERR\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 5))
    {
        printf("MLSPERR\r\n");
    }

    if (SCB_CFSR_MFSR & (1 << 7))
    {
        printf("MMARVALID\r\n");
        printf("SCB->MMAR: 0x%08x\r\n", SCB_MMAR);
    }

    return;
}

static void _arch_secure_fault_track(void)
{
    printf("secure fault:\r\n");
    printf("SAU_SFSR: 0x%02x\r\n", SAU_SFSR);

    if (SAU_SFSR & (1 << 0))
    {
        printf("INVEP\r\n");
    }

    if (SAU_SFSR & (1 << 1))
    {
        printf("INVIS\r\n");
    }

    if (SAU_SFSR & (1 << 2))
    {
        printf("INVER\r\n");
    }

    if (SAU_SFSR & (1 << 3))
    {
        printf("AUVIOL\r\n");
    }

    if (SAU_SFSR & (1 << 4))
    {
        printf("INVTRAN\r\n");
    }

    if (SAU_SFSR & (1 << 5))
    {
        printf("LSPERR\r\n");
    }

    if (SAU_SFSR & (1 << 6))
    {
        printf("SFARVALID\r\n");
        printf("SAU_SFAR: 0x%08x\r\n", SAU_SFAR);
    }

    if (SAU_SFSR & (1 << 7))
    {
        printf("LSERR\r\n");
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function analyzes the cause of hard fault. 
 *
 * @attention       This function is also used to analyze non-secure hard fault. The reason is that non-secure 
 *                  cannot read the registers of the exception cause.
 *
 * @param           None
 *
 * @return          None
 ***********************************************************************************************************************
 */
void _arch_hard_fault_track(void)
{
    if (SCB_HFSR & (1UL << 1))
    {
        printf("Failed vector fetch\r\n");
    }

    if (SCB_HFSR & (1UL << 30))
    {
        
        if (SCB_CFSR_UFSR)
        {
            _arch_usage_fault_track();
        }
        
        if (SCB_CFSR_BFSR)
        {
            _arch_bus_fault_track();
        }

        if (SCB_CFSR_MFSR)
        { 
            _arch_mem_manage_fault_track();
        }

        if(SAU_SFSR)
        {
            _arch_secure_fault_track();
        }
    }

    if(SCB_HFSR & (1UL << 31))
    {
        printf("Debug event\r\n");
    }

    return;
}
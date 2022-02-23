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
 * @file        arch_exc_secure.h
 *
 * @brief       A head file of exception functions related to the Cortex-V8M secure zone.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-25   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __OS_ARCH_EXC_SECURE_H__
#define __OS_ARCH_EXC_SECURE_H__


#ifdef __cplusplus
    extern "C" {
#endif

#define SAU_SFSR            (*(volatile const unsigned int *)0xE000EDE4)        /* Secure Fault Status Register */
#define SAU_SFAR            (*(volatile const unsigned int *)0xE000EDE8)        /* Secure Fault Address Register */

 

#define SCB_CFSR            (*(volatile const unsigned int *)0xE000ED28)        /* Configurable Fault Status Register */
#define SCB_HFSR            (*(volatile const unsigned int *)0xE000ED2C)        /* HardFault Status Register */
#define SCB_MMAR            (*(volatile const unsigned int *)0xE000ED34)        /* MemManage Fault Address register */
#define SCB_BFAR            (*(volatile const unsigned int *)0xE000ED38)        /* Bus Fault Address Register */
#define SCB_AIRCR           (*(volatile unsigned int *)0xE000ED0C)              /* Reset control Address Register */
#define SCB_RESET_VALUE     0x05FA0004                                          /* Reset value, write to SCB_AIRCR can reset cpu */
      
#define SCB_CFSR_MFSR       (*(volatile const unsigned char*)0xE000ED28)        /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR       (*(volatile const unsigned char*)0xE000ED29)        /* Bus Fault Status Register */
#define SCB_CFSR_UFSR       (*(volatile const unsigned short*)0xE000ED2A)       /* Usage Fault Status Register */

/* 1 (Stack frame contains floating point) or 0 (Stack frame does not contain floating point) */
#define EXCEPTION_STACK_FRAME_TYPE_MASK             0x00000010
/* 1 (Return to Thread) or 0 (Return to Handler) */
#define EXCEPTION_RETURN_MODE_MASK                  0x00000008
#define EXCEPTION_RETURN_MODE_THREAD                0x00000008
#define EXCEPTION_RETURN_MODE_HANDLER               0x00000000

/* 1 (Return with Process Stack) or 0 (Return with Main Stack)*/
#define EXCEPTION_RETURN_STACK_MASK                 0x00000004              

typedef unsigned int                    os_uint32_t;            /* 32bit unsigned integer type */

struct cpu_stack_frame
{
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;
};

struct cpu_stack_frame_fpu
{
    os_uint32_t r0;
    os_uint32_t r1;
    os_uint32_t r2;
    os_uint32_t r3;
    os_uint32_t r12;
    os_uint32_t lr;
    os_uint32_t pc;
    os_uint32_t psr;

    /* FPU registers */
    os_uint32_t S0;
    os_uint32_t S1;
    os_uint32_t S2;
    os_uint32_t S3;
    os_uint32_t S4;
    os_uint32_t S5;
    os_uint32_t S6;
    os_uint32_t S7;
    os_uint32_t S8;
    os_uint32_t S9;
    os_uint32_t S10;
    os_uint32_t S11;
    os_uint32_t S12;
    os_uint32_t S13;
    os_uint32_t S14;
    os_uint32_t S15;
    os_uint32_t FPSCR;
    os_uint32_t NO_NAME;
};

struct stack_frame_common
{
    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;
    os_uint32_t exc_return;
};

struct stack_frame_common_fpu
{
    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;
    os_uint32_t exc_return;

    /* FPU registers */
    os_uint32_t s16;
    os_uint32_t s17;
    os_uint32_t s18;
    os_uint32_t s19;
    os_uint32_t s20;
    os_uint32_t s21;
    os_uint32_t s22;
    os_uint32_t s23;
    os_uint32_t s24;
    os_uint32_t s25;
    os_uint32_t s26;
    os_uint32_t s27;
    os_uint32_t s28;
    os_uint32_t s29;
    os_uint32_t s30;
    os_uint32_t s31;
};

struct stack_frame
{
    /* Push or pop stack manually */
    struct stack_frame_common   frame_common;

    /* Push or pop stack automatically */
    struct cpu_stack_frame      cpu_frame;
};

struct stack_frame_fpu
{
    /* Push or pop stack manually */
    struct stack_frame_common_fpu     frame_common_fpu;

    /* Push or pop stack automatically */
    struct cpu_stack_frame_fpu        cpu_frame_fpu;
};

extern void _arch_hard_fault_track(void);

#ifdef __cplusplus
    }
#endif

#endif /* __OS_ARCH_EXCEPTION_H__ */


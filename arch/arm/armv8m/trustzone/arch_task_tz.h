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
 * @file        arch_task.h
 *
 * @brief       This file provides data struct related to the ARMv8-M architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __ARCH_TASK_TZ_H__
#define __ARCH_TASK_TZ_H__

#include <oneos_config.h>
#include <os_stddef.h>
#include <os_types.h>

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
    os_uint32_t tz;
    os_uint32_t psplim;
    os_uint32_t exc_return;

    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;
};

struct stack_frame_common_fpu
{
    os_uint32_t tz;
    os_uint32_t psplim;
    os_uint32_t exc_return;

    os_uint32_t r4;
    os_uint32_t r5;
    os_uint32_t r6;
    os_uint32_t r7;
    os_uint32_t r8;
    os_uint32_t r9;
    os_uint32_t r10;
    os_uint32_t r11;

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
#endif


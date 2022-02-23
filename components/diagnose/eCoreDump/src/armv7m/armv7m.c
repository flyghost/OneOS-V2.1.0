/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *use this file except in compliance with the License. You may obtain a copy of
 *the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file        armv7m.c
 *
 * @brief       This file provide armv7m specified code.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include "ecoredump.h"
#include "ecd_arch_define.h"

#define FPU_CPACR    0xE000ED88

int is_vfp_addressable()
{
    uint32_t reg_cpacr = *((volatile uint32_t *)FPU_CPACR);
    if (reg_cpacr & 0x00F00000)
        return 1;
    else
        return 0;
}

#if defined(__CC_ARM)

__asm void ecd_mini_dump()
{
    extern get_cur_core_regset_address;
    extern get_cur_fp_regset_address;
    extern ecd_mini_dump_ops;
    extern ecd_gen_coredump;
    extern is_vfp_addressable;

    PRESERVE8

    push {r7, lr}
    sub sp, sp, #24
    add r7, sp, #0

get_regset
    bl get_cur_core_regset_address
    str r0, [r0 , #0]
    add r0, r0, #4
    stmia r0!, {r1 - r12}
    mov r1, sp
    add r1, #32
    str r1, [r0, #0]
    ldr r1, [sp, #28]
    str r1, [r0, #4]
    add r1, pc
    str r1, [r0, #8]
    mrs r1, xpsr
    str r1, [r0, #12]

    bl is_vfp_addressable
    cmp r0, #0
    beq get_reg_done

    bl get_cur_fp_regset_address
    vstmia r0!, {d0 - d15}
    vmrs r1, fpscr
    str r1, [r0, #0]

get_reg_done
    mov r0, r7
    bl ecd_mini_dump_ops
    mov r0, r7
    bl ecd_gen_coredump
    nop
    adds r7, r7, #24
    mov sp, r7
    pop {r7, pc}
    nop
    nop
}

__asm void ecd_multi_dump()
{
    extern get_cur_core_regset_address;
    extern get_cur_fp_regset_address;
    extern ecd_rtos_thread_ops;
    extern ecd_gen_coredump;
    extern is_vfp_addressable;

    PRESERVE8

    push {r7, lr}
    sub sp, sp, #24
    add r7, sp, #0

get_regset1
    bl get_cur_core_regset_address
    str r0, [r0 , #0]
    add r0, r0, #4
    stmia r0!, {r1 - r12}
    mov r1, sp
    add r1, #32
    str r1, [r0, #0]
    ldr r1, [sp, #28]
    str r1, [r0, #4]
    add r1, pc
    str r1, [r0, #8]
    mrs r1, xpsr
    str r1, [r0, #12]

    bl is_vfp_addressable
    cmp r0, #0
    beq get_reg_done

    bl get_cur_fp_regset_address
    vstmia r0!, {d0 - d15}
    vmrs r1, fpscr
    str r1, [r0, #0]

get_reg_done1
    mov r0, r7
    bl ecd_rtos_thread_ops
    mov r0, r7
    bl ecd_gen_coredump
    nop
    adds r7, r7, #24
    mov sp, r7
    pop {r7, pc}
    nop
    nop
}

#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) \
          || defined ( __GNUC__ )

#define ecd_get_regset(regset)           \
    __asm volatile("  mov r0, %0  \n"              \
                   "  str r0, [r0 , #0]  \n"       \
                   "  add r0, r0, #4      \n"      \
                   "  stmia  r0!, {r1 - r12} \n"   \
                   "  mov r1, sp \n"               \
                   "  str  r1, [r0, #0]  \n"       \
                   "  mov r1, lr \n"               \
                   "  str  r1, [r0, #4]  \n"       \
                   "  mov r1, pc \n"               \
                   "  str  r1, [r0, #8]  \n"       \
                   "  mrs r1, xpsr  \n"            \
                   "  str  r1, [r0, #12]  \n"      \
                   :: "r"(regset) : "memory", "cc");

#define ecd_get_fpregset(regset)          \
    __asm volatile("mov r0, %0  \n"                   \
                    " vstmia r0!, {d0 - d15}   \n"    \
                    "  vmrs r1, fpscr           \n"   \
                    "  str  r1, [r0, #0]        \n"   \
                    :: "r"(regset) : "memory", "cc");

void ecd_mini_dump()
{
    struct thread_info_ops ops;

    ecd_get_regset((uint32_t *)get_cur_core_regset_address());

    if (is_vfp_addressable())
        ecd_get_fpregset((uint32_t *)get_cur_fp_regset_address());

    ecd_mini_dump_ops(&ops);
    ecd_gen_coredump(&ops);
}

void ecd_multi_dump()
{
    struct thread_info_ops ops;

    ecd_get_regset((uint32_t *)get_cur_core_regset_address());

    if (is_vfp_addressable())
        ecd_get_fpregset((uint32_t *)get_cur_fp_regset_address());

    ecd_rtos_thread_ops(&ops);
    ecd_gen_coredump(&ops);
}

#endif

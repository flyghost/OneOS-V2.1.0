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
 * @file        mips_excpt_gcc.S
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif

#include "../common/mips.h"

#define _EXC_STKSIZE 	20*1024

#define PTE_BASE_OFFSET                 23
#define PTE_BASE_SIZE                   9
#define MIPS32_BADVPN2_SHIFT            2


.section ".text", "ax"
.set	noreorder

LEAF(mips_tlb_refill_handlerx)
    .set    push
    .set    noat
    .set    noreorder
    .set    volatile

    ;/*
        ; * K1 = CP0_CTXT
        ; * K0 = K1
        ; */
    mfc0    k1 , CP0_CONTEXT 
    ehb
    move    k0 , k1 

    ;/*
        ; * K1 <<= PTE_BASE_SIZE
        ; * K1 >>= PTE_BASE_SIZE
        ; * K1 >>= 4
        ; * K1 >>= MIPS32_BADVPN2_SHIFT
        ; * K1 <<= 3
        ; */
    sll     k1 , PTE_BASE_SIZE
    srl     k1 , (PTE_BASE_SIZE + 4 + MIPS32_BADVPN2_SHIFT)
sll     k1 , (4 - 1)

    ;/*
        ; * K0 >>= PTE_BASE_OFFSET
        ; * K0 <<= PTE_BASE_OFFSET
        ; */
    srl     k0 , PTE_BASE_OFFSET
    sll     k0 , PTE_BASE_OFFSET

    ;/*
        ; * K1 = K1 | K0
        ; */
    or      k1 , k1 , k0

    ;/*
        ; * K0 = *K1
        ; * K1 = *(K1 + 4)
        ; */
    lw      k0 , 0(k1)
lw      k1 , 4(k1)

    ;/*
        ; * CP0_TLBLO0 = K0
        ; * CP0_TLBLO1 = K1
        ; */
    mtc0    k0 , CP0_ENTRYLO0
    mtc0    k1 , CP0_ENTRYLO1
    ehb

    tlbwr

    eret

    .set    pop
END(mips_tlb_refill_handlerx)

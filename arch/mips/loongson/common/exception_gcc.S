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
 * @file        board.h
 *
 * @brief       Board resource definition
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif

#include "mips.h"

    .section ".exc_vectors", "ax"
    .extern tlb_refill_handler
    .extern cache_error_handler
    .extern mips_irq_handle

    /* 0x0 - TLB refill handler */
    .global tlb_refill_exception
    .type	tlb_refill_exception,@function
ebase_start:
tlb_refill_exception:
    b	_general_exception_handler
    nop

    /* 0x080 - XTLB refill handler */
    .org ebase_start + 0x080
    b	_general_exception_handler
    nop

    /* 0x100 - Cache error handler */
    .org ebase_start + 0x100
    j	cache_error_handler
    nop

    /* 0x180 - Exception/Interrupt handler */
    .global general_exception
    .type	general_exception,@function
    .org ebase_start + 0x180
general_exception:
    b	_general_exception_handler
    nop

    /* 0x200 - Special Exception Interrupt handler (when IV is set in CP0_CAUSE) */
    .global irq_exception
    .type	irq_exception,@function
    .org ebase_start + 0x200
irq_exception:
    b	_general_exception_handler
    nop

    /* general exception handler */
_general_exception_handler:
    .set	noreorder
    PTR_LA	k0, mips_irq_handle
    jr	k0
    nop
    .set	reorder

    /* interrupt handler */
_irq_handler:
    .set	noreorder
    PTR_LA	k0, mips_irq_handle
    jr	k0
    nop
    .set	reorder


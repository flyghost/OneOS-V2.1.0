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
 * @file        entry_gcc.s
 *
 * @brief       entry asm file
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
//#include <oneos_config.h>

.section ".start", "ax"
.set noreorder

/* the program entry */
.globl  _start
_start:
PTR_LA	ra, _start

/* disable interrupt */
MTC0	zero, CP0_CAUSE
MTC0	zero, CP0_STATUS	# Set CPU to disable interrupt.
ehb

#ifdef ARCH_MIPS64
dli		t0, ST0_KX
MTC0	t0, CP0_STATUS
#endif

/* setup stack pointer */
PTR_LA	sp, _system_stack
PTR_SUBU sp, sp, 8
PTR_LA	gp, _gp

bal	os_cpu_early_init
nop

/* clear bss */
PTR_LA	t0, __bss_start
PTR_LA	t1, __bss_end

_clr_bss_loop:
sw	zero, 0(t0)
    bne	t1, t0, _clr_bss_loop
    addu	t0, 4
    /* jump to oneos */
    jal	entry
    nop

    /* restart, never die */
    j	_start
    nop


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
 * @file        mips_cache_gcc.S
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ASSEMBLY__
#define	__ASSEMBLY__
#endif

#include "../common/mips.h"

.text
.set noreorder

.globl  cache_init
.ent    cache_init
cache_init:
.set    noreorder
mtc0    zero, CP0_TAGLO
move    t0, a0                  // cache total size
move    t1, a1                  // cache line size
li      t2, 0x80000000
addu    t3, t0, t2

_cache_init_loop:
    cache   8, 0(t2)                // icache_index_store_tag
cache   9, 0(t2)                // dcache_index_store_tag
    addu    t2, t1
    bne     t2, t3, _cache_init_loop
    nop

    mfc0    t0, CP0_CONFIG
    li      t1, 0x7
    not     t1
    and     t0, t0, t1
    or      t0, 0x3                 // cacheable, noncoherent, write-back, write allocate
    mtc0    t0, CP0_CONFIG

    jr      ra
    nop

    .set    reorder
    .end    cache_init

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
 * @file        cpu.c
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <x1000.h>
#include <arch_interrupt.h>
#include <os_util.h>
#include "../common/mips.h"
#include <os_types.h>

mips32_core_cfg_t g_mips_core =
{
    .icache_line_size 	= 32,
    .icache_size		= 16384,
    .dcache_line_size 	= 32,
    .dcache_size		= 16384,
    .max_tlb_entries 	= 16,		/* max_tlb_entries */
};

void os_hw_tlb_init(void)
{
    /*----------------------------------------------------------------------------------
      cchappy tlb  0x30000000 to 0xC0000000
      ------------------------------------------------------------------------------------
      */
    unsigned int pagemask = 0x007fe000;//0x01ffe000; /* 4MB */
    /* cached D:allow-W V:valid G */
    unsigned int entrylo0 = (0x30000000 >> 6) | (3 << 3) + (1 << 2) + (1 << 1) + 1;
    unsigned int entrylo1 = (0x30400000 >> 6) | (3 << 3) + (1 << 2) + (1 << 1) + 1;
    unsigned int entryhi 	= 0xc0000000; /* kseg2 base */
    int i;
    __write_32bit_c0_register($5, 4, 0xa9000000);
    write_c0_pagemask(pagemask);
    write_c0_wired(0);
    /* indexed write 32 tlb entry */
    for(i = 0; i < 32; i++)
    {
        asm (
                ".macro _ssnop; sll $0, $0, 1; .endm\n\t"
                ".macro _ehb; sll $0, $0, 3; .endm\n\t"
                ".macro mtc0_tlbw_hazard; _ssnop; _ssnop; _ehb; .endm\n\t"
                ".macro tlbw_use_hazard; _ssnop; _ssnop; _ssnop; _ehb; .endm\n\t"
                "\n\t"
                "mtc0 %0, $0\n\t" /* write Index */
                "tlbw_use_hazard\n\t"
                "mtc0 %1, $5\n\t" /* write PageMask */
                "mtc0 %2, $10\n\t" /* write EntryHi */
                "mtc0 %3, $2\n\t" /* write EntryLo0 */
                "mtc0 %4, $3\n\t" /* write EntryLo1 */
                "mtc0_tlbw_hazard\n\t"
                "tlbwi \n\t" /* TLB indexed write */
                "tlbw_use_hazard\n\t"
                : : "Jr" (i), "r" (pagemask), "r" (entryhi),
                "r" (entrylo0), "r" (entrylo1)
            );
        entryhi += 0x0800000; /* 32MB */
        entrylo0 += (0x0800000 >> 6);
        entrylo1 += (0x0800000 >> 6);
    }
}

void os_hw_cache_init(void)
{
    r4k_cache_flush_all();
}

/**
 * this function will reset CPU
 *
 */
OS_WEAK void os_hw_cpu_reset()
{
    /* open the watch-dog */
    REG_WDT_TCSR  = WDT_TCSR_EXT_EN;
    REG_WDT_TCSR |= WDT_TCSR_PRESCALE_1024;
    REG_WDT_TDR   = 0x03;
    REG_WDT_TCNT  = 0x00;
    REG_WDT_TCER |= WDT_TCER_TCEN;

    os_kprintf("reboot system...\n");
    os_irq_lock();
    while (1);
}

/**
 * this function will shutdown CPU
 *
 */
OS_WEAK void os_hw_cpu_shutdown()
{
    os_kprintf("shutdown...\n");
    os_irq_lock();
    while (1);
}

/**
 * This function finds the first bit set (beginning with the least significant bit)
 * in value and return the index of that bit.
 *
 * Bits are numbered starting at 1 (the least significant bit).  A return value of
 * zero from any of these functions means that the argument was zero.
 *
 * @return return the index of the first bit set. If value is 0, then this function
 * shall return 0.
 */
OS_WEAK int __os_ffs(int value)
{
    return __builtin_ffs(value);
}

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
 * @file        cpuport.c
 *
 * @brief       cpu port file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-25   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_stddef.h>
#include <gs232.h>

/**
 * @addtogroup Loongson GS232
 */

/*@{*/

/**
 * this function will reset CPU
 *
 */
OS_WEAK void os_hw_cpu_reset(void)
{
    /* open the watch-dog */
    WDT_EN = 0x01;      /* watch dog enable */
    WDT_TIMER = 0x01;   /* watch dog will be timeout after 1 tick */
    WDT_SET = 0x01;     /* watch dog start */

    os_kprintf("reboot system...\n");
    while (1);
}

/**
 * this function will shutdown CPU
 *
 */
OS_WEAK void os_hw_cpu_shutdown(void)
{
    os_kprintf("shutdown...\n");

    while (1);
}

#define Hit_Invalidate_I    0x10
#define Hit_Invalidate_D    0x11
#define CONFIG_SYS_CACHELINE_SIZE   32
#define Hit_Writeback_Inv_D 0x15

void flush_cache(unsigned long start_addr, unsigned long size)
{
    unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
    unsigned long addr = start_addr & ~(lsize - 1);
    unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);

    while (1) {
        cache_op(Hit_Writeback_Inv_D, addr);
        cache_op(Hit_Invalidate_I, addr);
        if (addr == aend)
            break;
        addr += lsize;
    }
}


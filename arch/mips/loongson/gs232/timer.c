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
 * @file        timer.c
 *
 * @brief       timer driver
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <arch_hw.h>
#include <mips.h>
#include <board.h>
#include <os_clock.h>

/**
 * This is the timer interrupt service routine.
 */
void os_hw_timer_handler(void)
{
    unsigned int count;

    count = read_c0_compare();
    write_c0_compare(count);
    write_c0_count(0);
    /* increase a OS tick */
    os_tick_increase();
}

/**
 * This function will initial OS timer
 */
void os_hw_timer_init(void)
{
    write_c0_compare(CPU_HZ/2/OS_TICK_PER_SECOND);
    write_c0_count(0);
    mips_unmask_cpu_irq(7);
}


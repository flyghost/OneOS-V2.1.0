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
 * @file        cputime_test.c
 *
 * @brief       The test file for cputime.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_clock.h>
#include <oneos_config.h>
#include <timer/clocksource.h>
#include <timer/timekeeping.h>
#include <shell.h>

void timekeeping_test(int argc, char *argv[])
{
    int i, delay;
    os_uint64_t start, now;
    os_tick_t tick_start, tick_now;
    struct timeval tp_start, tp_now;

    os_task_t *self = os_task_self();
    os_uint8_t task_prio = self->current_priority;
    os_uint8_t high_prio = 1;
    os_task_set_priority(self,high_prio);

    for (i = 0; i < 12; i++)
    {
        delay = 1 << i;
        
        /* sync tick */
        os_task_msleep(1);

        tick_start = os_tick_get();
        start = timekeeping_gettimeofday(&tp_start, OS_NULL);
        os_task_msleep(delay);
        tick_now = os_tick_get();
        now = timekeeping_gettimeofday(&tp_now, OS_NULL);

        os_kprintf("msleep start tick: %d\r\n", tick_start);
        os_kprintf("msleep now   tick: %d             delta tick: %d\r\n",
                   tick_now, tick_now - tick_start);
        os_kprintf("msleep start time: %d.%09d\r\n", (int)(start / NSEC_PER_SEC), (int)(start % NSEC_PER_SEC));
        os_kprintf("msleep now   time: %d.%09d    %d, delta time: %d us\r\n\r\n",
                   (int)(now / NSEC_PER_SEC), (int)(now % NSEC_PER_SEC), delay, (int)((now - start) / 1000));
    }

    for (i = 0; i < 12; i++)
    {
        delay = (1 << i) * 1000 * 1000;
        tick_start = os_tick_get();
        start = timekeeping_gettimeofday(&tp_start, OS_NULL);
        os_clocksource_ndelay(delay);
        tick_now = os_tick_get();
        now = timekeeping_gettimeofday(&tp_now, OS_NULL);

        os_kprintf("udelay start tick: %d\r\n", tick_start);
        os_kprintf("udelay now   tick: %d             delta tick: %d\r\n",
                   tick_now, tick_now - tick_start);
        os_kprintf("udelay start time: %d.%09d\r\n", (int)(start / NSEC_PER_SEC), (int)(start % NSEC_PER_SEC));
        os_kprintf("udelay now   time: %d.%09d    %d, delta time: %d us\r\n\r\n",
                   (int)(now / NSEC_PER_SEC), (int)(now % NSEC_PER_SEC), delay, (int)((now - start) / 1000));
    }

    os_task_set_priority(self,task_prio);
}

SH_CMD_EXPORT(timekeeping_test, timekeeping_test, "timekeeping_test");


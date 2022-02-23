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
 * @file        drv_sram.c
 *
 * @brief       This file implements SRAM driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <board.h>

#ifdef OS_USING_SHELL
#include <shell.h>
#include <os_clock.h>
static int sram_test(void)
{
    int i = 0;
    uint32_t start_time = 0, time_cast = 0;
    uint8_t data       = 0;

    /* write data */
    os_kprintf("Writing the %ld bytes data, waiting....\r\n", SRAM_SIZE);
    start_time = os_tick_get();
    for (i = 0; i < SRAM_SIZE; i++)
    {
        *(__IO uint8_t *)(SRAM_BANK_ADDR + i) = (uint8_t)0x55;
    }
    time_cast = os_tick_get() - start_time;
    os_kprintf("Write data success, total time: %d.%03dS.\r\n",
              time_cast / OS_TICK_PER_SECOND,
              time_cast % OS_TICK_PER_SECOND / ((OS_TICK_PER_SECOND * 1 + 999) / 1000));

    /* read data */
    os_kprintf("start Reading and verifying data, waiting....\r\n");
    for (i = 0; i < SRAM_SIZE; i++)
    {
        data = *(__IO uint8_t *)(SRAM_BANK_ADDR + i);
        if (data != 0x55)
        {
            os_kprintf("SRAM test failed!\r\n");
            break;
        }
    }

    if (i >= SRAM_SIZE)
    {
        os_kprintf("SRAM test success!\r\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(sram_test, sram_test, "sram test");
#endif


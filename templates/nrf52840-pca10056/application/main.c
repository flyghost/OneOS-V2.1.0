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
 * @file        main.c 
 *
 * @brief       User application entry
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include "os_task.h"
#include "os_assert.h"
#include "nrf_uart.h"
#ifdef OS_USING_LPMGR
#include "lpmgr.h"
#endif
#include "nrf_soc.h"

extern void power_test(void);
static void user_task(void *parameter)
{
    int i = 0;
	
	
    for (i = 0; i < led_table_size; i++)
    {
        os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
    }

    for (i = 0; i < led_table_size; i++)
    {
        os_pin_write(led_table[i].pin, !led_table[i].active_level);
    }
    //os_lpmgr_request(2);

    while (1)
    {
        //os_lpmgr_request(2);
        for (i = 0; i < led_table_size; i++)
        {
            os_pin_write(led_table[i].pin, led_table[i].active_level);
            os_task_msleep(500);

            os_pin_write(led_table[i].pin, !led_table[i].active_level);
            os_task_msleep(500);
        }
        //os_lpmgr_release(2);
        os_task_msleep(1000);
    }

}

int main(void)
{
    os_task_t *task;
	
    task = os_task_create("user", user_task, 0, 512, 3);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

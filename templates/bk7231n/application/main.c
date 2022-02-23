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
#include "os_stddef.h"
#include "os_assert.h"

#ifdef NET_USING_ACW
#include "acw.h"
#endif

#ifdef NET_USING_ACW_OLD
#include "acw.h"
#endif

static void user_task(void *parameter)
{
    while (1)
    {
#ifdef OS_USING_BLE
        print_hci_pkt();
        print_str_buf();
#endif
        os_task_mdelay(100);
    }
}

int main(void)
{
    os_task_t *task;

    char *date = __DATE__;
    char *time = __TIME__;
    
#if defined(BEKEN_ATE) && defined(OS_USING_CONSOLE)
    extern void ate_app_init(void);
    ate_app_init();
#endif

    os_kprintf("date:%s\ttime:%s\n", date, time);
#ifdef NET_USING_ACW
    acw_main_proc(acw_intf_soc_wifi_bk7231n);
#endif
#ifdef NET_USING_ACW_OLD
    acw_main_proc();
#endif
    task = os_task_create("user", user_task, OS_NULL, 512, 10, 5);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

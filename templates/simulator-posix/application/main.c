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
 * 2020-11-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_task.h>
#include <os_util.h>
#include <os_assert.h>

static void user_task(void *parameter)
{
    int i = 0;
    while(1)
    {
        os_kprintf("i = %d \r\n",i++);
        os_task_tsleep(10);
    }
}


int main(void)
{
    int i = 0;
    os_task_t *task;
    //os_kprintf("hello\r\n");

    while(i--)
    {
        task = os_task_create("user", user_task, 0, 16*1024, 3);
        OS_ASSERT(task);
        os_task_startup(task);
    }

    return 0;
}


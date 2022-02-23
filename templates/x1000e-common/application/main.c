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
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_stddef.h>
#include <os_task.h>
#include <stdlib.h>

static void user_task(void *parameter)
{
    int i = 0;

    while(1)
    {
        os_kprintf("%s %d\r\n",__FUNCTION__,i++);

        os_task_tsleep(rand()%10);

    }
}

int main(void)
{
    os_task_t *task;
    int task_num = 0;
    while(task_num--)
    {
        task = os_task_create("user", user_task, OS_NULL, 512, task_num%30);
        os_task_startup(task);
    }

    int i = 0;
    while(1)
    {
        os_kprintf("%s %d\r\n",__FUNCTION__,i++);
        os_task_msleep(1000);
    }

    return 0;
}

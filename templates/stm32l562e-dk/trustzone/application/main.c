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
#include <arch_tz.h>

static void user_task(void *parameter)
{
    int i = 0;

    for (i = 0; i < led_table_size; i++)
    {
        os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
    }

    while (1)
    {
        for (i = 0; i < led_table_size; i++)
        {
            os_pin_write(led_table[i].pin, led_table[i].active_level);
            os_task_msleep(500);

            os_pin_write(led_table[i].pin, !led_table[i].active_level);
            os_task_msleep(500);
        }
    }
}

static void test_task(void *parameter)
{
    while (1)
    {
        os_kprintf("#########\r\n");
        os_task_tsleep(200);
    }
}

typedef enum
{
    SECURE_FAULT_CB_ID     = 0x00U, /*!< System secure fault callback ID */
    GTZC_ERROR_CB_ID       = 0x01U, /*!< GTZC secure error callback ID */
    HARD_FAULT_CB_ID       = 0x02U  /*!< System hard fault callback ID */ 
} SECURE_CallbackIDTypeDef;

extern void SECURE_RegisterCallback(SECURE_CallbackIDTypeDef CallbackId, void *func);
extern void arch_fault_exception_nonsecure(void *stack_frame, os_size_t *msp, os_size_t *psp);

int main(void)
{
    os_task_t *task;

    os_arch_tz_context_alloc(0);
    SECURE_RegisterCallback(HARD_FAULT_CB_ID, arch_fault_exception_nonsecure);
    os_arch_tz_context_free();

    task = os_task_create("user", user_task, NULL, 512, 3);
    OS_ASSERT(task);
    os_task_startup(task);

//    os_task_t *task1;
//    task1 = os_task_create("test", test_task, NULL, 512, 3);
//    OS_ASSERT(task1);
//    os_task_startup(task1);

    return 0;
}

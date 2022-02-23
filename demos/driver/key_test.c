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
 * @file        key_test.c
 *
 * @brief       The test file for key.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_clock.h>
#include <shell.h>

static void pin_callback(void *args)
{
    os_kprintf("----------------------pin_callback:%d\r\n", (int)(unsigned long)args);
}

static int key_test(int argc, char *argv[])
{
    int i, j;

    for (i = 0; i < key_table_size; i++)
    {
        os_pin_mode(key_table[i].pin, key_table[i].mode);
        os_pin_attach_irq(key_table[i].pin, key_table[i].irq_mode, pin_callback, (void *)key_table[i].pin);
        os_pin_irq_enable(key_table[i].pin, PIN_IRQ_ENABLE);
    }

    for (j = 0; j < 10; j++)
    {
        for (i = 0; i < key_table_size; i++)
        {
            os_kprintf("<%u> pin[%d] : %d\r\n", (unsigned int)os_tick_get(), key_table[i].pin, os_pin_read(key_table[i].pin));
        }

        os_task_msleep(1000);
    }

    os_kprintf("============\r\n");

    for (i = 0; i < key_table_size; i++)
    {
        os_pin_mode(key_table[i].pin, PIN_MODE_DISABLE);
        os_kprintf("disable <%u> pin[%d] : %d\r\n", (unsigned int)os_tick_get(), key_table[i].pin, os_pin_read(key_table[i].pin));
        os_task_msleep(1000);
    }


    return 0;
}
SH_CMD_EXPORT(key_test, key_test, "key_test");

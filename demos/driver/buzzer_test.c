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
 * @file        buzzer_test.c
 *
 * @brief       The test file for buzzer.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <shell.h>
#include <string.h>

static void buzzer_init(void)
{
    int i;

    for (i = 0; i < buzzer_table_size; i++)
    {
        os_pin_mode(buzzer_table[i].pin, PIN_MODE_OUTPUT);
    }
}

static void buzzer_test(int argc, char **argv)
{
    unsigned int    buzzer_index;
    char           *on_off;
    const buzzer_t *buzzer;
    os_base_t       value;

    if (argc != 3)
    {
    help:
        os_kprintf("\r\n");
        os_kprintf("buzzer_test <index> <on,off>\r\n");
        os_kprintf("buzzer_test 0 on\r\n");
        os_kprintf("buzzer_test 0 off\r\n");
        return;
    }

    on_off = argv[2];

    /* Buzzer index */
    buzzer_index = strtol(argv[1], NULL, 0);

    if (buzzer_index >= buzzer_table_size)
    {
        goto help;
    }

    buzzer = &buzzer_table[buzzer_index];

    /* Value */
    if (strcmp("on", on_off) == 0)
    {
        value = buzzer->active_level;
    }
    else if (strcmp("off", on_off) == 0)
    {
        value = !buzzer->active_level;
    }
    else
    {
        goto help;
    }

    buzzer_init();
    os_pin_write(buzzer->pin, value);
}

SH_CMD_EXPORT(buzzer_test, buzzer_test, "buzzer_test");

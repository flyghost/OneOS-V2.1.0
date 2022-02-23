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
 * @file        rgb_test.c
 *
 * @brief       The test file for rgb.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <shell.h>
#include <string.h>

static void rgb_init(void)
{
    os_pin_mode(LED_R.pin, PIN_MODE_OUTPUT);
    os_pin_mode(LED_G.pin, PIN_MODE_OUTPUT);
    os_pin_mode(LED_B.pin, PIN_MODE_OUTPUT);
}

static void rgb_test(int argc, char **argv)
{
    char        *led_name;
    char        *on_off;
    const led_t *led;
    os_base_t    value;

    if (argc != 3)
    {
    help:
        os_kprintf("\r\n");
        os_kprintf("rgb_test red on\r\n");
        os_kprintf("rgb_test red off\r\n");
        os_kprintf("rgb_test green on\r\n");
        os_kprintf("rgb_test green off\r\n");
        os_kprintf("rgb_test blue on\r\n");
        os_kprintf("rgb_test blue off\r\n");
        return;
    }

    led_name = argv[1];
    on_off   = argv[2];

    /* Led */
    if (strcmp("red", led_name) == 0)
    {
        led = &LED_R;
    }
    else if (strcmp("green", led_name) == 0)
    {
        led = &LED_G;
    }
    else if (strcmp("blue", led_name) == 0)
    {
        led = &LED_B;
    }
    else
    {
        goto help;
    }

    /* Value */
    if (strcmp("on", on_off) == 0)
    {
        value = led->active_level;
    }
    else if (strcmp("off", on_off) == 0)
    {
        value = !led->active_level;
    }
    else
    {
        goto help;
    }

    rgb_init();
    os_pin_write(led->pin, value);
}

SH_CMD_EXPORT(rgb_test, rgb_test, "rgb_test");

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
 * @file        drv_led.c
 *
 * @brief       This file implements led driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_memory.h>
#include "am_mcu_apollo.h"

static const int am_gpio_led_table[] = APOLLO_LED_TABLE;

#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof(ar[0]))
static const int am_gpio_led_nr = ARRAY_SIZE(am_gpio_led_table);

void os_hw_led_on(os_uint8_t LEDNum)
{
#ifdef OS_USING_PIN

    if (LEDNum >= am_gpio_led_nr)
        os_kprintf("invalide led index %d, %d\r\n", LEDNum, am_gpio_led_nr);

    os_pin_write(am_gpio_led_table[LEDNum], PIN_LOW);

#endif
}

void os_hw_led_off(os_uint8_t LEDNum)
{
#ifdef OS_USING_PIN

    if (LEDNum >= am_gpio_led_nr)
        os_kprintf("invalide led index %d, %d\r\n", LEDNum, am_gpio_led_nr);

    os_pin_write(am_gpio_led_table[LEDNum], PIN_HIGH);

#endif
}

int os_hw_led_init(void)
{
#ifdef OS_USING_PIN

    int i;

    for (i = 0; i < am_gpio_led_nr; i++)
    {
        /* config led */
        os_pin_mode(am_gpio_led_table[i], PIN_MODE_OUTPUT);

        /* turns off the led */
        os_hw_led_off(i);
    }

#endif

    os_kprintf("led_init!\n");

    return 0;
}

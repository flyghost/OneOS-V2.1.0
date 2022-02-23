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
 * @file        board.c
 *
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_gpio.h>

#if defined(OS_USING_LED)
const led_t led_table[] = 
{
    {GET_PIN(B, 4), PIN_LOW},
    {GET_PIN(B, 5), PIN_LOW},
};

const int led_table_size = sizeof(led_table) / sizeof(led_table[0]);
#endif

#if defined(OS_USING_PUSH_BUTTON)
const struct push_button key_table[] = 
{
    {26, PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},/*K1*/
};

const int key_table_size = ARRAY_SIZE(key_table);

void m5311_power_on(void)
{
    os_pin_write(M5311_POWER_PIN, PIN_HIGH);
}

void m5311_power_off(void)
{
    os_pin_write(M5311_POWER_PIN, PIN_LOW);
}

static os_err_t board_pin_init(void)
{
    os_pin_mode(M5311_POWER_PIN, PIN_MODE_OUTPUT);
    
    return OS_EOK;
}
OS_ENV_INIT(board_pin_init, OS_INIT_SUBLEVEL_LOW);
#endif

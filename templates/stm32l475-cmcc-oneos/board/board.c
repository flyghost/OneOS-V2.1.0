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

#ifdef OS_USING_LED
const led_t led_table[] = 
{
    {GET_PIN(E, 7), PIN_LOW},
    {GET_PIN(E, 8), PIN_LOW},
    {GET_PIN(E, 9), PIN_LOW},
};

const int led_table_size = ARRAY_SIZE(led_table);
#endif

/****key table****
*KEY1->K1 GPIOD_PIN_10
*KEY2->K2 GPIOC_PIN_13
*KEY3->K3 GPIOD_PIN_8
*KEY4->K4 GPIOD_PIN_9
*/
#ifdef OS_USING_PUSH_BUTTON
const struct push_button key_table[] = 
{
    {GET_PIN(D, 10),     PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
    {GET_PIN(C, 13),     PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
    {GET_PIN(D, 8),      PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
    {GET_PIN(D, 9),      PIN_MODE_INPUT_PULLUP,      PIN_IRQ_MODE_FALLING},
};

const int key_table_size = ARRAY_SIZE(key_table);
#endif

#ifdef OS_USING_BUZZER

/****buzzer table****
*BUZZER -> GPIOB_PIN_2
*/
const buzzer_t buzzer_table[] =
{
    {GET_PIN(B, 2), PIN_HIGH},
};

const int buzzer_table_size = ARRAY_SIZE(buzzer_table);
#endif





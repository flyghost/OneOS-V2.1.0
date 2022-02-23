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

const led_t led_table[] = 
{
    {0x1d, PIN_LOW},
};

const int led_table_size = ARRAY_SIZE(led_table);

const struct push_button key_table[] = 
{
    {0x3f, PIN_MODE_INPUT_PULLUP, PIN_IRQ_MODE_FALLING},
};

const int key_table_size = ARRAY_SIZE(key_table);

const int board_no_pin_tab[] = 
{
    0x1f,
    0x38,
    0x39,
    0x3a,
    0x3b,
    0x3c,
    0x3d,
    0x3e,
};

const int board_no_pin_tab_size = ARRAY_SIZE(board_no_pin_tab);

const int slot_no_pin_tab[] = 
{
    0x47,
    0x48,
    0x49,
    0x4a,
    0x4b,
};

const int slot_no_pin_tab_size = ARRAY_SIZE(slot_no_pin_tab);


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
    0x13, //GET_PIN(B, 3),
    0x0f, //GET_PIN(A, 15),
    0x0c, //GET_PIN(A, 12),
    0x0b, //GET_PIN(A, 11),
    0x03, //GET_PIN(A, 3),
    0x02, //GET_PIN(A, 2),
    0x01, //GET_PIN(A, 1),
    0x00, //GET_PIN(A, 0),
};

const int board_no_pin_tab_size = ARRAY_SIZE(board_no_pin_tab);

const int slot_no_pin_tab[] = 
{
    0x10, //GET_PIN(B, 0),
    0x11, //GET_PIN(B, 1),
    0x15, //GET_PIN(B, 5),
    0x16, //GET_PIN(B, 6),
    0x17, //GET_PIN(B, 7),    
};

const int slot_no_pin_tab_size = ARRAY_SIZE(slot_no_pin_tab);


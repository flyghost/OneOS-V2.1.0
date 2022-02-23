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
 * @file        drv_gpio.h
 *
 * @brief       This file provides struct/macro declaration and functions declaration for STM32 gpio driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <drv_common.h>
#include "board.h"

enum GPIO_PORT_INDEX
{
    GPIO_INDEX_A = 0,
    GPIO_INDEX_B,
    GPIO_INDEX_C,
    GPIO_INDEX_D,
    GPIO_INDEX_E,
    GPIO_INDEX_F,
    GPIO_INDEX_G,
    GPIO_INDEX_H,
    GPIO_INDEX_I,
    GPIO_INDEX_J,
    GPIO_INDEX_K,
    GPIO_INDEX_L,
    GPIO_INDEX_M,
    GPIO_INDEX_N,
    GPIO_INDEX_O,
    GPIO_INDEX_P,
    GPIO_INDEX_Q,
    GPIO_INDEX_R,
    GPIO_INDEX_S,
    GPIO_INDEX_T,
    GPIO_INDEX_U,
    GPIO_INDEX_V,
    GPIO_INDEX_W,
    GPIO_INDEX_X,
    GPIO_INDEX_Y,
    GPIO_INDEX_Z,
    GPIO_INDEX_MAX
};

#define GPIO_PIN_PER_PORT           (16)
#define GPIO_PORT_MAX               (GPIO_INDEX_MAX)
#define GPIO_PIN_MAX                (GPIO_PORT_MAX * GPIO_PIN_PER_PORT)

#define __PORT_INDEX(pin)           (pin / GPIO_PIN_PER_PORT)
#define __PIN_INDEX(pin)            (pin % GPIO_PIN_PER_PORT)

#define GET_PIN(PORTx, PIN)         (((GPIO_INDEX_##PORTx - GPIO_INDEX_A) * GPIO_PIN_PER_PORT) + PIN)
#define PIN_BASE(__pin)             get_pin_base(__pin)
#define PIN_OFFSET(__pin)           (1 << ((__pin) % GPIO_PIN_PER_PORT))

struct pin_pull_state
{
    os_int8_t pull_state;
};

struct pin_index
{
    GPIO_TypeDef *gpio_port;
    struct pin_pull_state pin_pulls[GPIO_PIN_PER_PORT];
};

struct pin_irq_map
{
    os_uint16_t pinbit;
    IRQn_Type   irqno;
};

GPIO_TypeDef *get_pin_base(os_base_t pin);
int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */

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
 * @brief       This file provides struct/macro declaration and functions declaration for htxxx gpio driver.
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
#include "fm33g0xx_lib.h"

enum GPIO_PORT_INDEX
{
    GPIO_INDEX_A = 0,
    GPIO_INDEX_B,
    GPIO_INDEX_C,
    GPIO_INDEX_D,
    GPIO_INDEX_E,
    GPIO_INDEX_F,
    GPIO_INDEX_G,
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
typedef void (*exti_fun)(GPIO_COMMON_Type *GPIOx, uint32_t EXTIPin);
struct pin_pull_state
{
    os_int8_t pull_state;
};

struct pin_index
{
    GPIOx_Type *gpio_port;
    struct pin_pull_state pin_pulls[GPIO_PIN_PER_PORT];
    os_uint8_t handler_mask;
};

struct pin_irq_param
{
    os_int32_t      pin;
    void          (*hdr)(void *args);
    void           *args;
    os_uint16_t     mode;
    os_uint8_t      irq_ref;

    os_list_node_t  list;
};


GPIOx_Type *get_pin_base(os_base_t pin);
int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */

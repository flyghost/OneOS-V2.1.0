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
 * @brief       This file provides struct/macro declaration and functions declaration for hc32 gpio driver.
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
#include "hc_sysctrl.h"
#include "hc_gpio.h"

#if defined(SERIES_HC32L17) || defined(SERIES_HC32L19) || defined(SERIES_HC32L07) || defined(SERIES_HC32F07) \
    || defined(SERIES_HC32F17) || defined(SERIES_HC32F19)

#define GPIOA (0)
#define GPIOB (1)
#define GPIOC (2)
#define GPIOD (3)
#define GPIOE (4)
#define GPIOF (5)

#else

#error "please add surpport for SOC..."

#endif

enum GPIO_PORT_INDEX
{
    GPIO_INDEX_A = 0,
    GPIO_INDEX_B,
    GPIO_INDEX_C,
    GPIO_INDEX_D,
    GPIO_INDEX_E,
    GPIO_INDEX_F,
    /*
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
    */
    GPIO_INDEX_MAX
};

#define GPIO_PIN_PER_PORT           (16)
#define GPIO_PORT_MAX               (GPIO_INDEX_MAX)
#define GPIO_PIN_MAX                (GPIO_PORT_MAX * GPIO_PIN_PER_PORT)

#define __PORT_INDEX(pin)           (pin / GPIO_PIN_PER_PORT)
#define __PIN_INDEX(pin)            (pin % GPIO_PIN_PER_PORT)

#define GET_PIN(PORTx, PIN)         (((GPIO_INDEX_##PORTx - GPIO_INDEX_A) * GPIO_PIN_PER_PORT) + PIN)
#define PIN_BASE(__pin)             get_pin_base(__pin)
#define PIN_OFFSET(__pin)           (en_gpio_pin_t)((__pin) % GPIO_PIN_PER_PORT)

#define PIN_PULL_STATE               0
#if (PIN_PULL_STATE)
struct pin_pull_state
{
    en_gpio_pu_t pull_up;
    en_gpio_pd_t pull_down;
};
#endif
struct pin_index
{
    en_gpio_port_t gpio_port;
#if (PIN_PULL_STATE)
    struct pin_pull_state pin_pulls[GPIO_PIN_PER_PORT];
#endif
};

struct pin_irq_map
{
    os_uint16_t pinbit;
    IRQn_Type   irqno;
};

struct pin_irq_param
{
    os_int32_t    pin;
    void (*hdr)(void *args);
    void *args;
    os_uint16_t mode;
    os_uint8_t	irq_ref;

    os_list_node_t list;
};

struct HC32_IRQ_STAT {
    IRQn_Type	irq;
    os_uint32_t ref;
};

en_gpio_port_t get_pin_base(os_base_t pin);
int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */

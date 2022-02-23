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
 * @brief       This file provides struct/macro declaration and functions declaration for GD32 gpio driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <os_types.h>
#include "gd32vf103.h"

#define __GD32_PORT(port)   GPIO##port
#define GET_PIN(PORTx, PIN) (os_base_t)((16 * (((os_base_t)__GD32_PORT(PORTx) - (os_base_t)GPIOA) / (0x0400UL))) + PIN)

#define __GD32_PIN(index, gpio, gpio_index)                                \
{                                                                       \
    index,RCU_GPIO##gpio,GPIO##gpio, GPIO_PIN_##gpio_index,             \
    GPIO_PORT_SOURCE_GPIO##gpio, GPIO_PIN_SOURCE_##gpio_index           \
}

/* GD32 GPIO driver */
struct pin_index
{
    os_int32_t      index;
    rcu_periph_enum clk;
    os_uint32_t     gpio_port;
    os_uint32_t     pin;
    os_uint32_t     port_src;
    os_uint32_t     pin_src;
};

struct pin_irq_map
{
    os_uint16_t pinbit;
    IRQn_Type   irqno;
};

const struct pin_index *get_pin(os_uint8_t pin);
int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */

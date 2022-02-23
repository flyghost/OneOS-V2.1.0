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
#include "ingenic_gpio.h"
#include <pin/pin.h>

#define __GET_PIN(PORTx, PIN)                                                                                            \
    (os_base_t)((32 * ((os_base_t)(PORTx) - (os_base_t)GPIO_PORT_A)) + PIN)

#define GET_PIN(PORTx, PIN)                                                                                            \
    (os_base_t)((32 * ((os_base_t)(GPIO_PORT_##PORTx) - (os_base_t)GPIO_PORT_A)) + PIN)

#define __INDEX_PIN(index, gpio, gpio_index)                                                                           \
{                                                                                                                  \
    index, GPIO_PORT_##gpio, gpio_index, OS_NULL, OS_NULL                                                                       \
}

struct pin_index
{
    os_int32_t    index;
    enum gpio_port port;
    os_uint32_t pin;
    void (*hdr)(void *args);
    void *args;
    os_uint16_t mode;
    os_uint8_t	irq_ref;
};

struct INGENIC_IRQ_STAT{
    unsigned int irq;
    os_uint32_t ref;
};

struct pin_pull_state
{
    os_int8_t pd;
    os_int8_t pu;
};
#if 0
struct pin_irq_map
{
    os_uint16_t pinbit;
    IRQn_Type   irqno;
};
#endif

#endif /* __DRV_GPIO_H__ */

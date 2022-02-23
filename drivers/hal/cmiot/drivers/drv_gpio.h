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
 * @brief       This file provides struct/macro declaration and functions declaration for cm32 gpio driver.
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
#include "cm32m101a_gpio.h"

#define	GPIOPORTA (0)
#define	GPIOPORTB (1)
#define	GPIOPORTC (2)
#define	GPIOPORTD (3)

#define __CM32_PORT(port)   GPIOPORT##port
#define GET_PIN(PORTx, PIN)                                                                                            \
    (os_base_t)((16 * ((os_base_t)__CM32_PORT(PORTx) - (os_base_t)GPIOPORTA)) + PIN)

#define __INDEX_PIN(index, gpio, gpio_index)                                                                           \
    {                                                                                                                  \
        index, GPIO##gpio, GPIO_PIN_##gpio_index, OS_NULL, OS_NULL, 0, 0                                               \
    }

/* CM32 GPIO driver */
struct pin_index
{
    os_int32_t    index;
    GPIO_Module*   port;
    uint16_t   pin;
    void (*hdr)(void *args);
    void *args;
    os_uint16_t mode;
    os_uint8_t	irq_ref;
    GPIO_PuPdType pull_state;
};

//struct pin_pull_state
//{
//    GPIO_PuPdType pull_state;
//};

struct pin_irq_map
{
    IRQn_Type   irqno;
    uint8_t     pinsource;
};

int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */

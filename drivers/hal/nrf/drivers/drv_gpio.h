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
 * @brief       This file provides struct/macro declaration and functions declaration for nrf gpio driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <os_types.h>
#include "pin.h"
#include <drv_common.h>
//#include <board.h>
#include "nrf_gpio.h"
#include "nrfx_gpiote.h"


#define __NRF5X_PORT(port)  NRF_P##port##_BASE

#define GET_PIN(PORTx,PIN) (rt_base_t)((32 * ( ((rt_base_t)__NRF5X_PORT(PORTx) - (rt_base_t)NRF_P0_BASE)/(0x0300UL) )) + PIN)

#define __NRF5X_PIN(index, gpio, gpio_index)                                \
    {                                                                       \
        index, NRF_P##gpio, gpio_index                                      \
    }

#define __NRF5X_PIN_RESERVE                                                 \
    {                                                                       \
        -1, 0, 0                                                            \
    }


/* NRF5 GPIO driver */
struct pin_index
{
    int index;
    NRF_GPIO_Type *gpio;//NRF_P0 or NRF_P1
    uint32_t pin;
};


const struct pin_index *get_pin(os_uint8_t pin);
int os_hw_pin_init(void);

#endif /* __DRV_GPIO_H__ */

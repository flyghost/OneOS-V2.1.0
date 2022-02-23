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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_i2c.h"
#include "drv_gpio.h"

#ifdef BSP_USING_I2C0
static const struct fm33_i2c_info hard_i2c0_info =
{
    .inst = I2C,
    .scl  = 
        {
            .port  = GPIOB,
            .gpio  = FL_GPIO_PIN_15,
            .pin   = GET_PIN(B,15),
        },
    .sda  = 
        {
            .port  = GPIOD,
            .gpio  = FL_GPIO_PIN_12,
            .pin   = GET_PIN(D,12),
        },
    .cfg  = 
        {
            .clockSource = FL_CMU_I2C_CLK_SOURCE_APBCLK,
            .baudRate    = 100000,
        },
};
OS_HAL_DEVICE_DEFINE("I2C_Type", "hard_i2c0", hard_i2c0_info);
#endif


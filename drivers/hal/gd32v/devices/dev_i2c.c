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
 * @file        dev_i2c.c
 *
 * @brief       This file implements usart driver for gd32v
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifdef BSP_USING_I2C0
struct gd_i2c_info i2c0_info = {.hi2c = I2C0, .clk_speed = 400000, .port = GPIOB, .pin = GPIO_PIN_6 | GPIO_PIN_7, .periph = RCU_GPIOB};
OS_HAL_DEVICE_DEFINE("I2C_Type", "i2c0", i2c0_info);
#endif

#ifdef BSP_USING_I2C1
struct gd_i2c_info i2c1_info = {.hi2c = I2C1, .clk_speed = 400000, .port = GPIOB, .pin = GPIO_PIN_10 | GPIO_PIN_11, .periph = RCU_GPIOB};
OS_HAL_DEVICE_DEFINE("I2C_Type", "i2c1", i2c1_info);
#endif

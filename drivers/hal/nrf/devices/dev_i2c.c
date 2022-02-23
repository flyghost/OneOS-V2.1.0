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
 * @file        drv_i2c.c
 *
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_i2c.h"

#define EVAL_I2C0                        NRFX_TWIM_INSTANCE(0)
#define EVAL_I2C1                        NRFX_TWIM_INSTANCE(1)

#ifdef NRF52832_XXAA
#define EVAL_I2C0_SCL_PIN                27
#define EVAL_I2C0_SDA_PIN                26
#elif NRF52840_XXAA
#define EVAL_I2C0_SCL_PIN                27
#define EVAL_I2C0_SDA_PIN                26
#define EVAL_I2C1_SCL_PIN                28
#define EVAL_I2C1_SDA_PIN                29
#endif

#ifdef BSP_USING_I2C0
struct nrf5_i2c_info i2c0_info = {.hi2c = EVAL_I2C0, .scl_pin = EVAL_I2C0_SCL_PIN, .sda_pin = EVAL_I2C0_SDA_PIN 
                                };
OS_HAL_DEVICE_DEFINE("I2C_Type", "i2c0", i2c0_info);
#endif

#ifdef BSP_USING_I2C1
struct nrf5_i2c_info i2c1_info = {.hi2c = EVAL_I2C1, .scl_pin = EVAL_I2C1_SCL_PIN, .sda_pin = EVAL_I2C1_SDA_PIN
                                };
OS_HAL_DEVICE_DEFINE("I2C_Type", "i2c1", i2c1_info);
#endif

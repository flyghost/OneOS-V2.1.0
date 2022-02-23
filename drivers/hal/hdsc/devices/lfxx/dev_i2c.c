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
 * @brief       This file implements i2c driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_i2c.h"

#ifdef BSP_USING_I2C0
struct hc32_i2c_info i2c0_info = {
    .i2c_base = M0P_I2C0,
    .mode = I2cMasterMode,
    .slave_addr = 0,
    .scl_port = I2C0_SCL_PORT,
    .scl_pin = I2C0_SCL_PIN,
    .sda_port = I2C0_SDA_PORT,
    .sda_pin = I2C0_SDA_PIN,
    .peripheral = SysctrlPeripheralI2c0,
    .gpio_af = I2C0_GPIO_AF,
};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "i2c0", i2c0_info);
#endif

#ifdef BSP_USING_I2C1
struct hc32_i2c_info i2c1_info = {
    .i2c_base = M0P_I2C1,
    .mode = I2cMasterMode,
    .slave_addr = 0,
    .scl_port = I2C1_SCL_PORT,
    .scl_pin = I2C1_SCL_PIN,
    .sda_port = I2C1_SDA_PORT,
    .sda_pin = I2C1_SDA_PIN,
    .peripheral = SysctrlPeripheralI2c1,
    .gpio_af = I2C1_GPIO_AF,
};
OS_HAL_DEVICE_DEFINE("I2C_HandleTypeDef", "i2c1", i2c1_info);
#endif

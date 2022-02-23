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
 * @file        soft_i2c_bus_config.c
 *
 * @brief       This file implements soft I2C for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <i2c/soft_i2c_bus.h>

#define SOFT_I2C_BUS_DEFINE(dev_name, scl_pin, sda_pin)     \
                                                            \
static struct soft_i2c_config soft_i2c_bus_##dev_name = {   \
    .scl = scl_pin,                                         \
    .sda = sda_pin,                                         \
};                                                          \
                                                            \
OS_DEVICE_INFO soft_i2c_bus_##dev_name##_info = {           \
    .name   = #dev_name,                                    \
    .driver = "soft_i2c_bus",                               \
    .info   = &soft_i2c_bus_##dev_name,                     \
}

#ifdef BSP_USING_SOFT_I2C1
SOFT_I2C_BUS_DEFINE(soft_i2c1, BSP_SOFT_I2C1_SCL_PIN, BSP_SOFT_I2C1_SDA_PIN);
#endif

#ifdef BSP_USING_SOFT_I2C2
SOFT_I2C_BUS_DEFINE(soft_i2c2, BSP_SOFT_I2C2_SCL_PIN, BSP_SOFT_I2C2_SDA_PIN);
#endif

#ifdef BSP_USING_SOFT_I2C3
SOFT_I2C_BUS_DEFINE(soft_i2c3, BSP_SOFT_I2C3_SCL_PIN, BSP_SOFT_I2C3_SDA_PIN);
#endif

#ifdef BSP_USING_SOFT_I2C4
SOFT_I2C_BUS_DEFINE(soft_i2c4, BSP_SOFT_I2C4_SCL_PIN, BSP_SOFT_I2C4_SDA_PIN);
#endif


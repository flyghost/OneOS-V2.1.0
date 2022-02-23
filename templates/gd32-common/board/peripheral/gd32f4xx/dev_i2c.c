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

#ifdef BSP_USING_I2C0
struct gd_i2c_info i2c0_info = {.hi2c = EVAL_I2C0, .i2c_rcu_clk = EVAL_I2C0_RCU_CLK, .i2c_speed = EVAL_I2C0_SPEED
                                , .scl_pin = EVAL_I2C0_SCL_PIN, .sda_pin = EVAL_I2C0_SDA_PIN 
                                , .pin_port = EVAL_I2C0_GPIO_PORT, .port_rcu_clk = EVAL_I2C0_PORT_RCU_CLK
                                };
OS_HAL_DEVICE_DEFINE("I2C_Type", "i2c0", i2c0_info);
#endif

#ifdef BSP_USING_I2C1
struct gd_i2c_info i2c1_info = {.hi2c = EVAL_I2C1, .i2c_rcu_clk = EVAL_I2C1_RCU_CLK, .i2c_speed = EVAL_I2C1_SPEED
                                , .scl_pin = EVAL_I2C1_SCL_PIN, .sda_pin = EVAL_I2C1_SDA_PIN
                                , .pin_port = EVAL_I2C1_GPIO_PORT, .port_rcu_clk = EVAL_I2C1_PORT_RCU_CLK
                                };
OS_HAL_DEVICE_DEFINE("I2C_Type", "i2c1", i2c1_info);
#endif

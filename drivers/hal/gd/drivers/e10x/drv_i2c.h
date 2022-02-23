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
 * @file        drv_i2c.h
 *
 * @brief       This file implements i2c driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRVI2C_H__
#define DRVI2C_H__

#include <os_device.h>
#include "gd32f30x.h"

#define EVAL_I2C0                        I2C0
#define EVAL_I2C0_RCU_CLK                RCU_I2C0
#define EVAL_I2C0_SPEED                  400000
#define EVAL_I2C0_SCL_PIN                GPIO_PIN_6
#define EVAL_I2C0_SDA_PIN                GPIO_PIN_7
#define EVAL_I2C0_GPIO_PORT              GPIOB
#define EVAL_I2C0_PORT_RCU_CLK           RCU_GPIOB

#define EVAL_I2C1                        I2C1
#define EVAL_I2C1_RCU_CLK                RCU_I2C1
#define EVAL_I2C1_SPEED                  400000
#define EVAL_I2C1_SCL_PIN                GPIO_PIN_10
#define EVAL_I2C1_SDA_PIN                GPIO_PIN_11
#define EVAL_I2C1_GPIO_PORT              GPIOB
#define EVAL_I2C1_PORT_RCU_CLK           RCU_GPIOB

struct gd_i2c_info {
    os_uint32_t hi2c;
    os_uint32_t i2c_rcu_clk;
    os_uint32_t i2c_speed;
    os_uint32_t scl_pin;
    os_uint32_t sda_pin;
    os_uint32_t pin_port;
    os_uint32_t port_rcu_clk;
};

#endif


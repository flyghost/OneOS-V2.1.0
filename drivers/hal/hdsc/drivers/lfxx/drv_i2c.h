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
 * @brief       This file provides functions declaration for i2c.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_I2C_H_
#define __DRV_I2C_H_

#include "hc_i2c.h"
#include "hc_gpio.h"

struct hc32_i2c_info
{
    M0P_I2C_TypeDef *i2c_base;
    en_i2c_mode_t mode;
    uint8_t slave_addr;
    en_gpio_port_t scl_port;
    en_gpio_pin_t scl_pin;
    en_gpio_port_t sda_port;
    en_gpio_pin_t sda_pin;
    en_gpio_af_t gpio_af;
    en_sysctrl_peripheral_gate_t peripheral;
};

#endif

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
 * @brief       This file implements i2c driver for mm32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _DRV_I2C_H_
#define _DRV_I2C_H_

#include <device.h>
#include "types.h"

struct mm32_i2c_info 
{
    void * hi2c;
    os_uint32_t i2c_clk;
    os_uint32_t speed;
    os_uint32_t scl_pin;
    os_uint32_t scl_pin_source;
    os_uint32_t sda_pin;
    os_uint32_t sda_pin_source;
    void *      pin_port;
    os_uint32_t pin_clk;
    os_uint32_t gpio_af_idx;
    void(*rcc_init_func)(os_uint32_t x, FunctionalState y);
};

#endif


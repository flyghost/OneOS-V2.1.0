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

#include <i2c.h>
#include "cm32m101a_i2c.h"
#include "cm32m101a_gpio.h"

struct cm32_i2c
{
    I2C_Module *i2c_base;
    uint8_t slave_addr;

    GPIO_Module *scl_port;
    uint16_t scl_pin;
    uint32_t af_i2c_scl;
    GPIO_Module *sda_port;
    uint16_t sda_pin;
    uint32_t af_i2c_sda;

    struct os_i2c_bus_device parent;
    const char *name;
};


int os_hw_i2c_init(void);

#endif


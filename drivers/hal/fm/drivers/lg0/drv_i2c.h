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
 * @brief       This file implements i2c driver configuration for fm33.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#include <device.h>
#include <board.h>

struct fm33_i2c_pin_info
{
    GPIO_Type *          port;
    os_uint32_t          gpio;
    os_uint32_t          pin;
};

struct fm33_i2c_info
{
    I2C_Type                       *inst;
    struct fm33_i2c_pin_info        scl;
    struct fm33_i2c_pin_info        sda;
    FL_I2C_MasterMode_InitTypeDef   cfg;
};

struct fm33_i2c
{
    struct os_i2c_bus_device  i2c;
    struct fm33_i2c_info     *info;

    os_sem_t                  i2c_sem;
    
    os_list_node_t            list;
};

#endif


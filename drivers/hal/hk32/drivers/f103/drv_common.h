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
 * @file        drv_common.h
 *
 * @brief       This file provides _Error_Handler() declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <board.h>
#include <arch_interrupt.h>
#include <device.h>
#include <driver.h>
#include <board.h>
#include <os_task.h>

#ifdef OS_USING_PIN
#include "drv_gpio.h"
#endif

enum hk32_rcc_type
{
    HK32_RCC_AHB,
    HK32_RCC_APB1,
    HK32_RCC_APB2,
};

#ifdef HAL_I2C_MODULE_ENABLED
struct stm32_i2c_info {
    I2C_HandleTypeDef *instance;
    os_uint16_t scl;
    os_uint16_t sda;
};
#endif

#endif

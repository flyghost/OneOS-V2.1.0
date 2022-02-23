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
 * @file        drv_spi.h
 *
 * @brief       This file implements spi driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include <board.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <drv_gpio.h>
#include <drv_common.h>
#include <string.h>
#include <drv_log.h>
#include "hc_spi.h"
#include "hc_reset.h"

struct hc32_hw_spi_cs
{
    en_gpio_port_t GPIOx;
    en_gpio_pin_t  GPIO_Pin;
};

struct hc32_spi_info
{
    M0P_SPI_TypeDef *spi_base;
    en_reset_peripheral0_t reset;
    en_sysctrl_peripheral_gate_t peripheral;
    en_gpio_port_t port_cs;
    en_gpio_pin_t pin_cs;
    en_gpio_port_t port_sck;
    en_gpio_pin_t pin_sck;
    en_gpio_port_t port_miso;
    en_gpio_pin_t pin_miso;
    en_gpio_port_t port_mosi;
    en_gpio_pin_t pin_mosi;
    en_gpio_af_t gpio_af;
    const char *name;
};

#endif /*__DRV_SPI_H_ */

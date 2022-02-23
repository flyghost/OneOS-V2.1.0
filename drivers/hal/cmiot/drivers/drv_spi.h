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
 * @brief       This file implements spi driver for cm32.
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
#include "cm32m101a_spi.h"

struct cm32_hw_spi_cs
{
    GPIO_Module* GPIOx;
    uint16_t  GPIO_Pin;
    uint32_t cs_pin_ref;
};

struct cm32_spi_config
{
    char        *bus_name;
};

struct cm32_spi
{
    struct os_spi_bus spi_bus;

    SPI_Module* spi_base;
    uint32_t  cs_af;
    GPIO_Module* port_sck;
    uint16_t pin_sck;
    uint32_t  sck_af;
    GPIO_Module* port_miso;
    uint16_t pin_miso;
    uint32_t  miso_af;
    GPIO_Module* port_mosi;
    uint16_t pin_mosi;
    uint32_t  mosi_af;

    struct os_spi_configuration *cfg;
    const char *name;
};

#endif /*__DRV_SPI_H_ */

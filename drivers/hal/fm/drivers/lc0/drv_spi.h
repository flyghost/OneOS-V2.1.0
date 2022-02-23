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
 * @brief       This file implements SPI driver for fm33.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include <board.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <drv_common.h>
#include <string.h>
#include <dlog.h>
#include <drv_gpio.h>


struct fm33_spi_info
{
    SPI_Type            *instance;
    os_uint16_t          index;

    GPIO_Type           *port;
    FL_GPIO_InitTypeDef  gpio_init_struct;
};

struct fm33_spi
{
    struct os_spi_bus            spi_bus;

    struct fm33_spi_info        *info;
    struct os_spi_configuration *cfg;

    os_list_node_t               list;
};

#define DBG_TAG "drv.spi"

#endif /*__DRV_SPI_H_ */

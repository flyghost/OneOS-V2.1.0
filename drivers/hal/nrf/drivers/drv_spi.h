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
 * @brief       This file provides operation functions declaration for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#include "nrfx_spi.h"
#include "nrf_drv_spi.h"

struct nrf5_spi_config
{
    char        *bus_name;
    struct dma_config *dma_rx, *dma_tx;
};

struct nrf5_spi_info {
    nrfx_spi_t spi_dev;
    os_uint8_t miso_pin;
    os_uint8_t mosi_pin;
    os_uint8_t sck_pin;
};

#endif  /*__DRV_SPI_H_*/

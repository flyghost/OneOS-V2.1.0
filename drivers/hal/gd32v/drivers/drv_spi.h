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
 * @brief       This file implements SPI driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_SPI_H_
#define __DRV_SPI_H_

#define DUMMY_BYTE       0xA5
#define SPI_USING_RX_DMA_FLAG (1 << 0)
#define SPI_USING_TX_DMA_FLAG (1 << 1)

struct gd32_spi_config
{
    char        *bus_name;
    struct dma_config *dma_rx, *dma_tx;
};

struct gd32_spi_device
{
    os_uint32_t pin;
    char *bus_name;
    char *device_name;
};

struct gd32_spi
{
    struct os_spi_bus spi_bus;

    struct gd_spi_info *spi_info;
    struct gd32_spi_config     *config;
    struct os_spi_configuration *cfg;

    os_uint8_t     spi_dma_flag;
    os_list_node_t list;
};

struct gd_spi_info {
    os_uint32_t hspi;
    os_uint32_t rcu_spi_gpio_base;
    os_uint32_t rcu_spi_base;
    os_uint32_t spi_pin_base;
    os_uint32_t spi_miso_pin;
    os_uint32_t spi_mosi_pin;
    os_uint32_t spi_sck_pin;
};

#endif /*__DRV_SPI_H_ */

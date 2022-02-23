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
 * @file        dev_spi.c
 *
 * @brief       This file implements spi driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_spi.h"

#ifdef BSP_USING_SPI0
struct hc32_spi_info spi0_info = {
    .spi_base = M0P_SPI0,
    .reset = ResetMskSpi0,
    .peripheral = SysctrlPeripheralSpi0,
    .port_cs = SPI0_CS_PORT,
    .pin_cs = SPI0_CS_PIN,
    .port_sck = SPI0_SCK_PORT,
    .pin_sck = SPI0_SCK_PIN,
    .port_miso = SPI0_MISO_PORT,
    .pin_miso = SPI0_MISO_PIN,
    .port_mosi = SPI0_MOSI_PORT,
    .pin_mosi = SPI0_MOSI_PIN,
    .gpio_af = SPI0_GPIO_AF,
    .name = "spi0",
};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi0", spi0_info);
#endif

#ifdef BSP_USING_SPI1
struct hc32_spi_info spi1_info = {
    .spi_base = M0P_SPI1,
    .reset = ResetMskSpi1,
    .peripheral = SysctrlPeripheralSpi1,
    .port_cs = SPI1_CS_PORT,
    .pin_cs = SPI1_CS_PIN,
    .port_sck = SPI1_SCK_PORT,
    .pin_sck = SPI1_SCK_PIN,
    .port_miso = SPI1_MISO_PORT,
    .pin_miso = SPI1_MISO_PIN,
    .port_mosi = SPI1_MOSI_PORT,
    .pin_mosi = SPI1_MOSI_PIN,
    .gpio_af = SPI1_GPIO_AF,
    .name = "spi1",
};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", spi1_info);
#endif

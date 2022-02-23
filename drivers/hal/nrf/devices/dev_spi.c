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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for nrf5
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <drv_spi.h>

#ifdef NRF52832_XXAA
#define BSP_SPI1          NRF_SPI1
#define BSP_SPI1_SCK_PIN  27
#define BSP_SPI1_MOSI_PIN 26
#define BSP_SPI1_MISO_PIN 7

#elif NRF52840_XXAA
#define BSP_SPI1          NRF_SPI1
#define BSP_SPI1_SCK_PIN  19
#define BSP_SPI1_MISO_PIN 21
#define BSP_SPI1_MOSI_PIN 20
#endif

#ifdef BSP_USING_SPI1
struct nrf5_spi_info spi1_info = {NRFX_SPI_INSTANCE(1),BSP_SPI1_MISO_PIN,BSP_SPI1_MOSI_PIN,BSP_SPI1_SCK_PIN};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", spi1_info);
#endif


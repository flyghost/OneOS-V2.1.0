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
 * @file        dev_qspi.c
 *
 * @brief       This file implements usart driver for nrf5
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <drv_qspi.h>

#ifdef NRF52832_XXAA
#define BSP_QSPI_SCK_PIN  -1
#define BSP_QSPI_CS_PIN   -1
#define BSP_QSPI_IO0_PIN  -1
#define BSP_QSPI_IO1_PIN  -1
#define BSP_QSPI_IO2_PIN  -1
#define BSP_QSPI_IO3_PIN  -1
#elif NRF52840_XXAA
#define BSP_QSPI_SCK_PIN  19
#define BSP_QSPI_CS_PIN   17
#define BSP_QSPI_IO0_PIN  20
#define BSP_QSPI_IO1_PIN  21
#define BSP_QSPI_IO2_PIN  22
#define BSP_QSPI_IO3_PIN  23
#endif

struct nrf5_qspi_info qspi_info = {BSP_SPI0_MISO_PIN,BSP_SPI0_MOSI_PIN,BSP_SPI0_SCK_PIN,BSP_SPI0_SS_PIN};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "qspi", qspi_info);



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
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_spi.h>

#ifdef BSP_USING_SPI0
struct gd_spi_info spi0_info = {SPI0,RCU_GPIOA,RCU_SPI0,GPIOA,GPIO_PIN_6,GPIO_PIN_7,GPIO_PIN_5};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi0", spi0_info);
#endif

#ifdef BSP_USING_SPI1
struct gd_spi_info spi1_info = {SPI1,RCU_GPIOB,RCU_SPI1,GPIOB,GPIO_PIN_14,GPIO_PIN_15,GPIO_PIN_13};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", spi1_info);
#endif


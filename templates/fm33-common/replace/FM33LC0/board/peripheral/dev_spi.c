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
 * @file        dev_usart.c
 *
 * @brief       This file define the information of spi device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_spi.h"

#ifdef BSP_USING_SPI1
struct fm33_spi_info spi1_info = {
    .instance        = SPI1,
    .index           = 1,

    .port            = GPIOB,
    .gpio_init_struct = {
        .pin         = FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10 | FL_GPIO_PIN_11,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_DISABLE,
     },
};
OS_HAL_DEVICE_DEFINE("SPI_Type", "spi1", spi1_info);
#endif

#ifdef BSP_USING_SPI2
struct fm33_spi_info spi2_info = {
    .instance        = SPI2,
    .index           = 2,

    .port            = GPIOC,
    .gpio_init_struct = {
        .pin         = FL_GPIO_PIN_7 | FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_DISABLE,
     },
};
OS_HAL_DEVICE_DEFINE("SPI_Type", "spi2", spi2_info);
#endif



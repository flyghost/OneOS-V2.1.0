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
#ifdef BSP_USING_SPI
#include <board.h>
#include <drv_spi.h>

#ifdef BSP_USING_SPI0
struct gd_spi_info spi0_info = {EVAL_SPI0,EVAL_SPI0_GPIO_CLK,EVAL_SPI0_CLK,EVAL_SPI0_GPIO_PORT,EVAL_SPI0_MISO_PIN,
                                    EVAL_SPI0_MOSI_PIN,EVAL_SPI0_SCK_PIN,EVAL_SPI0_GPIO_AF_IDX};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi0", spi0_info);
#endif

#ifdef BSP_USING_SPI1
struct gd_spi_info spi1_info = {EVAL_SPI1,EVAL_SPI1_GPIO_CLK,EVAL_SPI1_CLK,EVAL_SPI1_GPIO_PORT,EVAL_SPI1_MISO_PIN,
                                    EVAL_SPI1_MOSI_PIN,EVAL_SPI1_SCK_PIN,EVAL_SPI1_GPIO_AF_IDX};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi1", spi1_info);
#endif

#ifdef BSP_USING_SPI2
struct gd_spi_info spi2_info = {EVAL_SPI2,EVAL_SPI2_GPIO_CLK,EVAL_SPI2_CLK,EVAL_SPI2_GPIO_PORT,EVAL_SPI2_MISO_PIN,
                                    EVAL_SPI2_MOSI_PIN,EVAL_SPI2_SCK_PIN,EVAL_SPI2_GPIO_AF_IDX};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi2", spi2_info);
#endif

#ifdef BSP_USING_SPI3
struct gd_spi_info spi3_info = {EVAL_SPI3,EVAL_SPI3_GPIO_CLK,EVAL_SPI3_CLK,EVAL_SPI3_GPIO_PORT,EVAL_SPI3_MISO_PIN,
                                    EVAL_SPI3_MOSI_PIN,EVAL_SPI3_SCK_PIN,EVAL_SPI3_GPIO_AF_IDX};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi3", spi3_info);
#endif

#ifdef BSP_USING_SPI4
struct gd_spi_info spi4_info = {EVAL_SPI4,EVAL_SPI4_GPIO_CLK,EVAL_SPI4_CLK,EVAL_SPI4_GPIO_PORT,EVAL_SPI4_MISO_PIN,
                                    EVAL_SPI4_MOSI_PIN,EVAL_SPI4_SCK_PIN,EVAL_SPI4_GPIO_AF_IDX};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi4", spi4_info);
#endif

#ifdef BSP_USING_SPI5
struct gd_spi_info spi5_info = {EVAL_SPI5,EVAL_SPI5_GPIO_CLK,EVAL_SPI5_CLK,EVAL_SPI5_GPIO_PORT,EVAL_SPI5_MISO_PIN,
                                    EVAL_SPI5_MOSI_PIN,EVAL_SPI5_SCK_PIN,EVAL_SPI5_GPIO_AF_IDX};
OS_HAL_DEVICE_DEFINE("SPI_HandleTypeDef", "spi5", spi5_info);
#endif
#endif

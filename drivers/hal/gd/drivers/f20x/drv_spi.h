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

#define EVAL_SPI0                        SPI0
#define EVAL_SPI0_GPIO_CLK               RCU_GPIOA
#define EVAL_SPI0_CLK                    RCU_SPI0
#define EVAL_SPI0_GPIO_PORT              GPIOA
#define EVAL_SPI0_MISO_PIN               GPIO_PIN_6
#define EVAL_SPI0_MOSI_PIN               GPIO_PIN_7
#define EVAL_SPI0_SCK_PIN                GPIO_PIN_5
#define EVAL_SPI0_GPIO_AF_IDX            GPIO_AF_5

#define EVAL_SPI1                        SPI1
#define EVAL_SPI1_GPIO_CLK               RCU_GPIOB
#define EVAL_SPI1_CLK                    RCU_SPI1
#define EVAL_SPI1_GPIO_PORT              GPIOB
#define EVAL_SPI1_MISO_PIN               GPIO_PIN_14
#define EVAL_SPI1_MOSI_PIN               GPIO_PIN_15
#define EVAL_SPI1_SCK_PIN                GPIO_PIN_13
#define EVAL_SPI1_GPIO_AF_IDX            GPIO_AF_5

#define EVAL_SPI2                        SPI2
#define EVAL_SPI2_GPIO_CLK               RCU_GPIOB
#define EVAL_SPI2_CLK                    RCU_SPI2
#define EVAL_SPI2_GPIO_PORT              GPIOB
#define EVAL_SPI2_MISO_PIN               GPIO_PIN_4
#define EVAL_SPI2_MOSI_PIN               GPIO_PIN_5
#define EVAL_SPI2_SCK_PIN                GPIO_PIN_3
#define EVAL_SPI2_GPIO_AF_IDX            GPIO_AF_6

#define EVAL_SPI3                        SPI3
#define EVAL_SPI3_GPIO_CLK               RCU_GPIOE
#define EVAL_SPI3_CLK                    RCU_SPI3
#define EVAL_SPI3_GPIO_PORT              GPIOE
#define EVAL_SPI3_MISO_PIN               GPIO_PIN_5
#define EVAL_SPI3_MOSI_PIN               GPIO_PIN_6
#define EVAL_SPI3_SCK_PIN                GPIO_PIN_4
#define EVAL_SPI3_GPIO_AF_IDX            GPIO_AF_5

#define EVAL_SPI4                        SPI4
#define EVAL_SPI4_GPIO_CLK               RCU_GPIOE
#define EVAL_SPI4_CLK                    RCU_SPI4
#define EVAL_SPI4_GPIO_PORT              GPIOE
#define EVAL_SPI4_MISO_PIN               GPIO_PIN_13
#define EVAL_SPI4_MOSI_PIN               GPIO_PIN_14
#define EVAL_SPI4_SCK_PIN                GPIO_PIN_12
#define EVAL_SPI4_GPIO_AF_IDX            GPIO_AF_6

#define EVAL_SPI5                        SPI5
#define EVAL_SPI5_GPIO_CLK               RCU_GPIOG
#define EVAL_SPI5_CLK                    RCU_SPI5
#define EVAL_SPI5_GPIO_PORT              GPIOG
#define EVAL_SPI5_MISO_PIN               GPIO_PIN_12
#define EVAL_SPI5_MOSI_PIN               GPIO_PIN_14
#define EVAL_SPI5_SCK_PIN                GPIO_PIN_13
#define EVAL_SPI5_GPIO_AF_IDX            GPIO_AF_5

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

struct gd_spi_info {
    os_uint32_t hspi;
    rcu_periph_enum rcu_spi_gpio_base;
    rcu_periph_enum rcu_spi_base;
    os_uint32_t spi_pin_base;
    os_uint32_t spi_miso_pin;
    os_uint32_t spi_mosi_pin;
    os_uint32_t spi_sck_pin;
    os_uint32_t alternate_functions;
};

#endif /*__DRV_SPI_H_ */

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
 * @brief       This file implements usart driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_usart.h"

#ifdef BSP_USING_UART1
struct cm32_usart_info uart1_info = {
    .uart_device   = 1,
    .irqn          = USART1_IRQn,
    .idx           = USART1,
    .uart_clk      = USART1_CLK,
    .uart_gpio_clk = USART1_GPIO_CLK,
    .tx_port       = USART1_TX_GPIO,
    .tx_pin        = USART1_TX_PIN,
    .tx_af         = USART1_TX_GPIO_AF,
    .rx_port       = USART1_RX_GPIO,
    .rx_pin        = USART1_RX_PIN,
    .rx_af         = USART1_RX_GPIO_AF,

#ifdef UART1_USING_DMA
    .dma_support   = 1,
    .dma_channel   = DMA_CH2,
    .periph_addr   = USART1_BASE,
    .dma_remap     = DMA_REMAP_USART1_RX
#else
    .dma_support   = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart1", uart1_info);
#endif

#ifdef BSP_USING_UART2
struct cm32_usart_info uart2_info = {
    .uart_device   = 2,
    .irqn          = USART2_IRQn,
    .idx           = USART2,
    .uart_clk      = USART2_CLK,
    .uart_gpio_clk = USART2_GPIO_CLK,
    .tx_port       = USART2_TX_GPIO,
    .tx_pin        = USART2_TX_PIN,
    .tx_af         = USART2_TX_GPIO_AF,
    .rx_port       = USART2_RX_GPIO,
    .rx_pin        = USART2_RX_PIN,
    .rx_af         = USART2_RX_GPIO_AF,

#ifdef UART2_USING_DMA
    .dma_support   = 1,
    .dma_channel   = DMA_CH3,
    .periph_addr   = USART2_BASE,
    .dma_remap     = DMA_REMAP_USART2_RX
#else
    .dma_support   = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart2", uart2_info);
#endif

#ifdef BSP_USING_UART3
struct cm32_usart_info uart3_info = {
    .uart_device   = 3,
    .irqn          = USART3_IRQn,
    .idx           = USART3,
    .uart_clk      = USART3_CLK,
    .uart_gpio_clk = USART3_GPIO_CLK,
    .tx_port       = USART3_TX_GPIO,
    .tx_pin        = USART3_TX_PIN,
    .tx_af         = USART3_TX_GPIO_AF,
    .rx_port       = USART3_RX_GPIO,
    .rx_pin        = USART3_RX_PIN,
    .rx_af         = USART3_RX_GPIO_AF,

#ifdef UART3_USING_DMA
    .dma_support   = 1,
    .dma_channel   = DMA_CH4,
    .periph_addr   = USART3_BASE,
    .dma_remap     = DMA_REMAP_USART3_RX
#else
    .dma_support   = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart3", uart3_info);
#endif

#ifdef BSP_USING_UART4
struct cm32_usart_info uart4_info = {
    .uart_device   = 4,
    .irqn          = UART4_IRQn,
    .idx           = UART4,
    .uart_clk      = UART4_CLK,
    .uart_gpio_clk = UART4_GPIO_CLK,
    .tx_port       = UART4_TX_GPIO,
    .tx_pin        = UART4_TX_PIN,
    .tx_af         = UART4_TX_GPIO_AF,
    .rx_port       = UART4_RX_GPIO,
    .rx_pin        = UART4_RX_PIN,
    .rx_af         = UART4_RX_GPIO_AF,

#ifdef UART4_USING_DMA
    .dma_support   = 1,
    .dma_channel   = DMA_CH5,
    .periph_addr   = UART4_BASE,
    .dma_remap     = DMA_REMAP_UART4_RX
#else
    .dma_support   = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart4", uart4_info);
#endif

#ifdef BSP_USING_UART5
struct cm32_usart_info uart5_info = {
    .uart_device   = 5,
    .irqn          = UART5_IRQn,
    .idx           = UART5,
    .uart_clk      = UART5_CLK,
    .uart_gpio_clk = UART5_GPIO_CLK,
    .tx_port       = UART5_TX_GPIO,
    .tx_pin        = UART5_TX_PIN,
    .tx_af         = UART5_TX_GPIO_AF,
    .rx_port       = UART5_RX_GPIO,
    .rx_pin        = UART5_RX_PIN,
    .rx_af         = UART5_RX_GPIO_AF,

#ifdef UART5_USING_DMA
    .dma_support   = 1,
    .dma_channel   = DMA_CH6,
    .periph_addr   = UART5_BASE,
    .dma_remap     = DMA_REMAP_UART5_RX
#else
    .dma_support   = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart5", uart5_info);
#endif

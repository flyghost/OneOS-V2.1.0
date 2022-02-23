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
 * @brief       This file implements usart driver for fm33
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_uart.h"

#ifdef BSP_USING_UART0
struct fm33_usart_info uart0_info = {
    .instance   = UART0,
    .index      = 0,
    .port       = GPIOF,
    .tx_pin     = GPIO_Pin_4,
    .rx_pin     = GPIO_Pin_3,
    .clk_src    = UART0CLK,
    .irqn       = UART0_IRQn,
    .dma_param  =
    {
        .CHx     = DMA_CH1,
        .CHxSSEL = DMA_CH1CTRL_CH1SSEL_UART0_RX,
    },
    .dma_enable     = UART0_DMA_FLAG,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart0", uart0_info);
#endif

#ifdef BSP_USING_UART1
struct fm33_usart_info uart1_info = {
    .instance   = UART1,
    .index      = 1,
    .port       = GPIOB,
    .tx_pin     = GPIO_Pin_1,
    .rx_pin     = GPIO_Pin_0,
    .clk_src    = UART1CLK,
    .irqn       = UART1_IRQn,
    .dma_param  =
    {
        .CHx     = DMA_CH3,
        .CHxSSEL = DMA_CH3CTRL_CH3SSEL_UART1_RX,
    },
    .dma_enable     = UART1_DMA_FLAG,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", uart1_info);
#endif

#ifdef BSP_USING_UART2
struct fm33_usart_info uart2_info = {
    .instance   = UART2,
    .index      = 2,
    .port       = GPIOB,
    .tx_pin     = GPIO_Pin_3,
    .rx_pin     = GPIO_Pin_2,
    .clk_src    = UART2CLK,
    .irqn       = UART2_IRQn,
    .dma_param  =
    {
        .CHx     = DMA_CH5,
        .CHxSSEL = DMA_CH5CTRL_CH5SSEL_UART2_RX,
    },
    .dma_enable     = UART2_DMA_FLAG,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", uart2_info);
#endif

#ifdef BSP_USING_UART3
struct fm33_usart_info uart3_info = {
    .instance   = UART3,
    .index      = 3,
    .port       = GPIOC,
    .tx_pin     = GPIO_Pin_11,
    .rx_pin     = GPIO_Pin_10,
    .clk_src    = UART3CLK,
    .irqn       = UART3_IRQn,
    .dma_param  =
    {
        .CHx     = DMA_CH1,
        .CHxSSEL = DMA_CH1CTRL_CH1SSEL_UART3_RX,
    },
    .dma_enable     = UART3_DMA_FLAG,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", uart3_info);
#endif

#ifdef BSP_USING_UART4
struct fm33_usart_info uart4_info = {
    .instance   = UART4,
    .index      = 4,
    .port       = GPIOD,
    .tx_pin     = GPIO_Pin_1,
    .rx_pin     = GPIO_Pin_0,
    .clk_src    = UART4CLK,
    .irqn       = UART4_IRQn,
    .dma_param  =
    {
        .CHx     = DMA_CH3,
        .CHxSSEL = DMA_CH3CTRL_CH3SSEL_UART4_RX,
    },
    .dma_enable     = UART4_DMA_FLAG,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart4", uart4_info);
#endif

#ifdef BSP_USING_UART5
struct fm33_usart_info uart5_info = {
    .instance   = UART5,
    .index      = 5,
    .port       = GPIOC,
    .tx_pin     = GPIO_Pin_5,
    .rx_pin     = GPIO_Pin_4,
    .clk_src    = UART5CLK,
    .irqn       = UART5_IRQn,
    .dma_param  =
    {
        .CHx     = DMA_CH5,
        .CHxSSEL = DMA_CH5CTRL_CH5SSEL_UART5_RX,
    },
    .dma_enable     = UART5_DMA_FLAG,
};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart5", uart5_info);
#endif

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
 * @brief       This file implements usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_usart.h"

#ifdef BSP_USING_UART0
struct hc32_usart_info uart0_info = {
    .uart_device = 0,
    .irqn        = UART0_2_IRQn,
    .idx         = M0P_UART0,
    .tx_port     = UART0_TX_PORT,
    .tx_pin      = UART0_TX_PIN,
    .tx_af       = UART0_TX_AF,
    .rx_port     = UART0_RX_PORT,
    .rx_pin      = UART0_RX_PIN,
    .rx_af       = UART0_RX_AF,
    .req_num     = DmaUart0RxTrig,
#ifdef  UART0_USING_DMA
    .dma_support = 1
#else
    .dma_support = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart0", uart0_info);
#endif

#ifdef BSP_USING_UART1
struct hc32_usart_info uart1_info = {
    .uart_device = 1,
    .irqn        = UART1_3_IRQn,
    .idx         = M0P_UART1,
    .tx_port     = UART1_TX_PORT,
    .tx_pin      = UART1_TX_PIN,
    .tx_af       = UART1_TX_AF,
    .rx_port     = UART1_RX_PORT,
    .rx_pin      = UART1_RX_PIN,
    .rx_af       = UART1_RX_AF,
    .req_num     = DmaUart1RxTrig,
#ifdef  UART1_USING_DMA
    .dma_support = 1
#else
    .dma_support = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart1", uart1_info);
#endif

#ifdef BSP_USING_UART2
struct hc32_usart_info uart2_info = {
    .uart_device = 2,
    .irqn        = UART0_2_IRQn,
    .idx         = M0P_UART2,
    .tx_port     = UART2_TX_PORT,
    .tx_pin      = UART2_TX_PIN,
    .tx_af       = UART2_TX_AF,
    .rx_port     = UART2_RX_PORT,
    .rx_pin      = UART2_RX_PIN,
    .rx_af       = UART2_RX_AF,
    .req_num     = DmaUart2RxTrig,
#ifdef  UART2_USING_DMA
    .dma_support = 1
#else
    .dma_support = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart2", uart2_info);
#endif

#ifdef BSP_USING_UART3
struct hc32_usart_info uart3_info = {
    .uart_device = 3,
    .irqn        = UART1_3_IRQn,
    .idx         = M0P_UART3,
    .tx_port     = UART3_TX_PORT,
    .tx_pin      = UART3_TX_PIN,
    .tx_af       = UART3_TX_AF,
    .rx_port     = UART3_RX_PORT,
    .rx_pin      = UART3_RX_PIN,
    .rx_af       = UART3_RX_AF,
    .req_num     = DmaUart3RxTrig,
#ifdef  UART3_USING_DMA
    .dma_support = 1
#else
    .dma_support = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart3", uart3_info);
#endif

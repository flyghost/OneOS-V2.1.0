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
 * @file        drv_uart.h
 *
 * @brief       This file implements uart driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#ifndef DRV_UART_H__
#define DRV_UART_H__

#include "peripherals.h"

#define LPUART_IRQHandler_DEFINE(__uart_index)                                              \
void LPUART##__uart_index##_IRQHandler(void)                                                \
{                                                                                           \
    struct os_imxrt_uart *uart;                                                             \
                                                                                            \
    os_list_for_each_entry(uart, &imxrt_uart_list, struct os_imxrt_uart, list)              \
    {                                                                                       \
        if (uart->uart_info->uart_base == LPUART##__uart_index)                             \
        {                                                                                   \
            break;                                                                          \
        }                                                                                   \
    }                                                                                       \
                                                                                            \
    if (uart->uart_info->uart_base == LPUART##__uart_index)                                 \
        imxrt_uart_irq_callback(uart);                                                      \
}

#define UART_IRQ_INIT(_NXP_UART_, __index)                                                  \
    _NXP_UART_->clk_src             = LPUART##__index##_CLOCK_SOURCE;                       \
    _NXP_UART_->irqn                = LPUART##__index##_SERIAL_RX_TX_IRQN;                  \

#define UART_NBK_INIT(_NXP_UART_, __index)                                                  \
    _NXP_UART_->clk_src             = LPUART##__index##_CLOCK_SOURCE;                       \
    _NXP_UART_->handle              = &LPUART##__index##_handle;                            \

#define UART_DMA_INIT(_NXP_UART_, __index)                                                  \
    _NXP_UART_->clk_src             = LPUART##__index##_CLOCK_SOURCE;                       \
    _NXP_UART_->irqn                = LPUART##__index##_IRQn;                               \
    _NXP_UART_->rxdma_base          = LPUART##__index##_RX_DMA_BASEADDR;                    \
    _NXP_UART_->rxdma_channel       = LPUART##__index##_RX_DMA_CHANNEL;                     \
    _NXP_UART_->edma_handle         = &LPUART##__index##_LPUART_eDMA_Handle;                \

#define UART_POL_INIT(_NXP_UART_, __index)                                                  \
    _NXP_UART_->clk_src = LPUART##__index##_CLOCK_SOURCE;                                   \

#define UART_NULL_INIT(_NXP_UART_, __index)

#if defined(LPUART1_SERIAL_RX_TX_IRQN)
#define UART1_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART1_RX_BUFFER_SIZE) && defined(LPUART1_TX_BUFFER_SIZE)
#define UART1_CFG_INIT UART_NBK_INIT
#elif defined(LPUART1_RX_DMA_CHANNEL) && defined(LPUART1_TX_DMA_CHANNEL)
#define UART1_CFG_INIT UART_DMA_INIT
#elif defined(LPUART1_PERIPHERAL)
#define UART1_CFG_INIT UART_POL_INIT
#else
#define UART1_CFG_INIT UART_NULL_INIT
#endif

#if defined(LPUART2_SERIAL_RX_TX_IRQN)
#define UART2_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART2_RX_BUFFER_SIZE) && defined(LPUART2_TX_BUFFER_SIZE)
#define UART2_CFG_INIT UART_NBK_INIT
#elif defined(LPUART2_RX_DMA_CHANNEL) && defined(LPUART2_TX_DMA_CHANNEL)
#define UART2_CFG_INIT UART_DMA_INIT
#elif defined(LPUART2_PERIPHERAL)
#define UART2_CFG_INIT UART_POL_INIT
#else
#define UART2_CFG_INIT UART_NULL_INIT
#endif

#if defined(LPUART3_SERIAL_RX_TX_IRQN)
#define UART3_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART3_RX_BUFFER_SIZE) && defined(LPUART3_TX_BUFFER_SIZE)
#define UART3_CFG_INIT UART_NBK_INIT
#elif defined(LPUART3_RX_DMA_CHANNEL) && defined(LPUART3_TX_DMA_CHANNEL)
#define UART3_CFG_INIT UART_DMA_INIT
#elif defined(LPUART3_PERIPHERAL)
#define UART3_CFG_INIT UART_POL_INIT
#else
#define UART3_CFG_INIT UART_NULL_INIT
#endif

#if defined(LPUART4_SERIAL_RX_TX_IRQN)
#define UART4_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART4_RX_BUFFER_SIZE) && defined(LPUART4_TX_BUFFER_SIZE)
#define UART4_CFG_INIT UART_NBK_INIT
#elif defined(LPUART4_RX_DMA_CHANNEL) && defined(LPUART4_TX_DMA_CHANNEL)
#define UART4_CFG_INIT UART_DMA_INIT
#elif defined(LPUART4_PERIPHERAL)
#define UART4_CFG_INIT UART_POL_INIT
#else
#define UART4_CFG_INIT UART_NULL_INIT
#endif

#if defined(LPUART5_SERIAL_RX_TX_IRQN)
#define UART5_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART5_RX_BUFFER_SIZE) && defined(LPUART5_TX_BUFFER_SIZE)
#define UART5_CFG_INIT UART_NBK_INIT
#elif defined(LPUART5_RX_DMA_CHANNEL) && defined(LPUART5_TX_DMA_CHANNEL)
#define UART5_CFG_INIT UART_DMA_INIT
#elif defined(LPUART5_PERIPHERAL)
#define UART5_CFG_INIT UART_POL_INIT
#else
#define UART5_CFG_INIT UART_NULL_INIT
#endif

#if defined(LPUART6_SERIAL_RX_TX_IRQN)
#define UART6_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART6_RX_BUFFER_SIZE) && defined(LPUART6_TX_BUFFER_SIZE)
#define UART6_CFG_INIT UART_NBK_INIT
#elif defined(LPUART6_RX_DMA_CHANNEL) && defined(LPUART6_TX_DMA_CHANNEL)
#define UART6_CFG_INIT UART_DMA_INIT
#elif defined(LPUART6_PERIPHERAL)
#define UART6_CFG_INIT UART_POL_INIT
#else
#define UART6_CFG_INIT UART_NULL_INIT
#endif

#if defined(LPUART7_SERIAL_RX_TX_IRQN)
#define UART7_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART7_RX_BUFFER_SIZE) && defined(LPUART7_TX_BUFFER_SIZE)
#define UART7_CFG_INIT UART_NBK_INIT
#elif defined(LPUART7_RX_DMA_CHANNEL) && defined(LPUART7_TX_DMA_CHANNEL)
#define UART7_CFG_INIT UART_DMA_INIT
#elif defined(LPUART7_PERIPHERAL)
#define UART7_CFG_INIT UART_POL_INIT
#else
#define UART7_CFG_INIT UART_NULL_INIT
#endif

#if defined(LPUART8_SERIAL_RX_TX_IRQN)
#define UART8_CFG_INIT UART_IRQ_INIT
#elif defined(LPUART8_RX_BUFFER_SIZE) && defined(LPUART8_TX_BUFFER_SIZE)
#define UART8_CFG_INIT UART_NBK_INIT
#elif defined(LPUART8_RX_DMA_CHANNEL) && defined(LPUART8_TX_DMA_CHANNEL)
#define UART8_CFG_INIT UART_DMA_INIT
#elif defined(LPUART8_PERIPHERAL)
#define UART8_CFG_INIT UART_POL_INIT
#else
#define UART8_CFG_INIT UART_NULL_INIT
#endif

struct nxp_lpuart_info {
    LPUART_Type *uart_base;
    const lpuart_config_t *config;
};

#endif

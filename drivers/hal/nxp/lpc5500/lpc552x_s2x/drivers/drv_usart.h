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
 * @file        drv_usart.h
 *
 * @brief       This file implements usart driver for lpc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include <drv_common.h>

#define USART_IRQHandler_DEFINE(__index)                                                        \
void USART##__index##_FLEXCOMM_IRQHANDLER(void)                                                 \
{                                                                                               \
    lpc_usart_t *lpc_usart;                                                                     \
    os_list_for_each_entry(lpc_usart, &lpc_usart_list, lpc_usart_t, list)                       \
    {                                                                                           \
        if (lpc_usart->usart_info->usart_base == (USART_Type *)FLEXCOMM##__index)               \
        {                                                                                       \
            break;                                                                              \
        }                                                                                       \
    }                                                                                           \
    if (lpc_usart->usart_info->usart_base != (USART_Type *)FLEXCOMM##__index)                   \
        return;                                                                                 \
    lpc_usart_irq_callback(lpc_usart);                                                          \
    SDK_ISR_EXIT_BARRIER;                                                                       \
}

#define USART_IRQ_INIT(_NXP_USART_, __index)                                                    \
    _NXP_USART_->clk_src = USART##__index##_CLOCK_SOURCE;                                       \
    _NXP_USART_->irqn = USART##__index##_FLEXCOMM_IRQN;                                         \

#define USART_NBK_INIT(_NXP_USART_, __index)                                                    \
    _NXP_USART_->clk_src = USART##__index##_CLOCK_SOURCE;                                       \
    _NXP_USART_->usart_handle = &USART##__index##_handle;                                       \
    _NXP_USART_->usart_handle->callback = lpc_usart_transfer_callback;                          \
    _NXP_USART_->usart_handle->userData = _NXP_USART_;                                          \

#define USART_DMA_INIT(_NXP_USART_, __index)                                                    \
    _NXP_USART_->clk_src = USART##__index##_CLOCK_SOURCE;                                       \
    _NXP_USART_->dma_descriptor[0] = &lpc_usart##__index##_dma_pingpong_desc[0];                \
    _NXP_USART_->dma_descriptor[1] = &lpc_usart##__index##_dma_pingpong_desc[1];                \
    _NXP_USART_->usart_DmaHandle = &USART##__index##_USART_DMA_Handle;                          \
    _NXP_USART_->usart_DmaHandle->callback = lpc_usart_dma_callback;                            \
    _NXP_USART_->usart_DmaHandle->userData = _NXP_USART_;                                       \

#define USART_POL_INIT(_NXP_USART_, __index)                                                    \
    _NXP_USART_->clk_src = USART##__index##_CLOCK_SOURCE;                                       \

#define USART_NULL_INIT(_NXP_USART_, __index)

#if defined(USART0_FLEXCOMM_IRQN)
#define USART0_CFG_INIT USART_IRQ_INIT
#elif defined(USART0_RX_BUFFER_SIZE) && defined(USART0_TX_BUFFER_SIZE)
#define USART0_CFG_INIT USART_NBK_INIT
#elif defined(USART0_RX_DMA_CHANNEL) && defined(USART0_TX_DMA_CHANNEL)
#define USART0_CFG_INIT USART_DMA_INIT
#elif defined(USART0_FLEXCOMM)
#define USART0_CFG_INIT USART_POL_INIT
#else
#define USART0_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART1_FLEXCOMM_IRQN)
#define USART1_CFG_INIT USART_IRQ_INIT
#elif defined(USART1_RX_BUFFER_SIZE) && defined(USART1_TX_BUFFER_SIZE)
#define USART1_CFG_INIT USART_NBK_INIT
#elif defined(USART1_RX_DMA_CHANNEL) && defined(USART1_TX_DMA_CHANNEL)
#define USART1_CFG_INIT USART_DMA_INIT
#elif defined(USART1_FLEXCOMM)
#define USART1_CFG_INIT USART_POL_INIT
#else
#define USART1_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART2_FLEXCOMM_IRQN)
#define USART2_CFG_INIT USART_IRQ_INIT
#elif defined(USART2_RX_BUFFER_SIZE) && defined(USART2_TX_BUFFER_SIZE)
#define USART2_CFG_INIT USART_NBK_INIT
#elif defined(USART2_RX_DMA_CHANNEL) && defined(USART2_TX_DMA_CHANNEL)
#define USART2_CFG_INIT USART_DMA_INIT
#elif defined(USART2_FLEXCOMM)
#define USART2_CFG_INIT USART_POL_INIT
#else
#define USART2_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART3_FLEXCOMM_IRQN)
#define USART3_CFG_INIT USART_IRQ_INIT
#elif defined(USART3_RX_BUFFER_SIZE) && defined(USART3_TX_BUFFER_SIZE)
#define USART3_CFG_INIT USART_NBK_INIT
#elif defined(USART3_RX_DMA_CHANNEL) && defined(USART3_TX_DMA_CHANNEL)
#define USART3_CFG_INIT USART_DMA_INIT
#elif defined(USART3_FLEXCOMM)
#define USART3_CFG_INIT USART_POL_INIT
#else
#define USART3_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART4_FLEXCOMM_IRQN)
#define USART4_CFG_INIT USART_IRQ_INIT
#elif defined(USART4_RX_BUFFER_SIZE) && defined(USART4_TX_BUFFER_SIZE)
#define USART4_CFG_INIT USART_NBK_INIT
#elif defined(USART4_RX_DMA_CHANNEL) && defined(USART4_TX_DMA_CHANNEL)
#define USART4_CFG_INIT USART_DMA_INIT
#elif defined(USART4_FLEXCOMM)
#define USART4_CFG_INIT USART_POL_INIT
#else
#define USART4_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART5_FLEXCOMM_IRQN)
#define USART5_CFG_INIT USART_IRQ_INIT
#elif defined(USART5_RX_BUFFER_SIZE) && defined(USART5_TX_BUFFER_SIZE)
#define USART5_CFG_INIT USART_NBK_INIT
#elif defined(USART5_RX_DMA_CHANNEL) && defined(USART5_TX_DMA_CHANNEL)
#define USART5_CFG_INIT USART_DMA_INIT
#elif defined(USART5_FLEXCOMM)
#define USART5_CFG_INIT USART_POL_INIT
#else
#define USART5_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART6_FLEXCOMM_IRQN)
#define USART6_CFG_INIT USART_IRQ_INIT
#elif defined(USART6_RX_BUFFER_SIZE) && defined(USART6_TX_BUFFER_SIZE)
#define USART6_CFG_INIT USART_NBK_INIT
#elif defined(USART6_RX_DMA_CHANNEL) && defined(USART6_TX_DMA_CHANNEL)
#define USART6_CFG_INIT USART_DMA_INIT
#elif defined(USART6_FLEXCOMM)
#define USART6_CFG_INIT USART_POL_INIT
#else
#define USART6_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART7_FLEXCOMM_IRQN)
#define USART7_CFG_INIT USART_IRQ_INIT
#elif defined(USART7_RX_BUFFER_SIZE) && defined(USART7_TX_BUFFER_SIZE)
#define USART7_CFG_INIT USART_NBK_INIT
#elif defined(USART7_RX_DMA_CHANNEL) && defined(USART7_TX_DMA_CHANNEL)
#define USART7_CFG_INIT USART_DMA_INIT
#elif defined(USART7_FLEXCOMM)
#define USART7_CFG_INIT USART_POL_INIT
#else
#define USART7_CFG_INIT USART_NULL_INIT
#endif

#if defined(USART8_FLEXCOMM_IRQN)
#define USART8_CFG_INIT USART_IRQ_INIT
#elif defined(USART8_RX_BUFFER_SIZE) && defined(USART8_TX_BUFFER_SIZE)
#define USART8_CFG_INIT USART_NBK_INIT
#elif defined(USART8_RX_DMA_CHANNEL) && defined(USART8_TX_DMA_CHANNEL)
#define USART8_CFG_INIT USART_DMA_INIT
#elif defined(USART8_FLEXCOMM)
#define USART8_CFG_INIT USART_POL_INIT
#else
#define USART8_CFG_INIT USART_NULL_INIT
#endif

enum lpc_dma_status
{
    DMA_BUFF_INA = 0,
    DMA_BUFF_INA_IRQ,
    DMA_BUFF_INB,
    DMA_BUFF_INB_IRQ,
    DMA_BUFF_NONE,
};

typedef struct lpc_usart_info
{
    USART_Type *usart_base;
    const usart_config_t *usart_config;
}lpc_usart_info_t;

int os_hw_usart_init(void);
typedef void (*hal_usart_transfer_callback_t)(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);

#endif

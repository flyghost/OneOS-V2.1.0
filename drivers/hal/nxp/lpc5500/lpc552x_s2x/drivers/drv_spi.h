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
 * @brief       This file implements spi driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include <drv_common.h>

#define SPI_IRQHandler_DEFINE(__index)                                                      \
void SPI_##__index##_FLEXCOMM_IRQHANDLER(void)                                              \
{                                                                                           \
    struct lpc_spi *lpc_spi;                                                                \
    os_list_for_each_entry(lpc_spi, &lpc_spi_list, struct lpc_spi, list)                    \
    {                                                                                       \
        if (lpc_spi->spi_info->spi_base == (SPI_Type *)FLEXCOMM##__index)                   \
        {                                                                                   \
            break;                                                                          \
        }                                                                                   \
    }                                                                                       \
    if (lpc_spi->spi_info->spi_base != (SPI_Type *)FLEXCOMM##__index)                       \
        return;                                                                             \
    lpc_spi_irq_callback(lpc_spi);                                                          \
    SDK_ISR_EXIT_BARRIER;                                                                   \
}

#define SPI_IRQ_INIT(_NXP_SPI_, __index)                                                    \
        _NXP_SPI_->clk_src = SPI##__index##_CLOCK_SOURCE;                                   \
        _NXP_SPI_->irqn = SPI##__index##_FLEXCOMM_IRQN;
    
#define SPI_NBK_INIT(_NXP_SPI_, __index)                                                    \
        _NXP_SPI_->clk_src = SPI##__index##_CLOCK_SOURCE;                                   \
        _NXP_SPI_->spi_handle = &SPI##__index##_handle;                                     \
        _NXP_SPI_->spi_handle->callback = lpc_spi_transfer_callback;                        \
        _NXP_SPI_->spi_handle->userData = _NXP_SPI_;                                        \
    
#define SPI_DMA_INIT(_NXP_SPI_, __index)                                                    \
        _NXP_SPI_->clk_src = SPI##__index##_CLOCK_SOURCE;                                   \
        _NXP_SPI_->spi_DmaHandle = &SPI##__index##_SPI_DMA_Handle;                           \
        _NXP_SPI_->spi_DmaHandle->callback = lpc_spi_dma_callback;                           \
        _NXP_SPI_->spi_DmaHandle->userData = _NXP_SPI_;                                      \
    
#define SPI_POL_INIT(_NXP_SPI_, __index) _NXP_SPI_->clk_src = SPI##__index##_CLOCK_SOURCE;
    
#define SPI_NULL_INIT(_NXP_SPI_, __index) return;
        
#if defined(SPI0_FLEXCOMM_IRQN)
#define SPI0_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI0_RX_BUFFER_SIZE) && defined(SPI0_TX_BUFFER_SIZE)
#define SPI0_CFG_INIT SPI_NBK_INIT
#elif defined(SPI0_RX_DMA_CHANNEL) && defined(SPI0_TX_DMA_CHANNEL)
#define SPI0_CFG_INIT SPI_DMA_INIT
#elif defined(SPI0_FLEXCOMM)
#define SPI0_CFG_INIT SPI_POL_INIT
#else
#define SPI0_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI1_FLEXCOMM_IRQN)
#define SPI1_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI1_RX_BUFFER_SIZE) && defined(SPI1_TX_BUFFER_SIZE)
#define SPI1_CFG_INIT SPI_NBK_INIT
#elif defined(SPI1_RX_DMA_CHANNEL) && defined(SPI1_TX_DMA_CHANNEL)
#define SPI1_CFG_INIT SPI_DMA_INIT
#elif defined(SPI1_FLEXCOMM)
#define SPI1_CFG_INIT SPI_POL_INIT
#else
#define SPI1_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI2_FLEXCOMM_IRQN)
#define SPI2_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI2_RX_BUFFER_SIZE) && defined(SPI2_TX_BUFFER_SIZE)
#define SPI2_CFG_INIT SPI_NBK_INIT
#elif defined(SPI2_RX_DMA_CHANNEL) && defined(SPI2_TX_DMA_CHANNEL)
#define SPI2_CFG_INIT SPI_DMA_INIT
#elif defined(SPI2_FLEXCOMM)
#define SPI2_CFG_INIT SPI_POL_INIT
#else
#define SPI2_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI3_FLEXCOMM_IRQN)
#define SPI3_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI3_RX_BUFFER_SIZE) && defined(SPI3_TX_BUFFER_SIZE)
#define SPI3_CFG_INIT SPI_NBK_INIT
#elif defined(SPI3_RX_DMA_CHANNEL) && defined(SPI3_TX_DMA_CHANNEL)
#define SPI3_CFG_INIT SPI_DMA_INIT
#elif defined(SPI3_FLEXCOMM)
#define SPI3_CFG_INIT SPI_POL_INIT
#else
#define SPI3_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI4_FLEXCOMM_IRQN)
#define SPI4_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI4_RX_BUFFER_SIZE) && defined(SPI4_TX_BUFFER_SIZE)
#define SPI4_CFG_INIT SPI_NBK_INIT
#elif defined(SPI4_RX_DMA_CHANNEL) && defined(SPI4_TX_DMA_CHANNEL)
#define SPI4_CFG_INIT SPI_DMA_INIT
#elif defined(SPI4_FLEXCOMM)
#define SPI4_CFG_INIT SPI_POL_INIT
#else
#define SPI4_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI5_FLEXCOMM_IRQN)
#define SPI5_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI5_RX_BUFFER_SIZE) && defined(SPI5_TX_BUFFER_SIZE)
#define SPI5_CFG_INIT SPI_NBK_INIT
#elif defined(SPI5_RX_DMA_CHANNEL) && defined(SPI5_TX_DMA_CHANNEL)
#define SPI5_CFG_INIT SPI_DMA_INIT
#elif defined(SPI5_FLEXCOMM)
#define SPI5_CFG_INIT SPI_POL_INIT
#else
#define SPI5_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI6_FLEXCOMM_IRQN)
#define SPI6_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI6_RX_BUFFER_SIZE) && defined(SPI6_TX_BUFFER_SIZE)
#define SPI6_CFG_INIT SPI_NBK_INIT
#elif defined(SPI6_RX_DMA_CHANNEL) && defined(SPI6_TX_DMA_CHANNEL)
#define SPI6_CFG_INIT SPI_DMA_INIT
#elif defined(SPI6_FLEXCOMM)
#define SPI6_CFG_INIT SPI_POL_INIT
#else
#define SPI6_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI7_FLEXCOMM_IRQN)
#define SPI7_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI7_RX_BUFFER_SIZE) && defined(SPI7_TX_BUFFER_SIZE)
#define SPI7_CFG_INIT SPI_NBK_INIT
#elif defined(SPI7_RX_DMA_CHANNEL) && defined(SPI7_TX_DMA_CHANNEL)
#define SPI7_CFG_INIT SPI_DMA_INIT
#elif defined(SPI7_FLEXCOMM)
#define SPI7_CFG_INIT SPI_POL_INIT
#else
#define SPI7_CFG_INIT SPI_NULL_INIT
#endif
    
#if defined(SPI8_FLEXCOMM_IRQN)
#define SPI8_CFG_INIT SPI_IRQ_INIT
#elif defined(SPI8_RX_BUFFER_SIZE) && defined(SPI8_TX_BUFFER_SIZE)
#define SPI8_CFG_INIT SPI_NBK_INIT
#elif defined(SPI8_RX_DMA_CHANNEL) && defined(SPI8_TX_DMA_CHANNEL)
#define SPI8_CFG_INIT SPI_DMA_INIT
#elif defined(SPI8_FLEXCOMM)
#define SPI8_CFG_INIT SPI_POL_INIT
#else
#define SPI8_CFG_INIT SPI_NULL_INIT
#endif

typedef struct lpc_spi_info {
    SPI_Type *spi_base;
    const spi_master_config_t *spi_config;
}lpc_spi_info_t;

#endif

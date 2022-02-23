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

typedef enum 
{
    IMXRT_LPSPI_MASTER  = 0U,
    IMXRT_LPSPI_SLAVE   = 1U
}imxrt_lpspi_mode;

#define LPSPI_IRQHandler_DEFINE(__index)                                                        \
void LPSPI_##__index##_IRQHandler(void)                                                         \
{                                                                                               \
    struct os_imxrt_spi *imxrt_spi;                                                             \
    os_list_for_each_entry(imxrt_spi, &imxrt_spi_list, struct os_imxrt_spi, list)               \
    {                                                                                           \
        if (imxrt_spi->info->base == (LPSPI_Type *)LPSPI##__index)                              \
        {                                                                                       \
            break;                                                                              \
        }                                                                                       \
    }                                                                                           \
    if (imxrt_spi->info->base != (LPSPI_Type *)LPSPI##__index)                                  \
        return;                                                                                 \
    imxrt_spi_irq_callback(imxrt_spi);                                                          \
    SDK_ISR_EXIT_BARRIER;                                                                       \
}

#define LPSPI_IRQ_INIT(_NXP_LPSPI_, __index)                                                    \
    _NXP_LPSPI_->clk_src                = LPSPI##__index##_CLOCK_FREQ;                          \
    _NXP_LPSPI_->irqn                   = LPSPI##__index##_FLEXCOMM_IRQN;
    
#define LPSPI_NBK_INIT(_NXP_LPSPI_, __index)                                                    \
    _NXP_LPSPI_->clk_src                = LPSPI##__index##_CLOCK_FREQ;                          \
    _NXP_LPSPI_->handle                 = (void *)&LPSPI##__index##_handle;                     \
    
#define LPSPI_DMA_INIT(_NXP_LPSPI_, __index)                                                    \
    _NXP_LPSPI_->clk_src                = LPSPI##__index##_CLOCK_FREQ;                          \
    _NXP_LPSPI_->edma_handle            = (void *)&LPSPI##__index##_LPSPI_DMA_Handle;           \
    
#define LPSPI_POL_INIT(_NXP_LPSPI_, __index) _NXP_LPSPI_->clk_src = LPSPI##__index##_CLOCK_FREQ;
    
#define LPSPI_NULL_INIT(_NXP_LPSPI_, __index) return;
        
#if defined(LPSPI1_IRQN)
#define LPSPI1_CFG_INIT LPSPI_IRQ_INIT
#elif defined(LPSPI1_BUFFER_SIZE)
#define LPSPI1_CFG_INIT LPSPI_NBK_INIT
#elif defined(LPSPI1_RX_DMA_CHANNEL) && defined(LPSPI1_TX_DMA_CHANNEL)
#define LPSPI1_CFG_INIT LPSPI_DMA_INIT
#elif defined(LPSPI1_PERIPHERAL)
#define LPSPI1_CFG_INIT LPSPI_POL_INIT
#else
#define LPSPI1_CFG_INIT LPSPI_NULL_INIT
#endif

#if defined(LPSPI2_IRQN)
#define LPSPI2_CFG_INIT LPSPI_IRQ_INIT
#elif defined(LPSPI2_BUFFER_SIZE)
#define LPSPI2_CFG_INIT LPSPI_NBK_INIT
#elif defined(LPSPI2_RX_DMA_CHANNEL) && defined(LPSPI2_TX_DMA_CHANNEL)
#define LPSPI2_CFG_INIT LPSPI_DMA_INIT
#elif defined(LPSPI2_PERIPHERAL)
#define LPSPI2_CFG_INIT LPSPI_POL_INIT
#else
#define LPSPI2_CFG_INIT LPSPI_NULL_INIT
#endif

#if defined(LPSPI3_IRQN)
#define LPSPI3_CFG_INIT LPSPI_IRQ_INIT
#elif defined(LPSPI3_BUFFER_SIZE)
#define LPSPI3_CFG_INIT LPSPI_NBK_INIT
#elif defined(LPSPI3_RX_DMA_CHANNEL) && defined(LPSPI3_TX_DMA_CHANNEL)
#define LPSPI3_CFG_INIT LPSPI_DMA_INIT
#elif defined(LPSPI3_PERIPHERAL)
#define LPSPI3_CFG_INIT LPSPI_POL_INIT
#else
#define LPSPI3_CFG_INIT LPSPI_NULL_INIT
#endif

#if defined(LPSPI4_IRQN)
#define LPSPI4_CFG_INIT LPSPI_IRQ_INIT
#elif defined(LPSPI4_BUFFER_SIZE)
#define LPSPI4_CFG_INIT LPSPI_NBK_INIT
#elif defined(LPSPI4_RX_DMA_CHANNEL) && defined(LPSPI4_TX_DMA_CHANNEL)
#define LPSPI4_CFG_INIT LPSPI_DMA_INIT
#elif defined(LPSPI4_PERIPHERAL)
#define LPSPI4_CFG_INIT LPSPI_POL_INIT
#else
#define LPSPI4_CFG_INIT LPSPI_NULL_INIT
#endif

typedef struct nxp_lpspi_info {
    LPSPI_Type *base;
    const lpspi_master_config_t *mconfig;
    const lpspi_slave_config_t  *sconfig;
}imxrt_spi_info_t;

#endif

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
 * @file        drv_i2c.h
 *
 * @brief       This file implements i2c driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#include <drv_common.h>

#define I2C_IRQHandler_DEFINE(__index)                                                      \
void I2C_##__index##_FLEXCOMM_IRQHANDLER(void)                                              \
{                                                                                           \
    lpc_i2c_t *lpc_i2c;                                                                \
    os_list_for_each_entry(lpc_i2c, &lpc_i2c_list, lpc_i2c_t, list)                    \
    {                                                                                       \
        if (lpc_i2c->i2c_info->i2c_base == (I2C_Type *)FLEXCOMM##__index)                   \
        {                                                                                   \
            break;                                                                          \
        }                                                                                   \
    }                                                                                       \
    if (lpc_i2c->i2c_info->i2c_base != (I2C_Type *)FLEXCOMM##__index)                       \
        return;                                                                             \
    lpc_i2c_irq_callback(lpc_i2c);                                                          \
    SDK_ISR_EXIT_BARRIER;                                                                   \
}

#define I2C_IRQ_INIT(_NXP_I2C_, __index)                                                    \
        _NXP_I2C_->clk_src = I2C##__index##_CLOCK_SOURCE;                                   \
        _NXP_I2C_->irqn = I2C##__index##_FLEXCOMM_IRQN;
    
#define I2C_NBK_INIT(_NXP_I2C_, __index)                                                    \
        _NXP_I2C_->clk_src = I2C##__index##_CLOCK_SOURCE;                                   \
        _NXP_I2C_->i2c_handle = &I2C##__index##_handle;                                     \
        _NXP_I2C_->i2c_handle->callback = lpc_i2c_transfer_callback;                        \
        _NXP_I2C_->i2c_handle->userData = _NXP_I2C_;                                        \
    
#define I2C_DMA_INIT(_NXP_I2C_, __index)                                                    \
        _NXP_I2C_->clk_src = I2C##__index##_CLOCK_SOURCE;                                   \
        _NXP_I2C_->i2cDmaHandle = &I2C##__index##_I2C_DMA_Handle;                           \
        _NXP_I2C_->i2cDmaHandle->callback = lpc_i2c_dma_callback;                           \
        _NXP_I2C_->i2cDmaHandle->userData = _NXP_I2C_;                                      \
    
#define I2C_POL_INIT(_NXP_I2C_, __index) _NXP_I2C_->clk_src = I2C##__index##_CLOCK_SOURCE;
    
#define I2C_NULL_INIT(_NXP_I2C_, __index) return;
        
#if defined(I2C0_FLEXCOMM_IRQN)
#define I2C0_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C0_RX_BUFFER_SIZE) && defined(I2C0_TX_BUFFER_SIZE)
#define I2C0_CFG_INIT I2C_NBK_INIT
#elif defined(I2C0_RX_DMA_CHANNEL) && defined(I2C0_TX_DMA_CHANNEL)
#define I2C0_CFG_INIT I2C_DMA_INIT
#elif defined(I2C0_FLEXCOMM)
#define I2C0_CFG_INIT I2C_POL_INIT
#else
#define I2C0_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C1_FLEXCOMM_IRQN)
#define I2C1_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C1_RX_BUFFER_SIZE) && defined(I2C1_TX_BUFFER_SIZE)
#define I2C1_CFG_INIT I2C_NBK_INIT
#elif defined(I2C1_RX_DMA_CHANNEL) && defined(I2C1_TX_DMA_CHANNEL)
#define I2C1_CFG_INIT I2C_DMA_INIT
#elif defined(I2C1_FLEXCOMM)
#define I2C1_CFG_INIT I2C_POL_INIT
#else
#define I2C1_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C2_FLEXCOMM_IRQN)
#define I2C2_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C2_RX_BUFFER_SIZE) && defined(I2C2_TX_BUFFER_SIZE)
#define I2C2_CFG_INIT I2C_NBK_INIT
#elif defined(I2C2_RX_DMA_CHANNEL) && defined(I2C2_TX_DMA_CHANNEL)
#define I2C2_CFG_INIT I2C_DMA_INIT
#elif defined(I2C2_FLEXCOMM)
#define I2C2_CFG_INIT I2C_POL_INIT
#else
#define I2C2_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C3_FLEXCOMM_IRQN)
#define I2C3_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C3_RX_BUFFER_SIZE) && defined(I2C3_TX_BUFFER_SIZE)
#define I2C3_CFG_INIT I2C_NBK_INIT
#elif defined(I2C3_RX_DMA_CHANNEL) && defined(I2C3_TX_DMA_CHANNEL)
#define I2C3_CFG_INIT I2C_DMA_INIT
#elif defined(I2C3_FLEXCOMM)
#define I2C3_CFG_INIT I2C_POL_INIT
#else
#define I2C3_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C4_FLEXCOMM_IRQN)
#define I2C4_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C4_RX_BUFFER_SIZE) && defined(I2C4_TX_BUFFER_SIZE)
#define I2C4_CFG_INIT I2C_NBK_INIT
#elif defined(I2C4_RX_DMA_CHANNEL) && defined(I2C4_TX_DMA_CHANNEL)
#define I2C4_CFG_INIT I2C_DMA_INIT
#elif defined(I2C4_FLEXCOMM)
#define I2C4_CFG_INIT I2C_POL_INIT
#else
#define I2C4_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C5_FLEXCOMM_IRQN)
#define I2C5_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C5_RX_BUFFER_SIZE) && defined(I2C5_TX_BUFFER_SIZE)
#define I2C5_CFG_INIT I2C_NBK_INIT
#elif defined(I2C5_RX_DMA_CHANNEL) && defined(I2C5_TX_DMA_CHANNEL)
#define I2C5_CFG_INIT I2C_DMA_INIT
#elif defined(I2C5_FLEXCOMM)
#define I2C5_CFG_INIT I2C_POL_INIT
#else
#define I2C5_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C6_FLEXCOMM_IRQN)
#define I2C6_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C6_RX_BUFFER_SIZE) && defined(I2C6_TX_BUFFER_SIZE)
#define I2C6_CFG_INIT I2C_NBK_INIT
#elif defined(I2C6_RX_DMA_CHANNEL) && defined(I2C6_TX_DMA_CHANNEL)
#define I2C6_CFG_INIT I2C_DMA_INIT
#elif defined(I2C6_FLEXCOMM)
#define I2C6_CFG_INIT I2C_POL_INIT
#else
#define I2C6_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C7_FLEXCOMM_IRQN)
#define I2C7_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C7_RX_BUFFER_SIZE) && defined(I2C7_TX_BUFFER_SIZE)
#define I2C7_CFG_INIT I2C_NBK_INIT
#elif defined(I2C7_RX_DMA_CHANNEL) && defined(I2C7_TX_DMA_CHANNEL)
#define I2C7_CFG_INIT I2C_DMA_INIT
#elif defined(I2C7_FLEXCOMM)
#define I2C7_CFG_INIT I2C_POL_INIT
#else
#define I2C7_CFG_INIT I2C_NULL_INIT
#endif
    
#if defined(I2C8_FLEXCOMM_IRQN)
#define I2C8_CFG_INIT I2C_IRQ_INIT
#elif defined(I2C8_RX_BUFFER_SIZE) && defined(I2C8_TX_BUFFER_SIZE)
#define I2C8_CFG_INIT I2C_NBK_INIT
#elif defined(I2C8_RX_DMA_CHANNEL) && defined(I2C8_TX_DMA_CHANNEL)
#define I2C8_CFG_INIT I2C_DMA_INIT
#elif defined(I2C8_FLEXCOMM)
#define I2C8_CFG_INIT I2C_POL_INIT
#else
#define I2C8_CFG_INIT I2C_NULL_INIT
#endif


typedef struct lpc_i2c_info {
    I2C_Type *i2c_base;
    const i2c_master_config_t *master_config;
}lpc_i2c_info_t;

#endif


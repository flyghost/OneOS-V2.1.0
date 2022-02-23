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
 * @file        mm32_it_dma.h
 *
 * @brief       This file provides dma IRQ declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __MM32_IT_DMA_H__
#define __MM32_IT_DMA_H__

#include "mm32_hal.h"
#include <os_stddef.h>

typedef enum
{
    IRQN_UART_TX    = 0x00,
    IRQN_UART_RX    = 0x01,
    IRQN_SPI_TX     = 0x10,
    IRQN_SPI_RX     = 0x11,
    IRQN_I2C_TX     = 0x20,
    IRQN_I2C_RX     = 0x21,
}irqn_type;

typedef enum
{
    TYPE_DMA_IT_GL    = 0x00,
    TYPE_DMA_IT_TC,
    TYPE_DMA_IT_HT,
    TYPE_DMA_IT_TE,
}irqn_type_flag;

typedef void (*dma_callback)(void *handle, irqn_type_flag type_flag);

typedef struct mm32_dma_table
{
    DMA_Channel_TypeDef    *dma_channel;
    void                   *handle;
    dma_callback            callback;
}mm32_dma_table_t;

#if defined(SERIES_MM32F327XX)
void HAL_DMA_IRQHandler(DMA_Channel_TypeDef *dma_channel, DMA_Interrupts_TypeDef Interrupts_Type_Base);
#else
void HAL_DMA_IRQHandler(DMA_Channel_TypeDef *dma_channel, os_uint32_t Interrupts_Type_Base);
#endif
os_err_t dma_info_table_config(DMA_Channel_TypeDef *dma_channel, void *handle, dma_callback callback);

#endif 

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
 * @file        mm32_it_dma.c
 *
 * @brief       This file provides dma IRQ functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mm32_it_dma.h"
#include <os_stddef.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.irq_dma"
#include <drv_log.h>

static mm32_dma_table_t dma_table[] = 
{
#ifdef DMA1_Channel1
    {DMA1_Channel1, OS_NULL, OS_NULL},
#endif
#ifdef DMA1_Channel2
    {DMA1_Channel2, OS_NULL, OS_NULL},
#endif
#ifdef DMA1_Channel3
    {DMA1_Channel3, OS_NULL, OS_NULL},
#endif
#ifdef DMA1_Channel4
    {DMA1_Channel4, OS_NULL, OS_NULL},
#endif
#ifdef DMA1_Channel5
    {DMA1_Channel5, OS_NULL, OS_NULL},
#endif
#ifdef DMA1_Channel6
    {DMA1_Channel6, OS_NULL, OS_NULL},
#endif
#ifdef DMA1_Channel7
    {DMA1_Channel7, OS_NULL, OS_NULL},
#endif
    
#ifdef DMA2_Channel1
    {DMA2_Channel1, OS_NULL, OS_NULL},
#endif
#ifdef DMA2_Channel2
    {DMA2_Channel2, OS_NULL, OS_NULL},
#endif
#ifdef DMA2_Channel3
    {DMA2_Channel3, OS_NULL, OS_NULL},
#endif
#ifdef DMA2_Channel4
    {DMA2_Channel4, OS_NULL, OS_NULL},
#endif
#ifdef DMA2_Channel5
    {DMA2_Channel5, OS_NULL, OS_NULL},
#endif
#ifdef DMA2_Channel6
    {DMA2_Channel6, OS_NULL, OS_NULL},
#endif
#ifdef DMA2_Channel7
    {DMA2_Channel7, OS_NULL, OS_NULL},
#endif
};
static os_uint8_t dma_table_size = sizeof(dma_table) / sizeof(dma_table[0]);

os_err_t dma_info_table_config(DMA_Channel_TypeDef *dma_channel, void *handle, dma_callback callback)
{
    os_uint8_t i = 0;
    for(i = 0;i < dma_table_size;i++)
    {
        if (dma_table[i].dma_channel == dma_channel)
        {
            if (dma_table[i].handle != OS_NULL)
            {
                LOG_E(DRV_EXT_TAG, "this dma channel has been used!");
                return OS_ERROR;
            }
            
            dma_table[i].handle = handle;
            dma_table[i].callback = callback;
            return OS_EOK;
        }
    }
    LOG_E(DRV_EXT_TAG, "cannot find dma channel irqn!");
    return OS_ERROR;
}

#if defined(SERIES_MM32F327XX) || defined(SERIES_MM32F027XX)
void HAL_DMA_IRQHandler(DMA_Channel_TypeDef *dma_channel, DMA_Interrupts_TypeDef Interrupts_Type_Base)
{
    os_uint8_t i = 0;
    os_uint32_t type_base = 0;
    os_uint32_t type_bit = 0;
    DMA_Interrupts_TypeDef Interrupts_Type;
    
    mm32_dma_table_t *table = OS_NULL;
    
    for(i = 0;i < dma_table_size;i++)
    {
        if (dma_table[i].dma_channel == dma_channel)
        {
            table = &dma_table[i];
            break;
        }
    }
    
    type_base = Interrupts_Type_Base & 0x10000000;
    type_bit = Interrupts_Type_Base & 0x0FFFFFFF;
    
    Interrupts_Type = (DMA_Interrupts_TypeDef)(type_base | (type_bit << 1));
    if (RESET != DMA_GetITStatus(Interrupts_Type))
    {
        DMA_ClearITPendingBit(Interrupts_Type);
        table->callback(table->handle, TYPE_DMA_IT_TC);
    }
    
    Interrupts_Type = (DMA_Interrupts_TypeDef)(type_base | (type_bit << 2));
    if (RESET != DMA_GetITStatus(Interrupts_Type))
    {
        DMA_ClearITPendingBit(Interrupts_Type);
        table->callback(table->handle, TYPE_DMA_IT_HT);
    }
    
    Interrupts_Type = (DMA_Interrupts_TypeDef)(type_base | (type_bit << 3));
    if (RESET != DMA_GetITStatus(Interrupts_Type))
    {
        DMA_ClearITPendingBit(Interrupts_Type);
        table->callback(table->handle, TYPE_DMA_IT_TE);
    }
}
#else
void HAL_DMA_IRQHandler(DMA_Channel_TypeDef *dma_channel, os_uint32_t Interrupts_Type_Base)
{
    os_uint8_t i = 0;
    os_uint32_t type_base = 0;
    os_uint32_t type_bit = 0;
    os_uint32_t Interrupts_Type;
    
    mm32_dma_table_t *table = OS_NULL;
    
    for(i = 0;i < dma_table_size;i++)
    {
        if (dma_table[i].dma_channel == dma_channel)
        {
            table = &dma_table[i];
            break;
        }
    }
    
    type_base = Interrupts_Type_Base & 0x10000000;
    type_bit = Interrupts_Type_Base & 0x7FFFFFFF;
    
    Interrupts_Type = (os_uint32_t)(type_base | (type_bit << 1));
    if (RESET != DMA_GetITStatus(Interrupts_Type))
    {
        table->callback(table->handle, TYPE_DMA_IT_TC);
        DMA_ClearITPendingBit(Interrupts_Type);
    }
    
    Interrupts_Type = (os_uint32_t)(type_base | (type_bit << 2));
    if (RESET != DMA_GetITStatus(Interrupts_Type))
    {
        table->callback(table->handle, TYPE_DMA_IT_HT);
        DMA_ClearITPendingBit(Interrupts_Type);
    }
    
    Interrupts_Type = (os_uint32_t)(type_base | (type_bit << 3));
    if (RESET != DMA_GetITStatus(Interrupts_Type))
    {
        table->callback(table->handle, TYPE_DMA_IT_TE);
        DMA_ClearITPendingBit(Interrupts_Type);
    }
}
#endif

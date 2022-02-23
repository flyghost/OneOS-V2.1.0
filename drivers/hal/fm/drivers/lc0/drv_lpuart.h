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
 * @file        drv_lpuart.h
 *
 * @brief       This file provides struct/macro declaration and functions declaration for lpuart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-07-29   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_LPUART_H__
#define __DRV_LPUART_H__

#include <board.h>

struct fm33_lpuart_info
{
    LPUART_Type  *hlpuart;

    os_uint32_t  index;
    os_uint32_t  clock_src;
    os_uint32_t  baud_rate;

    /*interrupt information*/
    IRQn_Type    irqn;
    os_uint32_t  irq_priority;

    /*pin information*/
    GPIO_Type   *port;
    FL_GPIO_InitTypeDef GPIO_InitStruct_rx_pin;
    FL_GPIO_InitTypeDef GPIO_InitStruct_tx_pin;

    /*dma information*/
    DMA_Type    *hdma;

    os_uint32_t  hdma_rx_channel;
    FL_DMA_InitTypeDef dmaInitStruct_rx;
};

struct fm33_lpuart
{
    struct os_serial_device serial;
    struct fm33_lpuart_info *info;

    soft_dma_t  sdma;

    os_uint8_t *rx_buff;
    os_uint32_t rx_size;
    os_uint32_t rx_index;

    const os_uint8_t *tx_buff;
    os_uint32_t tx_size;
    os_uint32_t tx_index;

    os_list_node_t list;
};

void LPUART_DMA_IRQHandler_Callback(void);

#endif /* __DRV_LPUART_H__ */

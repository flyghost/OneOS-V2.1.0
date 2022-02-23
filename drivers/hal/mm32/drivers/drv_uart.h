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
 * @brief        This file provides functions declaration for uart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include <os_types.h>
#include "mm32_hal.h"
#include "mm32_it.h"

#if defined(SERIES_MM32F103XX) || defined(SERIES_MM32L3XX) || defined(SERIES_MM32SPIN2XX)
#define MM32_UART_ISR_RX             UART_ISR_RX_INTF
#define MM32_UART_IER_RX             UART_IER_RXIEN
#define MM32_UART_ISR_TX             UART_ISR_TX_INTF
#define MM32_UART_IER_TX             UART_IER_TXIEN
#else
#define MM32_UART_ISR_RX             UART_ISR_RX
#define MM32_UART_IER_RX             UART_IER_RX
#define MM32_UART_ISR_RXIDLE         UART_ISR_RXIDLE
#define MM32_UART_IER_RXIDLE         UART_IER_RXIDLE
#define MM32_UART_ISR_TX             UART_ISR_TX
#define MM32_UART_IER_TX             UART_IER_TX
#endif

typedef struct {
#if defined(SERIES_MM32F103XX) || defined(SERIES_MM32L3XX) || defined(SERIES_MM32SPIN2XX)
    os_uint32_t RX_DMA_Mode;
    os_uint32_t RX_DMA_Priority;
    os_uint32_t TX_DMA_Priority;
#else
    DMA_circular_normal_mode_TypeDef RX_DMA_Mode;
    DMA_priority_level_TypeDef RX_DMA_Priority;
    DMA_priority_level_TypeDef TX_DMA_Priority;
#endif
} DMA_InitInfo;

typedef struct mm32_uart_info
{
    UART_TypeDef               *huart;
    os_uint32_t                 uart_clk;
    GPIO_InitTypeDef            tx_pin_info;
    os_uint8_t                  tx_pin_source;
    GPIO_TypeDef               *tx_pin_port;
    GPIO_InitTypeDef            rx_pin_info;
    os_uint8_t                  rx_pin_source;
    GPIO_TypeDef               *rx_pin_port;
    os_uint8_t                  pin_af;
    UART_InitTypeDef            uart_info;
    NVIC_InitTypeDef            uart_nvic_info;
    DMA_InitInfo                dma_info;
    os_uint32_t                 dma_tx_clk;
    DMA_Channel_TypeDef        *dma_tx_channel;
    NVIC_InitTypeDef            dma_tx_nvic_info;
    os_uint32_t                 dma_rx_clk;
    DMA_Channel_TypeDef        *dma_rx_channel;
    NVIC_InitTypeDef            dma_rx_nvic_info;
}mm32_uart_info_t;

typedef struct mm32_uart
{
    struct os_serial_device     serial;
    struct mm32_uart_info      *info;
    
    soft_dma_t                  sdma;
    os_uint32_t                 sdma_hard_size;
    
    os_uint8_t                 *rx_buff;
    os_uint32_t                 rx_index;
    os_uint32_t                 rx_size;

    const os_uint8_t           *tx_buff;
    os_uint32_t                 tx_count;
    os_uint32_t                 tx_size;

    os_list_node_t              list;
}mm32_uart_t;

#endif /* __DRV_UART_H__ */
 
 
 

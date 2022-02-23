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
 * @brief       This file provides struct/macro declaration and functions declaration for uart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include <drv_common.h>
#include "board.h"
#include "fm33g0xx_lib.h"

#if defined(BSP_USING_UART0) && defined (UART0_USE_DMA)
#define UART0_DMA_FLAG OS_TRUE
#else
#define UART0_DMA_FLAG OS_FALSE
#endif

#if defined(BSP_USING_UART1) && defined (UART1_USE_DMA)
#define UART1_DMA_FLAG OS_TRUE
#else
#define UART1_DMA_FLAG OS_FALSE
#endif

#if defined(BSP_USING_UART2) && defined (UART2_USE_DMA)
#define UART2_DMA_FLAG OS_TRUE
#else
#define UART2_DMA_FLAG OS_FALSE
#endif

#if defined(BSP_USING_UART3) && defined (UART3_USE_DMA)
#define UART3_DMA_FLAG OS_TRUE
#else
#define UART3_DMA_FLAG OS_FALSE
#endif

#if defined(BSP_USING_UART4) && defined (UART4_USE_DMA)
#define UART4_DMA_FLAG OS_TRUE
#else
#define UART4_DMA_FLAG OS_FALSE
#endif

#if defined(BSP_USING_UART5) && defined (UART5_USE_DMA)
#define UART5_DMA_FLAG OS_TRUE
#else
#define UART5_DMA_FLAG OS_FALSE
#endif

struct fm33_usart_info
{
    UARTx_Type       *instance;
    os_uint32_t      index;

    /*pin*/
    GPIOx_Type       *port;
    os_uint32_t       tx_pin;
    os_uint32_t       rx_pin; 

    /*params*/
    uint32_t          clk_src;
    UART_SInitTypeDef uart_param;

    /*irq*/
    IRQn_Type         irqn;

    /*dma*/
    DMA_InitTypeDef   dma_param;
    os_bool_t         dma_enable;
		os_uint32_t       dma_addr_start;
};

#endif /* __DRV_UART_H__ */

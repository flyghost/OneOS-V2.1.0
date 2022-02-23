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
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_LPUART_H__
#define __DRV_LPUART_H__

#include <drv_common.h>
#include "board.h"
#include "fm33g0xx_lib.h"

#if defined(BSP_USING_LPUART0) && defined (LPUART0_USE_DMA)
#define LPUART0_DMA_FLAG OS_TRUE
#else
#define LPUART0_DMA_FLAG OS_FALSE
#endif

struct fm33_lpuart_info
{
    LPUART_Type      *instance;

    /*pin*/
    GPIOx_Type       *port;
    os_uint32_t       tx_pin;
    os_uint32_t       rx_pin; 
    os_uint32_t       baud_rate;
    os_uint32_t       irq_priority;

    /*params*/
    LPUART_SInitTypeDef uart_param;

    /*irq*/
    IRQn_Type         irqn;

    /*dma*/
    DMA_InitTypeDef   dma_param;
    os_bool_t         dma_enable;
    os_uint32_t       dma_addr_start;
};

#endif /* __DRV_LPUART_H__ */

/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        drv_uart.h
 *
 * \@brief       This file provides operation functions declaration for uart.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_UART_H_
#define __DRV_UART_H_

#include "drv_common.h"
#include "fm33lg0xx_fl_dma.h"
#include "fm33lg0xx_fl_gpio.h"


struct fm33_usart_info
{
    UART_Type   *husart;

    os_uint32_t  index;
    os_uint32_t  clock_src;
    os_uint32_t  baud_rate;

    /*interrupt information*/
    IRQn_Type    irqn;
    os_uint32_t  irq_priority;

    /*pin information*/
    GPIO_Type   *port;
    FL_GPIO_InitTypeDef GPIO_InitStruct;

    /*dma information*/
    DMA_Type    *hdma;

    os_uint32_t  hdma_rx_channel;
    FL_DMA_InitTypeDef dmaInitStruct_rx;
};

void UART_DMA_IRQHandler_Callback(void);

#endif /* __DRV_UART_H_ */

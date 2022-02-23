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
 * @file        drv_usart.h
 *
 * @brief       The head file of usart driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include "board.h"

struct cm32_usart_info
{
    uint32_t    uart_device;
    uint8_t   irqn;
    USART_Module* idx;
    uint32_t  uart_clk;
    uint32_t  uart_gpio_clk;
    GPIO_Module*  tx_port;
    uint16_t  tx_pin;
    uint32_t   tx_af;
    GPIO_Module* rx_port;
    uint16_t  rx_pin;
    uint32_t   rx_af;
    uint8_t   dma_support;

#if defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA) \
    || defined(UART4_USING_DMA) || defined(UART5_USING_DMA)

    DMA_ChannelType*  dma_channel;
    uint32_t periph_addr;
    uint32_t dma_remap;
#endif
};

int os_hw_usart_init(void);

#endif /* __DRV_USART_H__ */

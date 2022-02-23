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
 * @file        drv_dma_irq.h
 *
 * @brief       The head file of dma irq for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_DMA_IRQ_H__
#define __DRV_DMA_IRQ_H__

#include "board.h"
#include "drv_usart.h"

#ifdef BSP_USING_LPUART
#include "drv_lpuart.h"
#endif

struct dma_channel_flag
{
    uint8_t ch0_lpuart;
    uint8_t ch0_uart;
    uint8_t ch1_lpuart;
    uint8_t ch1_uart;
};

void uart_dma_channel_get(struct hc_uart *uart);

#ifdef BSP_USING_LPUART
void lpuart_dma_channel_get(struct hc_lpuart *lpuart);
#endif

#endif /* __DRV_DMA_IRQ_H__ */

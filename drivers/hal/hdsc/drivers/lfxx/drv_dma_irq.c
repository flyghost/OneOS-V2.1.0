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
 * @file        drv_dma_irq.c
 *
 * @brief       This file implements dma irq for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_dma_irq.h>

struct dma_channel_flag dma_ch_flag = {0};
static uint8_t dma_channel_used = 0;

void uart_dma_channel_get(struct hc_uart *uart)
{
    if (uart->info->dma_support == 1)
    {
        if (dma_channel_used >= 2)
        {
            OS_ASSERT(0);
        }

        if (dma_channel_used == 0)
        {
            uart->info->dma_channel = DmaCh0;
            dma_ch_flag.ch0_uart = 1;
            dma_channel_used++;
            return;
        }
        else if (dma_channel_used == 1)
        {
            uart->info->dma_channel = DmaCh1;
            dma_ch_flag.ch1_uart = 1;
            dma_channel_used++;
            return;
        }
    }

    return;
}

#ifdef BSP_USING_LPUART
void lpuart_dma_channel_get(struct hc_lpuart *lpuart)
{
    if (lpuart->info->dma_support == 1)
    {
        if (dma_channel_used >= 2)
        {
            OS_ASSERT(0);
        }

        if (dma_channel_used == 0)
        {
            lpuart->info->dma_channel = DmaCh0;
            dma_ch_flag.ch0_lpuart = 1;
            dma_channel_used++;
            return;
        }
        else if (dma_channel_used == 1)
        {
            lpuart->info->dma_channel = DmaCh1;
            dma_ch_flag.ch1_lpuart = 1;
            dma_channel_used++;
            return;
        }
    }

    return;
}
#endif

void Dmac_IRQHandler(void)
{
    if (DmaTransferComplete == Dma_GetStat(DmaCh0))
    {
        Dma_DisableChannel(DmaCh0);
        Dma_DisableChannelIrq(DmaCh0);

#ifdef BSP_USING_LPUART
        if (dma_ch_flag.ch0_lpuart == 1)
        {
            lpuart_dma_irq(DmaCh0);
        }
#endif

#ifdef BSP_USING_UART
        if (dma_ch_flag.ch0_uart == 1)
        {
            uart_dma_irq(DmaCh0);
        }
#endif
    }

    if (DmaTransferComplete == Dma_GetStat(DmaCh1))
    {
        Dma_DisableChannel(DmaCh1);
        Dma_DisableChannelIrq(DmaCh1);

#ifdef BSP_USING_LPUART
        if (dma_ch_flag.ch1_lpuart == 1)
        {
            lpuart_dma_irq(DmaCh1);
        }
#endif

#ifdef BSP_USING_UART
        if (dma_ch_flag.ch1_uart == 1)
        {
            uart_dma_irq(DmaCh1);
        }
#endif
    }
}

void __os_hw_console_output(char *str)
{

}

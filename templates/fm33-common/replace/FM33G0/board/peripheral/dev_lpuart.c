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
 * @file        dev_lpuart.c
 *
 * @brief       This file implements lpuart selection for fm33
 *
 * @revision
 * Date         Author          Notes
 * 202-07-30   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_lpuart.h"

#ifdef BSP_USING_LPUART0
struct fm33_lpuart_info lpuart0_info = {
    .instance   = LPUART,
    .port       = GPIOF,
    .tx_pin     = GPIO_Pin_4,
    .rx_pin     = GPIO_Pin_3,
    .baud_rate  = BAUD_RATE_9600,
    .irq_priority = 2,
    .irqn       = LPUART_IRQn,
    .dma_param  =
    {
        .CHx     = DMA_CH5,
        .CHxSSEL = DMA_CH5CTRL_CH5SSEL_LPUART_RX,
    },
    .dma_enable     = LPUART0_DMA_FLAG,
};
OS_HAL_DEVICE_DEFINE("LPUART_HandleTypeDef", "lpuart0", lpuart0_info);
#endif

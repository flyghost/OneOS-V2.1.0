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
 * @brief       This file implements lpuart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_lpuart.h"

#ifdef BSP_USING_LPUART0
struct hc32_lpuart_info lpuart0_info = {
    .uart_device = 0,
    .irqn        = LPUART0_IRQn,
    .lp_idx      = M0P_LPUART0,
    .tx_port     = LPUART0_TX_PORT,
    .tx_pin      = LPUART0_TX_PIN,
    .tx_af       = LPUART0_TX_AF,
    .rx_port     = LPUART0_RX_PORT,
    .rx_pin      = LPUART0_RX_PIN,
    .rx_af       = LPUART0_RX_AF,
    .req_num     = DmaLpUart0RxTrig,
#ifdef  LPUART0_USING_DMA
    .dma_support = 1
#else
    .dma_support = 0
#endif
};
OS_HAL_DEVICE_DEFINE("LPuart_Type", "lpuart0", lpuart0_info);
#endif

#ifdef BSP_USING_LPUART1
struct hc32_lpuart_info lpuart1_info = {
    .uart_device = 1,
    .irqn        = LPUART1_IRQn,
    .lp_idx      = M0P_LPUART1,
    .tx_port     = LPUART1_TX_PORT,
    .tx_pin      = LPUART1_TX_PIN,
    .tx_af       = LPUART1_TX_AF,
    .rx_port     = LPUART1_RX_PORT,
    .rx_pin      = LPUART1_RX_PIN,
    .rx_af       = LPUART1_RX_AF,
    .req_num     = DmaLpUart1RxTrig,
#ifdef  LPUART1_USING_DMA
    .dma_support = 1
#else
    .dma_support = 0
#endif
};
OS_HAL_DEVICE_DEFINE("LPuart_Type", "lpuart1", lpuart1_info);
#endif

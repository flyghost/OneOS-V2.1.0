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
 * @brief       This file implements lpuart driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_lpuart.h"

#ifdef BSP_USING_LPUART0
struct cm32_lpuart_info lpuart0_info = {
    .lpuart_clk      = LPUART0_CLK,
    .lpuart_gpio_clk = LPUART0_GPIO_CLK,
    .tx_port         = LPUART0_TX_GPIO,
    .tx_pin          = LPUART0_TxPin,
    .tx_af           = LPUART0_Tx_GPIO_AF,
    .rx_port         = LPUART0_RX_GPIO,
    .rx_pin          = LPUART0_RxPin,
    .rx_af           = LPUART0_Rx_GPIO_AF,

#ifdef LPUART0_USING_DMA
    .dma_support     = 1
#endif
};
OS_HAL_DEVICE_DEFINE("LPuart_Type", "lpuart0", lpuart0_info);
#endif

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
 * @file        dev_usart.c
 *
 * @brief       This file implements usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "drv_usart.h"
#include "hc32f4a0_pwc.h"

#ifdef BSP_USING_UART1
struct hc32_usart_info uart1_info = {
    .uart_device = 1,
    .irqn        = Int000_IRQn,
    .idx         = M4_USART1,
    .periph      = PWC_FCG3_USART1,
    .int_Src     = INT_USART1_RI,
    .tx_port     = UART1_TX_PORT,
    .tx_pin      = UART1_TX_PIN,
    .tx_func     = UART1_TX_FUNC,
    .rx_port     = UART1_RX_PORT,
    .rx_pin      = UART1_RX_PIN,
    .rx_func     = UART1_RX_FUNC,
#ifdef  UART1_USING_DMA
    .dma_support = 1
#else
    .dma_support = 0
#endif
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart1", uart1_info);
#endif

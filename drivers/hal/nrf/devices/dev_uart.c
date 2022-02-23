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
 * @file        drv_uart.c
 *
 * @brief       This file implements uart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include "drv_uart.h"

#ifdef NRF52832_XXAA
#define EVAL_COM0_TX_PIN                 6
#define EVAL_COM0_RX_PIN                 8

#elif NRF52840_XXAA
#define EVAL_COM0_TX_PIN                 6
#define EVAL_COM0_RX_PIN                 8
#define EVAL_COM1_TX_PIN                 26
#define EVAL_COM1_RX_PIN                 27
#endif

#ifdef BSP_USING_UART0
struct nrf5_uart_info uart0_info = {NRFX_UARTE_INSTANCE(0)
                                        , EVAL_COM0_RX_PIN
                                        , EVAL_COM0_TX_PIN};
OS_HAL_DEVICE_DEFINE("uart_Type", "uart0", uart0_info);
#endif

#ifdef BSP_USING_UART1
struct nrf5_uart_info uart1_info = {NRFX_UARTE_INSTANCE(1)
                                        , EVAL_COM1_RX_PIN
                                        , EVAL_COM1_TX_PIN};
OS_HAL_DEVICE_DEFINE("uart_Type", "uart1", uart1_info);
#endif




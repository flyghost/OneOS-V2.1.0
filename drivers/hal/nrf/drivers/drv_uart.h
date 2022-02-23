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
 * @brief        This file provides functions declaration for usart driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include "nrfx_uarte.h"

#define EVAL_COM0                        0
#define EVAL_COM0_PARITY                 _PRIO_APP_LOWEST
#define EVAL_COM0_RTS_PIN                 9
#define EVAL_COM0_CTS_PIN                 10
#define EVAL_COM0_FLOW_CTR                 APP_UART_FLOW_CONTROL_DISABLED

#define EVAL_COM1                        1
#define EVAL_COM1_PARITY                 _PRIO_APP_LOWEST
#define EVAL_COM1_RTS_PIN                 9
#define EVAL_COM1_CTS_PIN                 10
#define EVAL_COM1_FLOW_CTR                 APP_UART_FLOW_CONTROL_DISABLED

#define UART_RX_BUF_SIZE 256
#define UART_TX_BUF_SIZE 256

struct nrf5_uart_info
{
    nrfx_uarte_t uart;
    uint32_t rx_pin;
    uint32_t tx_pin;
};

void nrf5_uart_recfg(void);

#endif

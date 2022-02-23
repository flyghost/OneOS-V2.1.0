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
 * @brief       The head file of usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include "board.h"

#include "hc32f4a0_gpio.h"
#include "hc32f4a0_usart.h"
#include "hc32f4a0_pwc.h"
#include "hc32f4a0_interrupts.h"

struct hc32_usart_info
{
    uint32_t            uart_device;
    IRQn_Type           irqn;
    M4_USART_TypeDef   *idx;
    uint32_t            periph;
    en_int_src_t        int_Src;
    uint16_t            tx_port;
    uint16_t            tx_pin;
    uint16_t            tx_func;
    uint16_t            rx_port;
    uint16_t            rx_pin;
    uint16_t            rx_func;
    uint8_t             dma_support;
};


#define MEM_ZERO_STRUCT(x)   do {                                                \
                                 memset((void*)&(x), 0l, (sizeof(x)));           \
                             } while(0)


#endif /* __DRV_USART_H__ */

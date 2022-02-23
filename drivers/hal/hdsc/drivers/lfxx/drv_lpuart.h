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
 * @file        drv_lpuart.h
 *
 * @brief       The head file of lpuart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_LPUART_H__
#define __DRV_LPUART_H__

#include "board.h"
#include "hc_dmac.h"
#include "hc_lpuart.h"
#include "hc_gpio.h"

struct hc32_lpuart_info
{
    uint32_t    uart_device;
    IRQn_Type   irqn;
    M0P_LPUART_TypeDef* lp_idx;
    en_gpio_port_t tx_port;
    en_gpio_pin_t  tx_pin;
    en_gpio_af_t   tx_af;
    en_gpio_port_t rx_port;
    en_gpio_pin_t  rx_pin;
    en_gpio_af_t   rx_af;
    en_dma_trig_sel_t req_num;
    uint8_t   dma_support;
    en_dma_channel_t dma_channel;
};

/* lpuart driver */
typedef struct hc_lpuart
{
    struct os_serial_device serial_dev;
    struct hc32_lpuart_info *info;
    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;
    uint8_t  *rx_buff;
    uint32_t  rx_index;
    uint32_t  rx_size;
    os_list_node_t list;
} hc_lpuart_t;

void lpuart_dma_irq(en_dma_channel_t dma_channel);

#endif /* __DRV_LPUART_H__ */

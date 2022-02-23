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

#include <os_task.h>
#include <device.h>
#include <drv_cfg.h>
#include "gd32vf103.h"
#include "gd32vf103_rcu.h"
#include <soft_dma.h>

/* GD32 uart driver */
struct gd32_uart
{
    struct os_serial_device serial;
        struct gd_usart_info *usart_info;    
        soft_dma_t  sdma;
        os_uint32_t sdma_hard_size;        
        os_uint8_t *rx_buff;
        os_uint32_t rx_index;
        os_uint32_t rx_size;
        os_list_node_t list;
        os_size_t tx_count;
        os_size_t tx_size;
        const os_uint8_t *tx_buff;
};


struct gd_usart_info {
    uint32_t uart_periph;
        IRQn_Type irqn;
        rcu_periph_enum per_clk;
        rcu_periph_enum tx_gpio_clk;
        rcu_periph_enum rx_gpio_clk;
        uint32_t tx_port;
        uint16_t tx_pin;
        uint32_t rx_port;
        uint16_t rx_pin;        
        uint8_t use_dma;
        uint32_t dma_periph;
        uint8_t dma_channel;
        IRQn_Type dma_irqn;
        rcu_periph_enum dma_clk;
};

int os_hw_usart_init(void);

#endif /* __DRV_USART_H__ */

/******************* end of file *******************/

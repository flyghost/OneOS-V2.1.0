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
 * @brief       The head file of lpuart driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_LPUART_H__
#define __DRV_LPUART_H__

#include "board.h"

struct cm32_lpuart_info
{
    uint32_t  lpuart_clk;
    uint32_t  lpuart_gpio_clk;
    GPIO_Module*  tx_port;
    uint16_t  tx_pin;
    uint32_t  tx_af;
    GPIO_Module* rx_port;
    uint16_t  rx_pin;
    uint32_t  rx_af;
    uint8_t   dma_support;
};

int os_hw_lpuart_init(void);

#endif /* __DRV_LPUART_H__ */

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

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include <device.h>
#include <gd32f4xx.h>


#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"


struct gd32_usart_info {
    os_uint32_t husart;
    os_uint32_t usart_clk;
    os_uint32_t baud_rate;
    os_uint32_t irq;
    os_uint32_t tx_pin;
    os_uint32_t rx_pin;
    os_uint32_t pin_port;
    os_uint32_t pin_clk;
    os_uint32_t gpio_af_idx;
    
    os_uint32_t *dma_periph;           /* DMA0 */
    dma_channel_enum dma_channel;      /* CHANNEL3 */
};

#endif /* __DRV_USART_H__ */

/******************* end of file *******************/


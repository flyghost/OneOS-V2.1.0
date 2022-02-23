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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include "drv_usart.h"

#ifdef BSP_USING_USART0
struct gd32_usart_info usart0_info = 
{
    .husart = USART0, 
    .usart_clk = RCU_USART0,
    .baud_rate = BAUD_RATE_115200, 
    .irq = USART0_IRQn,
    .tx_pin = GPIO_PIN_9, 
    .rx_pin = GPIO_PIN_10,
    .pin_port = GPIOA, 
    .pin_clk = RCU_GPIOA,
    .gpio_af_idx = 0,
    .dma_periph = (os_uint32_t *)DMA0,
    .dma_channel = DMA_CH4
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart0", usart0_info);
#endif

#ifdef BSP_USING_USART1
struct gd32_usart_info usart1_info = 
{   
    .husart = USART1, 
    .usart_clk = RCU_USART1,
    .baud_rate = BAUD_RATE_115200,
    .irq = USART1_IRQn,
    .tx_pin = GPIO_PIN_2,
    .rx_pin = GPIO_PIN_3,
    .pin_port = GPIOA, 
    .pin_clk = RCU_GPIOA,
    .gpio_af_idx = 0
    .dma_periph = (os_uint32_t *)DMA0,
    .dma_channel = DMA_CH5    
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart1", usart1_info);
#endif  

#ifdef BSP_USING_USART2
struct gd32_usart_info usart2_info = 
{
    .husart = USART2, 
    .usart_clk = RCU_USART2,
    .baud_rate = BAUD_RATE_921600, 
    .irq = USART2_IRQn,
    .tx_pin = GPIO_PIN_8,
    .rx_pin = GPIO_PIN_9,
    .pin_port = GPIOD, 
    .pin_clk = RCU_GPIOD,
    .gpio_af_idx = 0
    .dma_periph = (os_uint32_t *)DMA0,
    .dma_channel = DMA_CH2        
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "usart2", usart2_info);
#endif 


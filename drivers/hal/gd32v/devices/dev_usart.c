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
 * @file        dev_i2c.c
 *
 * @brief       This file implements usart driver for gd32v
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_usart.h>
#ifdef OS_USING_USART0
struct gd_usart_info usart0_info = {.uart_periph=USART0,.irqn=USART0_IRQn,.per_clk=RCU_USART0,.tx_gpio_clk=RCU_GPIOA,
    .rx_gpio_clk=RCU_GPIOA,.tx_port=GPIOA,.tx_pin=GPIO_PIN_9,.rx_port=GPIOA,.rx_pin=GPIO_PIN_10,
#if defined(OS_USING_USART0_DMA)
    .use_dma=1,
#else
    .use_dma=0,
#endif
    .dma_periph=DMA0,.dma_channel=DMA_CH4,.dma_irqn=DMA0_Channel4_IRQn,.dma_clk=RCU_DMA0};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart0", usart0_info);
#endif

#ifdef OS_USING_USART1
struct gd_usart_info usart1_info = {.uart_periph=USART1,.irqn=USART1_IRQn,.per_clk=RCU_USART1,.tx_gpio_clk=RCU_GPIOA,
    .rx_gpio_clk=RCU_GPIOA,.tx_port=GPIOA,.tx_pin=GPIO_PIN_2,.rx_port=GPIOA,.rx_pin=GPIO_PIN_3,
#if defined(OS_USING_USART1_DMA)
    .use_dma=1,
#else
    .use_dma=0,
#endif
    .dma_periph=DMA0,.dma_channel=DMA_CH5,.dma_irqn=DMA0_Channel5_IRQn,.dma_clk=RCU_DMA0};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", usart1_info);
#endif


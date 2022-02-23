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
 * 2021-04-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifdef BSP_USING_USART
#include <board.h>
#include "drv_usart.h"
#endif

#ifdef BSP_USING_USART1
struct hk32_usart_info usart1_info =
{
    .huart = USART1,
    .irq   = USART1_IRQn,
    .rcc_tpye = HK32_RCC_APB2,
    .rcc   = RCC_APB2Periph_USART1,

    .gpio_tx_port = GPIOA,
    .gpio_tx_pin  = GPIO_Pin_9,
    .gpio_tx_rcc  = RCC_AHBPeriph_GPIOA,

    .gpio_rx_port = GPIOA,
    .gpio_rx_pin  = GPIO_Pin_10,
    .gpio_rx_rcc  = RCC_AHBPeriph_GPIOA,
};

OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", usart1_info);
#endif

#ifdef BSP_USING_USART2
struct hk32_usart_info usart2_info =
{
    .huart = USART2,
    .irq   = USART2_IRQn,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc   = RCC_APB1Periph_USART2,

    .gpio_tx_port = GPIOA,
    .gpio_tx_pin  = GPIO_Pin_2,
    .gpio_tx_rcc  = RCC_AHBPeriph_GPIOA,

    .gpio_rx_port = GPIOA,
    .gpio_rx_pin  = GPIO_Pin_3,
    .gpio_rx_rcc  = RCC_AHBPeriph_GPIOA,
};

OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", usart2_info);
#endif

#ifdef BSP_USING_USART3
struct hk32_usart_info usart3_info =
{
    .huart = USART3,
    .irq   = USART3_IRQn,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc   = RCC_APB1Periph_USART3,

    .gpio_tx_port = GPIOB,
    .gpio_tx_pin  = GPIO_Pin_10,
    .gpio_tx_rcc  = RCC_AHBPeriph_GPIOB,

    .gpio_rx_port = GPIOB,
    .gpio_rx_pin  = GPIO_Pin_11,
    .gpio_rx_rcc  = RCC_AHBPeriph_GPIOB,
};

OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart3", usart3_info);
#endif

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
 * @file        dev_usart.c
 *
 * @brief       This file implements usart devices for ingenic uart
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_usart.h>

#ifdef BSP_USING_USART0
struct ingenic_usart_info usart0_info = {.hw_base=UART0_BASE,.irqno=IRQ_UART0,.use_dma = 0};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart0", usart0_info);
#endif

#ifdef BSP_USING_USART1
struct ingenic_usart_info usart1_info = {.hw_base=UART1_BASE,.irqno=IRQ_UART1,.use_dma = 0};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart1", usart1_info);
#endif  

#ifdef BSP_USING_USART2
struct ingenic_usart_info usart2_info = {.hw_base=UART2_BASE,.irqno=IRQ_UART2,.use_dma = 0};
OS_HAL_DEVICE_DEFINE("UART_HandleTypeDef", "uart2", usart2_info);
#endif


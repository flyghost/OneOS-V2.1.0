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
 * @brief       This file implements usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_uart.h"

#ifdef BSP_USING_UART0
struct fm33_usart_info uart0_info = {
    .husart          = UART0,
    .index           = 0,
    .clock_src       = FL_RCC_UART0_CLK_SOURCE_APB1CLK,
    .baud_rate       = 0,

    .irqn            = UART0_IRQn,
    .irq_priority    = 2,

    .port            = GPIOA,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_13 | FL_GPIO_PIN_14,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_ENABLE,
        .remapPin    = FL_DISABLE,
     },

    .hdma            = DMA,

    .hdma_rx_channel = FL_DMA_CHANNEL_1,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION1,
     },
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart0", uart0_info);
#endif

#ifdef BSP_USING_UART1
struct fm33_usart_info uart1_info = {
    .husart          = UART1,
    .index           = 1,
    .clock_src       = FL_RCC_UART1_CLK_SOURCE_APB1CLK,
    .baud_rate       = 0,

    .irqn            = UART1_IRQn,
    .irq_priority    = 2,

    .port            = GPIOB,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_13 | FL_GPIO_PIN_14,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_ENABLE,
        .remapPin    = FL_DISABLE,
     },

    .hdma            = DMA,

    .hdma_rx_channel = FL_DMA_CHANNEL_3,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION3,
     },
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart1", uart1_info);
#endif

#ifdef BSP_USING_UART4
struct fm33_usart_info uart4_info = {
    .husart          = UART4,
    .index           = 4,
    /*.clock_src       = FL_RCC_UART4_CLK_SOURCE_APB1CLK,*/
    .baud_rate       = 0,

    .irqn            = UART4_IRQn,
    .irq_priority    = 2,

    .port            = GPIOA,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_0 | FL_GPIO_PIN_1,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_ENABLE,
        .remapPin    = FL_DISABLE,
     },

    .hdma            = DMA,

    .hdma_rx_channel = FL_DMA_CHANNEL_2,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION3,
     },
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart4", uart4_info);
#endif

#ifdef BSP_USING_UART5
struct fm33_usart_info uart5_info = {
    .husart          = UART5,
    .index           = 5,
    /*.clock_src       = FL_RCC_UART5_CLK_SOURCE_APB1CLK,*/
    .baud_rate       = 0,

    .irqn            = UART5_IRQn,
    .irq_priority    = 2,

    .port            = GPIOD,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_0 | FL_GPIO_PIN_1,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_ENABLE,
        .remapPin    = FL_DISABLE,
     },

    .hdma            = DMA,

    .hdma_rx_channel = FL_DMA_CHANNEL_4,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION6,
     },
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart5", uart5_info);
#endif

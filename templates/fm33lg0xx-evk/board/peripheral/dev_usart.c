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
 * @brief       This file implements usart driver configuration for fm33
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_uart.h"

#ifdef BSP_USING_UART0
static const struct fm33_usart_info uart0_info = {
    .husart          = UART0,
    .index           = 0,
    .clock_src       = FL_CMU_UART0_CLK_SOURCE_APBCLK,
    .baud_rate       = 0,

    .irqn            = UART0_IRQn,
    .irq_priority    = 17,

    .port            = GPIOA,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_13 | FL_GPIO_PIN_14,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_DISABLE,
     },
#ifdef UART0_USE_DMA
    .hdma            = DMA,
#else
    .hdma            = OS_NULL,
#endif
    .hdma_rx_channel = FL_DMA_CHANNEL_1,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION2,
     },
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart0", uart0_info);
#endif

#ifdef BSP_USING_UART1
static const struct fm33_usart_info uart1_info = {
    .husart          = UART1,
    .index           = 1,
    .clock_src       = FL_CMU_UART1_CLK_SOURCE_APBCLK,
    .baud_rate       = 0,

    .irqn            = UART1_IRQn,
    .irq_priority    = 18,

    .port            = GPIOB,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_13 | FL_GPIO_PIN_14,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_DISABLE,
     },
#ifdef UART1_USE_DMA
    .hdma            = DMA,
#else
    .hdma            = OS_NULL,
#endif
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

#ifdef BSP_USING_UART3
static const struct fm33_usart_info uart3_info = {
    .husart          = UART3,
    .index           = 3,
    /*.clock_src       = FL_CMU_UART1_CLK_SOURCE_APBCLK,*/
    .baud_rate       = 0,

    .irqn            = UART3_IRQn,
    .irq_priority    = 18,

    .port            = GPIOB,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_0 | FL_GPIO_PIN_1,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_ENABLE,
     },
#ifdef UART3_USE_DMA
    .hdma            = DMA,
#else
    .hdma            = OS_NULL,
#endif
    .hdma_rx_channel = FL_DMA_CHANNEL_5,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION4,
     },
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart3", uart3_info);
#endif

#ifdef BSP_USING_UART4
static const struct fm33_usart_info uart4_info = {
    .husart          = UART4,
    .index           = 4,
    /*.clock_src       = FL_CMU_UART1_CLK_SOURCE_APBCLK,*/
    .baud_rate       = 0,

    .irqn            = UART4_IRQn,
    .irq_priority    = 18,

    .port            = GPIOB,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_2 | FL_GPIO_PIN_3,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_DISABLE,
     },
#ifdef UART4_USE_DMA
    .hdma            = DMA,
#else
    .hdma            = OS_NULL,
#endif
    .hdma_rx_channel = FL_DMA_CHANNEL_2,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION4,
     },
};
OS_HAL_DEVICE_DEFINE("Usart_Type", "uart4", uart4_info);
#endif

#ifdef BSP_USING_UART5
static const struct fm33_usart_info uart5_info = {
    .husart          = UART5,
    .index           = 5,
    /*.clock_src       = FL_CMU_UART1_CLK_SOURCE_APBCLK,*/
    .baud_rate       = 0,

    .irqn            = UART5_IRQn,
    .irq_priority    = 18,

    .port            = GPIOC,
    .GPIO_InitStruct = {
        .pin         = FL_GPIO_PIN_4 | FL_GPIO_PIN_5,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_DISABLE,
     },
#ifdef UART5_USE_DMA
    .hdma            = DMA,
#else
    .hdma            = OS_NULL,
#endif
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



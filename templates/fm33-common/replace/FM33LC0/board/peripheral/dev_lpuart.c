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
 * @file        dev_lpuart.c
 *
 * @brief       This file implements lpuart selection for fm33
 *
 * @revision
 * Date         Author          Notes
 * 202-07-30   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_lpuart.h"

#ifdef BSP_USING_LPUART0
static const struct fm33_lpuart_info lpuart0_info = {
    .hlpuart          = LPUART0,
    .index           = 0,
    .clock_src       = FL_RCC_LPUART_CLK_SOURCE_LSCLK,
    .baud_rate       = FL_LPUART_BAUDRATE_9600,

    .irqn            = LPUART0_IRQn,
    .irq_priority    = 2,

    .port            = GPIOA,
    .GPIO_InitStruct_rx_pin = {
        .pin         = FL_GPIO_PIN_2,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_ENABLE,
        .remapPin    = FL_ENABLE,
     },
	 .GPIO_InitStruct_tx_pin = {
        .pin         = FL_GPIO_PIN_3,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_ENABLE,
     },
#ifdef LPUART0_USE_DMA
    .hdma            = DMA,
#else
    .hdma            = OS_NULL,
#endif
    .hdma_rx_channel = FL_DMA_CHANNEL_0,
    .dmaInitStruct_rx= {
        .circMode    = FL_ENABLE,
        .dataSize    = FL_DMA_BANDWIDTH_8B,
        .direction   = FL_DMA_DIR_PERIPHERAL_TO_RAM,
        .memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE,
        .priority    = FL_DMA_PRIORITY_HIGH,
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION2,
     },
};
OS_HAL_DEVICE_DEFINE("Lpuart_Type", "lpuart0", lpuart0_info);
#endif

#ifdef BSP_USING_LPUART1
static const struct fm33_lpuart_info lpuart1_info = {
    .hlpuart         = LPUART1,
    .index           = 1,
    .clock_src       = FL_CMU_LPUART_CLK_SOURCE_LSCLK,
    .baud_rate       = FL_LPUART_BAUDRATE_9600,

    .irqn            = LPUART1_IRQn,
    .irq_priority    = 2,

    .port            = GPIOC,
    .GPIO_InitStruct_rx_pin = {
        .pin         = FL_GPIO_PIN_2,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_ENABLE,
        .remapPin    = FL_ENABLE,
     },
	 .GPIO_InitStruct_tx_pin = {
        .pin         = FL_GPIO_PIN_3,
        .mode        = FL_GPIO_MODE_DIGITAL,
        .outputType  = FL_GPIO_OUTPUT_PUSHPULL,
        .pull        = FL_DISABLE,
        .remapPin    = FL_ENABLE,
     },
#ifdef LPUART1_USE_DMA
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
        .periphAddress = FL_DMA_PERIPHERAL_FUNCTION5,
     },
};
OS_HAL_DEVICE_DEFINE("Lpuart_Type", "lpuart1", lpuart1_info);
#endif

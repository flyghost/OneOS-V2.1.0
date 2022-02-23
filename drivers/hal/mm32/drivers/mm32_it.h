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
 * @file        mm32_it.h
 *
 * @brief       This file provides systick IRQ declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __MM32_IT__
#define __MM32_IT__

#include "mm32_hal.h"
#include "mm32_it_dma.h"
#include <os_types.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef NVIC_PriorityGroup_4
#define NVIC_PriorityGroup NVIC_PriorityGroup_4
#endif


void HAL_NVIC_SetPriorityGrouping(void);
void SysTick_Handler(void);
void HAL_UART_IRQHandler(UART_TypeDef *huart);
void HAL_TIM_IRQHandler(TIM_TypeDef *htim);
#ifdef __cplusplus
}
#endif

#endif /* __MM32F327x_IT__ */


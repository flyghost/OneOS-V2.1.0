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
 * @file        mm32l3xx_it.c
 *
 * @brief       This file provides systick time init/IRQ functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mm32_hal.h"

#include "os_clock.h"
#include "os_stddef.h"
#include "oneos_config.h"

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#endif

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.irq"
#include <drv_log.h>

#include "drv_gpio.h"
#include "mm32_it.h"

void HAL_NVIC_SetPriorityGrouping(void)
{
#if defined(SERIES_MM32F103XX) || defined(SERIES_MM32F327XX) || defined(SERIES_MM32L3XX)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup);
#endif
}

void SysTick_Handler(void)
{
    os_tick_increase();

#ifdef OS_USING_CLOCKSOURCE	
    os_clocksource_update();
#endif
}

OS_WEAK void HAL_UART_IRQHandler(UART_TypeDef *huart) {}
OS_WEAK void HAL_TIM_IRQHandler(TIM_TypeDef *htim) {}


void UART1_IRQHandler(void)
{
#ifdef UART1
    HAL_UART_IRQHandler(UART1);
#endif
}
void UART2_IRQHandler(void)
{
#ifdef UART2
    HAL_UART_IRQHandler(UART2);
#endif
}
void UART3_IRQHandler(void)
{
#ifdef UART3
    HAL_UART_IRQHandler(UART3);
#endif
}
void UART4_IRQHandler(void)
{
#ifdef UART4
    HAL_UART_IRQHandler(UART4);
#endif
}
void UART5_IRQHandler(void)
{
#ifdef UART5
    HAL_UART_IRQHandler(UART5);
#endif
}
void UART6_IRQHandler(void)
{
#ifdef UART6
    HAL_UART_IRQHandler(UART6);
#endif
}
void UART7_IRQHandler(void)
{
#ifdef UART7
    HAL_UART_IRQHandler(UART7);
#endif
}
void UART8_IRQHandler(void)
{
#ifdef UART8
    HAL_UART_IRQHandler(UART8);
#endif
}

void DMA1_Channel1_IRQHandler(void)
{
#ifdef DMA1_Channel1
    HAL_DMA_IRQHandler(DMA1_Channel1, DMA1_IT_GL1);
#endif
}
void DMA1_Channel2_IRQHandler(void)
{
#ifdef DMA1_Channel2
    HAL_DMA_IRQHandler(DMA1_Channel2, DMA1_IT_GL2);
#endif
}
void DMA1_Channel3_IRQHandler(void)
{
#ifdef DMA1_Channel3
    HAL_DMA_IRQHandler(DMA1_Channel3, DMA1_IT_GL3);
#endif
}
void DMA1_Channel4_IRQHandler(void)
{
#ifdef DMA1_Channel4
    HAL_DMA_IRQHandler(DMA1_Channel4, DMA1_IT_GL4);
#endif
}
void DMA1_Channel5_IRQHandler(void)
{
#ifdef DMA1_Channel5
    HAL_DMA_IRQHandler(DMA1_Channel5, DMA1_IT_GL5);
#endif
}
void DMA1_Channel6_IRQHandler(void)
{
#ifdef DMA1_Channel6
    HAL_DMA_IRQHandler(DMA1_Channel6, DMA1_IT_GL6);
#endif
}
void DMA1_Channel7_IRQHandler(void)
{
#ifdef DMA1_Channel7
    HAL_DMA_IRQHandler(DMA1_Channel7, DMA1_IT_GL7);
#endif
}
void DMA1_Channel2_3_IRQHandler(void)
{
#ifdef DMA1_Channel2
    HAL_DMA_IRQHandler(DMA1_Channel2, DMA1_IT_GL2);
#endif
#ifdef DMA1_Channel3
    HAL_DMA_IRQHandler(DMA1_Channel3, DMA1_IT_GL3);
#endif
}

void DMA2_Channel1_IRQHandler(void)
{
#ifdef DMA2_Channel1
    HAL_DMA_IRQHandler(DMA2_Channel1, DMA2_IT_GL1);
#endif
}
void DMA2_Channel2_IRQHandler(void)
{
#ifdef DMA2_Channel2
    HAL_DMA_IRQHandler(DMA2_Channel2, DMA2_IT_GL2);
#endif
}
void DMA2_Channel3_IRQHandler(void)
{
#ifdef DMA2_Channel3
    HAL_DMA_IRQHandler(DMA2_Channel3, DMA2_IT_GL3);
#endif
}
void DMA2_Channel4_IRQHandler(void)
{
#ifdef DMA2_Channel4
    HAL_DMA_IRQHandler(DMA2_Channel4, DMA2_IT_GL4);
#endif
}
void DMA2_Channel5_IRQHandler(void)
{
#ifdef DMA2_Channel5
    HAL_DMA_IRQHandler(DMA2_Channel5, DMA2_IT_GL5);
#endif
}
void DMA2_Channel6_IRQHandler(void)
{
#ifdef DMA2_Channel6
    HAL_DMA_IRQHandler(DMA2_Channel6, DMA2_IT_GL6);
#endif
}
void DMA2_Channel7_IRQHandler(void)
{
#ifdef DMA2_Channel7
    HAL_DMA_IRQHandler(DMA2_Channel7, DMA2_IT_GL7);
#endif
}


void EXTI0_IRQHandler(void)
{
    pin_irq_hdr(0);
}
void EXTI1_IRQHandler(void)
{
    pin_irq_hdr(1);
}
void EXTI2_IRQHandler(void)
{
    pin_irq_hdr(2);
}
void EXTI3_IRQHandler(void)
{
    pin_irq_hdr(3);
}
void EXTI4_IRQHandler(void)
{
    pin_irq_hdr(4);
}
void EXTI0_1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
        pin_irq_hdr(0);
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
        pin_irq_hdr(1);
}
void EXTI2_3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
        pin_irq_hdr(2);
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
        pin_irq_hdr(3);
}
void EXTI4_15_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
        pin_irq_hdr(4);
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
        pin_irq_hdr(5);
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
        pin_irq_hdr(6);
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
        pin_irq_hdr(7);
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
        pin_irq_hdr(8);
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
        pin_irq_hdr(9);
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
        pin_irq_hdr(10);
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
        pin_irq_hdr(11);
    if (EXTI_GetITStatus(EXTI_Line12) != RESET)
        pin_irq_hdr(12);
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
        pin_irq_hdr(13);
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
        pin_irq_hdr(14);
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)
        pin_irq_hdr(15);
}
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
        pin_irq_hdr(5);
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
        pin_irq_hdr(6);
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
        pin_irq_hdr(7);
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
        pin_irq_hdr(8);
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
        pin_irq_hdr(9);
}
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
        pin_irq_hdr(10);
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
        pin_irq_hdr(11);
    if (EXTI_GetITStatus(EXTI_Line12) != RESET)
        pin_irq_hdr(12);
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
        pin_irq_hdr(13);
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
        pin_irq_hdr(14);
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)
        pin_irq_hdr(15);
}

void TIM1_BRK_IRQHandler(void)
{
#ifdef TIM1
    HAL_TIM_IRQHandler(TIM1);
#endif
}
void TIM1_UP_IRQHandler(void)
{
#ifdef TIM1
    HAL_TIM_IRQHandler(TIM1);
#endif
}
void TIM1_TRG_COM_IRQHandler(void)
{
#ifdef TIM1
    HAL_TIM_IRQHandler(TIM1);
#endif
}
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
#ifdef TIM1
    HAL_TIM_IRQHandler(TIM1);
#endif
}
void TIM1_CC_IRQHandler(void)
{
#ifdef TIM1
    HAL_TIM_IRQHandler(TIM1);
#endif
}
void TIM2_IRQHandler(void)
{
#ifdef TIM2
    HAL_TIM_IRQHandler(TIM2);
#endif
}
void TIM3_IRQHandler(void)
{
#ifdef TIM3
    HAL_TIM_IRQHandler(TIM3);
#endif
}
void TIM4_IRQHandler(void)
{
#ifdef TIM4
    HAL_TIM_IRQHandler(TIM4);
#endif
}
void TIM5_IRQHandler(void)
{
#ifdef TIM5
    HAL_TIM_IRQHandler(TIM5);
#endif
}
void TIM6_IRQHandler(void)
{
#ifdef TIM6
    HAL_TIM_IRQHandler(TIM6);
#endif
}
void TIM7_IRQHandler(void)
{
#ifdef TIM7
    HAL_TIM_IRQHandler(TIM7);
#endif
}
void TIM8_BRK_IRQHandler(void)
{
#ifdef TIM8
    HAL_TIM_IRQHandler(TIM8);
#endif
}
void TIM8_TRG_COM_IRQHandler(void)
{
#ifdef TIM8
    HAL_TIM_IRQHandler(TIM8);
#endif
}
void TIM8_UP_IRQHandler(void)
{
#ifdef TIM8
    HAL_TIM_IRQHandler(TIM8);
#endif
}
void TIM8_CC_IRQHandler(void)
{
#ifdef TIM8
    HAL_TIM_IRQHandler(TIM8);
#endif
}



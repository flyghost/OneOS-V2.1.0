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
 * @brief       This file implements usart driver for hk32
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifdef BSP_USING_TIMER
#include "drv_hwtimer.h"
#endif

#ifdef BSP_USING_TIMER1
static const struct hk32_timer_info timer1_info =
{
    .htim = TIM1,
    .rcc_tpye = HK32_RCC_APB2,
    .rcc  = RCC_APB2Periph_TIM1,
    .irq  = TIM1_UP_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer1", timer1_info);
#endif

#ifdef BSP_USING_TIMER2
static const struct hk32_timer_info timer2_info =
{
    .htim = TIM2,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM2,
    .irq  = TIM2_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer2", timer2_info);
#endif

#ifdef BSP_USING_TIMER3
static const struct hk32_timer_info timer3_info =
{
    .htim = TIM3,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM3,
    .irq  = TIM3_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer3", timer3_info);
#endif

#ifdef BSP_USING_TIMER4
static const struct hk32_timer_info timer4_info =
{
    .htim = TIM4,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM4,
    .irq  = TIM4_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer4", timer4_info);
#endif

#ifdef BSP_USING_TIMER5
static const struct hk32_timer_info timer5_info =
{
    .htim = TIM5,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM5,
    .irq  = TIM5_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer5", timer5_info);
#endif

#ifdef BSP_USING_TIMER6
static const struct hk32_timer_info timer6_info =
{
    .htim = TIM6,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM6,
    .irq  = TIM6_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer6", timer6_info);
#endif

#ifdef BSP_USING_TIMER7
static const struct hk32_timer_info timer7_info =
{
    .htim = TIM7,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM7,
    .irq  = TIM7_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer7", timer7_info);
#endif

#ifdef BSP_USING_TIMER8
static const struct hk32_timer_info timer8_info =
{
    .htim = TIM8,
    .rcc_tpye = HK32_RCC_APB2,
    .rcc  = RCC_APB2Periph_TIM8,
    .irq  = TIM8_UP_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer8", timer8_info);
#endif

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

#ifdef BSP_USING_TIMER14
static const struct hk32_timer_info timer1_info =
{
    .htim = TIM1,
    .rcc_tpye = HK32_RCC_APB2,
    .rcc  = RCC_APB2Periph_TIM1,
    .irq  = TIM14_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer1", timer1_info);
#endif

#ifdef BSP_USING_TIMER15
static const struct hk32_timer_info timer2_info =
{
    .htim = TIM2,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM2,
    .irq  = TIM15_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer2", timer2_info);
#endif

#ifdef BSP_USING_TIMER16
static const struct hk32_timer_info timer3_info =
{
    .htim = TIM3,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM3,
    .irq  = TIM16_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer3", timer3_info);
#endif

#ifdef BSP_USING_TIMER17
static const struct hk32_timer_info timer4_info =
{
    .htim = TIM4,
    .rcc_tpye = HK32_RCC_APB1,
    .rcc  = RCC_APB1Periph_TIM4,
    .irq  = TIM17_IRQn,
    
};
OS_HAL_DEVICE_DEFINE("TIM_HandleTypeDef", "timer4", timer4_info);
#endif

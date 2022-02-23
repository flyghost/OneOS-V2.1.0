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
 * @brief       This file define the information of TIMER device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_hwtimer.h"
#include "drv_lptim.h"

#ifdef BSP_USING_ATIM
const struct fm33_timer_info atimer_info = {
    .instance   = ATIM,
    .index      = 0,
    .irqn       = ATIM_IRQn,
    .type       = TYPE_ATIM,
    .mode       = TIMER_MODE_TIM,
    .bits       = 16,
};
OS_HAL_DEVICE_DEFINE("TIM_Type", "atimer", atimer_info);
#endif

#ifdef BSP_USING_BSTIM32
const struct fm33_timer_info btimer_info = {
    .instance   = BSTIM32,
    .index      = 0,
    .irqn       = BSTIM_IRQn,
    .type       = TYPE_BTIM,
    .mode       = TIMER_MODE_TIM,
    .bits       = 32,
};
OS_HAL_DEVICE_DEFINE("TIM_Type", "btimer", btimer_info);
#endif

#ifdef BSP_USING_GPTIM0
const struct fm33_timer_info gtimer0_info = {
    .instance   = GPTIM0,
    .index      = 0,
    .irqn       = GPTIM0_IRQn,
    .type       = TYPE_GPTIM,
    .mode       = TIMER_MODE_TIM,
    .bits       = 16,
};
OS_HAL_DEVICE_DEFINE("TIM_Type", "gtimer0", gtimer0_info);
#endif

#ifdef BSP_USING_GPTIM1
const struct fm33_timer_info gtimer1_info = {
    .instance   = GPTIM1,
    .index      = 1,
    .irqn       = GPTIM1_IRQn,
    .type       = TYPE_GPTIM,
    .mode       = TIMER_MODE_TIM,
    .bits       = 16,
};
OS_HAL_DEVICE_DEFINE("TIM_Type", "gtimer1", gtimer1_info);
#endif

#ifdef BSP_USING_LPTIM32
const struct fm33_lptimer_info lptimer_info = {
	.instance   = LPTIM32,
	.index      = 0,
	.irqn       = LPTIM_IRQn,
};
OS_HAL_DEVICE_DEFINE("LPTIM_Type", "lptimer", lptimer_info);
#endif



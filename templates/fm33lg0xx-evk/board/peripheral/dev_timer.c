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
 * @file        dev_timer.c
 *
 * @brief       This file implements hardware timer driver configuration for fm33
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifdef BSP_USING_TIM

#include <drv_hwtimer.h>

#ifdef BSP_USING_ATIM
static const struct fm33_timer_info atim_info =
{
    .type          = TYPE_ATIM,
    .inst          = (void *)ATIM,
    .irqn          = ATIM_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "atim", atim_info);
#endif

#ifdef BSP_USING_BSTIM16
static const struct fm33_timer_info bstim16_info =
{
    .type          = TYPE_BSTIM16,
    .inst          = (void *)BSTIM16,
    .irqn          = BSTIM_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "bstim16", bstim16_info);
#endif


#ifdef BSP_USING_BSTIM32
static const struct fm33_timer_info bstim32_info =
{
    .type          = TYPE_BSTIM32,
    .inst          = (void *)BSTIM32,
    .irqn          = BSTIM_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "bstim32", bstim32_info);
#endif

#ifdef BSP_USING_GPTIM0
static const struct fm33_timer_info gptim0_info =
{
    .type          = TYPE_GPTIM0,
    .inst          = (void *)GPTIM0,
    .irqn          = GPTIM01_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "gptim0", gptim0_info);
#endif

#ifdef BSP_USING_GPTIM1
static const struct fm33_timer_info gptim1_info =
{
    .type          = TYPE_GPTIM1,
    .inst          = (void *)GPTIM1,
    .irqn          = GPTIM01_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "gptim1", gptim1_info);
#endif

#ifdef BSP_USING_GPTIM2
static const struct fm33_timer_info gptim2_info =
{
    .type          = TYPE_GPTIM2,
    .inst          = (void *)GPTIM2,
    .irqn          = GPTIM2_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "gptim2", gptim2_info);
#endif

#ifdef BSP_USING_LPTIM16
static const struct fm33_timer_info lptim16_info =
{
    .type          = TYPE_LPTIM16,
    .inst          = (void *)LPTIM16,
    .irqn          = LPTIMx_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "lptim16", lptim16_info);
#endif

#ifdef BSP_USING_LPTIM32
static const struct fm33_timer_info lptim32_info =
{
    .type          = TYPE_LPTIM32,
    .inst          = (void *)LPTIM32,
    .irqn          = LPTIMx_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "lptim32", lptim32_info);
#endif


#endif /*BSP_USING_TIM*/

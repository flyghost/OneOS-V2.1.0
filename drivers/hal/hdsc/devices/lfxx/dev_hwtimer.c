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
 * @file        dev_hwtimer.c
 *
 * @brief       This file implements hwtimer driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <hc_bt.h>
#include <drv_hwtimer.h>

#ifdef OS_USING_TIMER_DRIVER
static const struct hc32_tim_info tim0_info =
{
    .mode = TIMER_MODE_TIM,
    .unit = TIM0,
    .irqn = TIM0_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "tim0", tim0_info);

static const struct hc32_tim_info tim1_info =
{
    .mode = TIMER_MODE_TIM,
    .unit = TIM1,
    .irqn = TIM1_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "tim1", tim1_info);
#endif

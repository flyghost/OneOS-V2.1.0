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
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_hwtimer.h"

#ifdef BSP_USING_TIMER0
static const struct nrf5_timer_info timer0_info = {NRFX_TIMER_INSTANCE(0), NRF_TIMER_CC_CHANNEL0, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer0", timer0_info);
#endif

#ifdef BSP_USING_TIMER1
static const struct nrf5_timer_info timer1_info = {NRFX_TIMER_INSTANCE(1), NRF_TIMER_CC_CHANNEL0, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer1", timer1_info);
#endif

#ifdef BSP_USING_TIMER2
static const struct nrf5_timer_info timer2_info = {NRFX_TIMER_INSTANCE(2), NRF_TIMER_CC_CHANNEL0, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer2", timer2_info);
#endif

#ifdef BSP_USING_TIMER3
static const struct nrf5_timer_info timer3_info = {NRFX_TIMER_INSTANCE(3), NRF_TIMER_CC_CHANNEL0, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer3", timer3_info);
#endif


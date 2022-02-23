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
#ifdef BSP_USING_TIMER
static const struct gd32_timer_info timer1_info =
{
    .timer_periph   = TIMER1,
    .periph         = RCU_TIMER1,
    .nvic_irq       = TIMER1_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer1", timer1_info);

static const struct gd32_timer_info timer2_info =
{
    .timer_periph   = TIMER2,
    .periph         = RCU_TIMER2,
    .nvic_irq       = TIMER2_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer2", timer2_info);
#if 0
static const struct gd32_timer_info timer3_info =
{
    .timer_periph   = TIMER3,
    .periph         = RCU_TIMER3,
    .nvic_irq       = TIMER3_IRQn,
};
OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer3", timer3_info);

static const struct gd32_timer_info timer4_info =
{
    .timer_periph   = TIMER4,
    .periph         = RCU_TIMER4,
    .nvic_irq       = TIMER4_IRQn,
};

OS_HAL_DEVICE_DEFINE("TIMER_Type", "timer4", timer4_info);
#endif
#endif

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
 * @file        drv_pwm.c
 *
 * @brief       This file implements pwm driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include "am_mcu_apollo.h"

#ifdef OS_USING_PWM

/* LED3 is pin 19 */
#define AM_BSP_GPIO_PWM_LED     19
#define AM_BSP_GPIO_CFG_PWM_LED AM_HAL_PIN_19_TCTB1

#define AM_BSP_PWM_LED_TIMER     1
#define AM_BSP_PWM_LED_TIMER_SEG AM_HAL_CTIMER_TIMERB
#define AM_BSP_PWM_LED_TIMER_INT AM_HAL_CTIMER_INT_TIMERB1

volatile uint32_t g_ui32Index           = 0;
const uint32_t g_pui32Brightness[64] =
{
    1, 1, 1, 2, 3, 4, 6, 8,
    10, 12, 14, 17, 20, 23, 25, 28,
    31, 35, 38, 40, 43, 46, 49, 51,
    53, 55, 57, 59, 60, 61, 62, 62,
    63, 62, 62, 61, 60, 59, 57, 55,
    53, 51, 49, 46, 43, 40, 38, 35,
    32, 28, 25, 23, 20, 17, 14, 12,
    10, 8, 6, 4, 3, 2, 1, 1
};

void am_ctimer_isr(void)
{
    /* Clear the interrupt that got us here */
    am_hal_ctimer_int_clear(AM_BSP_PWM_LED_TIMER_INT);

    /* Now set new PWM half-period for the LED */
    am_hal_ctimer_period_set(AM_BSP_PWM_LED_TIMER, AM_BSP_PWM_LED_TIMER_SEG, 64, g_pui32Brightness[g_ui32Index]);

    /* Set up the LED duty cycle for the next pulse */
    g_ui32Index = (g_ui32Index + 1) % 64;
}

int os_hw_pwm_init(void)
{
    /* init pwm gpio */
    am_hal_gpio_pin_config(AM_BSP_GPIO_PWM_LED, AM_BSP_GPIO_CFG_PWM_LED);

    /* Configure a timer to drive the LED */
    am_hal_ctimer_config_single(AM_BSP_PWM_LED_TIMER,
                                AM_BSP_PWM_LED_TIMER_SEG,
                                (AM_HAL_CTIMER_FN_PWM_REPEAT |
                                 AM_HAL_CTIMER_XT_2_048KHZ |
                                 AM_HAL_CTIMER_INT_ENABLE |
                                 AM_HAL_CTIMER_PIN_ENABLE));

    /* Set up initial timer period */
    am_hal_ctimer_period_set(AM_BSP_PWM_LED_TIMER, AM_BSP_PWM_LED_TIMER_SEG, 64, 32);

    /* Enable interrupts for the Timer we are using on this board */
    am_hal_ctimer_int_enable(AM_BSP_PWM_LED_TIMER_INT);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);

    /* Start the timer */
    am_hal_ctimer_start(AM_BSP_PWM_LED_TIMER, AM_BSP_PWM_LED_TIMER_SEG);

    os_kprintf("pwm_init!\n");

    return 0;
}
#ifdef OS_USING_COMPONENTS_INIT
INIT_BOARD_EXPORT(os_hw_pwm_init);
#endif

#endif

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
 * @file        drv_hwtimer.c
 *
 * @brief       This file implements timer driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <timer/timer.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#include <drv_hwtimer.h>

#ifdef OS_USING_PWM
#include "drv_pwm.h"
#endif

#ifdef OS_USING_PULSE_ENCODER
#include "drv_pulse_encoder.h"
#endif

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>

#if defined(SERIES_STM32F1)
#define TIM1_LINE2
#elif defined(SERIES_STM32F2)
#define TIM1_LINE2
#define TIM8_LINE2
#define TIM9_LINE2
#define TIM10_LINE2
#define TIM11_LINE2
#elif defined(SERIES_STM32F3)
#define TIM1_LINE2
#define TIM15_LINE2
#define TIM16_LINE2
#define TIM17_LINE2
#elif defined(SERIES_STM32F4)
#define TIM1_LINE2
#define TIM8_LINE2
#define TIM9_LINE2
#define TIM10_LINE2
#define TIM11_LINE2
#elif defined(SERIES_STM32F7)
#define TIM1_LINE2
#define TIM8_LINE2
#define TIM9_LINE2
#define TIM10_LINE2
#define TIM11_LINE2
#elif defined(SERIES_STM32G0)
#define TIM1_LINE2
#define TIM8_LINE2
#define TIM15_LINE2
#define TIM16_LINE2
#define TIM17_LINE2
#elif defined(SERIES_STM32L0)
#define TIM21_LINE2
#define TIM22_LINE2
#elif defined(SERIES_STM32L1)
#define TIM9_LINE2
#define TIM10_LINE2
#define TIM11_LINE2
#elif defined(SERIES_STM32L4) || defined(SERIES_STM32MP1)|| defined(SERIES_STM32L5)
#define TIM1_LINE2
#define TIM8_LINE2
#define TIM15_LINE2
#define TIM16_LINE2
#define TIM17_LINE2
#endif /* TIM_CLK_LINE */

#define TIMER_MODE_TIM 0x00
#define TIMER_MODE_PWM 0x01
#define TIMER_MODE_PULSE_ENCODER 0x02

static os_list_node_t stm32_timer_list = OS_LIST_INIT(stm32_timer_list);

os_bool_t stm32_timer_is_32b(TIM_TypeDef *Instance)
{
#ifdef IS_TIM_32B_COUNTER_INSTANCE
    return IS_TIM_32B_COUNTER_INSTANCE(Instance);
#else
#if defined(SERIES_STM32F1) || defined(SERIES_STM32L0)
    //return OS_FALSE;
#elif defined(SERIES_STM32L1) || defined(SERIES_STM32L1Cat345)
    if (Instance == TIM5)
        return OS_TRUE;
#else
    if (Instance == TIM2 || Instance == TIM5)
        return OS_TRUE;
#endif
    return OS_FALSE;
#endif
}

#ifdef RCC_CLOCKTYPE_PCLK2
static os_uint8_t stm32_timer_line(TIM_TypeDef *Instance)
{
#if defined(TIM1) && defined(TIM1_LINE2)
    if (Instance == TIM1)
        return 2;
#endif
#if defined(TIM2) && defined(TIM2_LINE2)
    if (Instance == TIM2)
        return 2;
#endif
#if defined(TIM3) && defined(TIM3_LINE2)
    if (Instance == TIM3)
        return 2;
#endif
#if defined(TIM4) && defined(TIM4_LINE2)
    if (Instance == TIM4)
        return 2;
#endif
#if defined(TIM5) && defined(TIM5_LINE2)
    if (Instance == TIM5)
        return 2;
#endif
#if defined(TIM6) && defined(TIM6_LINE2)
    if (Instance == TIM6)
        return 2;
#endif
#if defined(TIM7) && defined(TIM7_LINE2)
    if (Instance == TIM7)
        return 2;
#endif
#if defined(TIM8) && defined(TIM8_LINE2)
    if (Instance == TIM8)
        return 2;
#endif
#if defined(TIM9) && defined(TIM9_LINE2)
    if (Instance == TIM9)
        return 2;
#endif
#if defined(TIM10) && defined(TIM10_LINE2)
    if (Instance == TIM10)
        return 2;
#endif
#if defined(TIM11) && defined(TIM11_LINE2)
    if (Instance == TIM11)
        return 2;
#endif
#if defined(TIM12) && defined(TIM12_LINE2)
    if (Instance == TIM12)
        return 2;
#endif
#if defined(TIM13) && defined(TIM13_LINE2)
    if (Instance == TIM13)
        return 2;
#endif
#if defined(TIM14) && defined(TIM14_LINE2)
    if (Instance == TIM14)
        return 2;
#endif
#if defined(TIM15) && defined(TIM15_LINE2)
    if (Instance == TIM15)
        return 2;
#endif
#if defined(TIM16) && defined(TIM16_LINE2)
    if (Instance == TIM16)
        return 2;
#endif
#if defined(TIM17) && defined(TIM17_LINE2)
    if (Instance == TIM17)
        return 2;
#endif
#if defined(TIM18) && defined(TIM18_LINE2)
    if (Instance == TIM18)
        return 2;
#endif
#if defined(TIM19) && defined(TIM19_LINE2)
    if (Instance == TIM19)
        return 2;
#endif
#if defined(TIM20) && defined(TIM20_LINE2)
    if (Instance == TIM20)
        return 2;
#endif
#if defined(TIM21) && defined(TIM21_LINE2)
    if (Instance == TIM21)
        return 2;
#endif
#if defined(TIM22) && defined(TIM22_LINE2)
    if (Instance == TIM22)
        return 2;
#endif

    return 1;
}
#endif

static uint32_t stm32_timer_get_freq(TIM_HandleTypeDef *tim)
{
    OS_ASSERT(tim != OS_NULL);

    os_uint32_t val_hclk;
    os_uint32_t val_pclk1;

#if defined(SERIES_STM32MP1)
    val_hclk  = HAL_RCC_GetMLHCLKFreq();
#else
    val_hclk  = HAL_RCC_GetHCLKFreq();
#endif
    val_pclk1 = HAL_RCC_GetPCLK1Freq();
    
#ifdef RCC_CLOCKTYPE_PCLK2
    if (stm32_timer_line(tim->Instance) == 2)
    {
        os_uint32_t val_pclk2 = HAL_RCC_GetPCLK2Freq();
        if (val_hclk == val_pclk2)
        {
            return val_pclk2;
        }
        else
        {
            return val_pclk2 * 2;
        }
    }
    else
#endif
    {
        if (val_hclk == val_pclk1)
        {
            return val_pclk1;
        }
        else
        {
            return val_pclk1 * 2;
        }
    }
}

static os_uint8_t stm32_timer_mode_judge(TIM_HandleTypeDef *tim)
{
    int i = 0;
    uint32_t tmpccmrx1 = 0;
    uint32_t tmpccmrx2 = 0;
    uint32_t tmpsmcr = 0;
    
#ifdef TIM_CCMR3_OC5FE
    uint32_t tmpccmrx3 = 0;
    tmpccmrx3 = tim->Instance->CCMR3;
#endif
    tmpccmrx1 = tim->Instance->CCMR1;
    tmpccmrx2 = tim->Instance->CCMR2;
    tmpsmcr = tim->Instance->SMCR;

    for (i = 0;i < 4;i++)
    {
        if ((tmpccmrx1 >> (i * 8)) & TIM_OCMODE_PWM1)
            return TIMER_MODE_PWM;
        if ((tmpccmrx1 >> (i * 8)) & TIM_OCMODE_PWM2)
            return TIMER_MODE_PWM;
        if ((tmpccmrx2 >> (i * 8)) & TIM_OCMODE_PWM1)
            return TIMER_MODE_PWM;
        if ((tmpccmrx2 >> (i * 8)) & TIM_OCMODE_PWM2)
            return TIMER_MODE_PWM;
#ifdef TIM_CCMR3_OC5FE
        if ((tmpccmrx3 >> (i * 8)) & TIM_OCMODE_PWM1)
            return TIMER_MODE_PWM;
        if ((tmpccmrx3 >> (i * 8)) & TIM_OCMODE_PWM2)
            return TIMER_MODE_PWM;
#endif
    }
    
    if ((tmpsmcr & TIM_ENCODERMODE_TI1) || (tmpsmcr & TIM_ENCODERMODE_TI2) || (tmpsmcr & TIM_ENCODERMODE_TI12))
        return TIMER_MODE_PULSE_ENCODER;
    else 
        return TIMER_MODE_TIM;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    struct stm32_timer *timer;
    
    os_list_for_each_entry(timer, &stm32_timer_list, struct stm32_timer, list)
    {
        if (timer->handle == htim)
        {
#ifdef OS_USING_CLOCKEVENT
            if (timer->mode == TIMER_MODE_TIM)
            {
                os_clockevent_isr((os_clockevent_t *)timer);
                return;
            }
#endif

#ifdef OS_USING_PULSE_ENCODER          
            if (timer->mode == TIMER_MODE_PULSE_ENCODER)
            {
                pulse_encoder_update_isr(timer);
                return;
            }
#endif      
            return;     
        }
    }
}

#if defined(OS_USING_CLOCKSOURCE) || defined(OS_USING_CLOCKEVENT)
static os_uint64_t stm32_timer_read(void *clock)
{
    struct stm32_timer *timer;

    timer = (struct stm32_timer *)clock;

    return __HAL_TIM_GET_COUNTER(timer->handle);
}

#endif

#ifdef OS_USING_CLOCKEVENT
static void stm32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct stm32_timer *timer;

    TIM_HandleTypeDef *tim;

    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer = (struct stm32_timer *)ce;
    tim = timer->handle;

    tim->Init.Prescaler     = prescaler - 1;
    tim->Init.Period        = count;
    tim->Init.CounterMode   = TIM_COUNTERMODE_UP;
    tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    __HAL_TIM_CLEAR_IT(tim, TIM_IT_UPDATE);
    HAL_TIM_Base_Init(tim);
    __HAL_TIM_CLEAR_IT(tim, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(tim);
}

static void stm32_timer_stop(os_clockevent_t *ce)
{
    struct stm32_timer *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct stm32_timer *)ce;

    HAL_TIM_Base_Stop_IT(timer->handle);
}

static const struct os_clockevent_ops stm32_tim_ops =
{
    .start = stm32_timer_start,
    .stop  = stm32_timer_stop,
    .read  = stm32_timer_read,
};
#endif

/**
 ***********************************************************************************************************************
 * @brief           stm32_tim_probe:probe timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int stm32_tim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_timer *timer;
    TIM_HandleTypeDef  *tim;

    timer = os_calloc(1, sizeof(struct stm32_timer));
    OS_ASSERT(timer);

    tim = (TIM_HandleTypeDef *)dev->info;
    
    timer->handle = tim;

    timer->freq = stm32_timer_get_freq(timer->handle);

    timer->mode = stm32_timer_mode_judge(timer->handle);

#ifdef OS_USING_PWM
    if (timer->mode == TIMER_MODE_PWM)
    {
        stm32_pwm_register(dev->name,timer);
    }
#endif
#ifdef OS_USING_PULSE_ENCODER
    if (timer->mode == TIMER_MODE_PULSE_ENCODER)
    {
        stm32_pulse_encoder_register(dev->name,timer);
    }
#endif
    if (timer->mode == TIMER_MODE_TIM)
    {
#ifdef OS_USING_CLOCKSOURCE
        if (os_clocksource_best() == OS_NULL && stm32_timer_is_32b(timer->handle->Instance))
        {
            tim->Init.Prescaler     = 0;
            tim->Init.Period        = 0xfffffffful;
            tim->Init.CounterMode   = TIM_COUNTERMODE_UP;
            tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

            HAL_TIM_Base_Init(tim);
            HAL_TIM_Base_Start(tim);
        
            timer->clock.cs.rating  = 320;
            timer->clock.cs.freq    = timer->freq;
            timer->clock.cs.mask    = 0xffffffffull;
            timer->clock.cs.read    = stm32_timer_read;

            os_clocksource_register(dev->name, &timer->clock.cs);
        }
        else
#endif
        {
#ifdef OS_USING_CLOCKEVENT
            timer->clock.ce.rating  = stm32_timer_is_32b(timer->handle->Instance) ? 320 : 160;
            timer->clock.ce.freq    = timer->freq;
            timer->clock.ce.mask    = 0xffffffffull;
            
            timer->clock.ce.prescaler_mask = 0xfffful;
            timer->clock.ce.prescaler_bits = 16;
            
            timer->clock.ce.count_mask = stm32_timer_is_32b(timer->handle->Instance) ? 0xfffffffful : 0xfffful;
            timer->clock.ce.count_bits = stm32_timer_is_32b(timer->handle->Instance) ? 32 : 16;

            timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

            timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
            
            timer->clock.ce.ops     = &stm32_tim_ops;
            os_clockevent_register(dev->name, &timer->clock.ce);
#endif
        }
    }

    os_list_add(&stm32_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO stm32_tim_driver = {
    .name   = "TIM_HandleTypeDef",
    .probe  = stm32_tim_probe,
};

OS_DRIVER_DEFINE(stm32_tim_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);


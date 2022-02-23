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
 * @brief       This file implements timer driver for fm33.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_hwtimer.h>

#include <os_memory.h>
#include <timer/timer.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>

/*uniform sdk timer interface*/
#define timer_init(TIMx,init_struct)         FL_##TIMx##_Init(##TIMx##, ##init_struct##)
#define timer_enable(TIMx)                   FL_##TIMx##_Enable(##TIMx##)
#define timer_enable_it_update(TIMx)         FL_##TIMx##_EnableIT_Update(##TIMx##)
#define timer_disable(TIMx)                  FL_##TIMx##_Disable(##TIMx##)
#define timer_read_counter(TIMx)             FL_##TIMx##_ReadCounter(##TIMx##)

#define gptimer_init(TIMx,init_struct)       FL_GPTIM_Init(##TIMx##, ##init_struct##)
#define gptimer_enable(TIMx)                 FL_GPTIM_Enable(##TIMx##)
#define gptimer_enable_it_update(TIMx)       FL_GPTIM_EnableIT_Update(##TIMx##)
#define gptimer_disable(TIMx)                FL_GPTIM_Disable(##TIMx##)
#define gptimer_read_counter(TIMx,x)         FL_GPTIM_ReadCounter(##TIMx##)

static os_list_node_t fm33_timer_list = OS_LIST_INIT(fm33_timer_list);

os_bool_t fm33_timer_is_32b(struct          fm33_timer *timer)
{
    if(timer->info->type == TYPE_BSTIM32)    /* LPTIM32 force to clockevent */
    {
        return OS_TRUE;
    }
    else 
    {
        return OS_FALSE;
    }
}

static uint32_t fm33_timer_get_freq(struct fm33_timer *timer)
{
    OS_ASSERT(timer != OS_NULL);

    os_uint32_t freq = FL_CMU_GetAPBClockFreq();

    return freq;
}

static os_uint8_t fm33_timer_mode_judge(struct fm33_timer *timer)
{
    return TIMER_MODE_TIM;
}

static void fm33_timer_update_callback(hwtimer_type_t type)
{
    struct fm33_timer *timer;

    os_list_for_each_entry(timer, &fm33_timer_list, struct fm33_timer, list)
    {
        if (timer->info->type == type && timer->clock.ce.parent.type == OS_DEVICE_TYPE_CLOCKEVENT)
        {
#ifdef OS_USING_CLOCKEVENT
            if (timer->mode == TIMER_MODE_TIM)
            {
                os_clockevent_isr((os_clockevent_t *)timer);
            }
#endif
        }
    }
}

/*timer irq handlers*/
#if defined (BSP_USING_ATIM)
void ATIM_IRQHandler()
{
    if(FL_ATIM_IsEnabledIT_Update(ATIM) && FL_ATIM_IsActiveFlag_Update(ATIM))
    {
        FL_ATIM_ClearFlag_Update(ATIM);
        fm33_timer_update_callback(TYPE_ATIM);
    }
}
#endif

#if defined (BSP_USING_BSTIM)
void BSTIM_IRQHandler()
{
    if(FL_BSTIM16_IsEnabledIT_Update(BSTIM16) && FL_BSTIM16_IsActiveFlag_Update(BSTIM16))
    {
        FL_BSTIM16_ClearFlag_Update(BSTIM16);
        fm33_timer_update_callback(TYPE_BSTIM16);
    }

    if(FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        FL_BSTIM32_ClearFlag_Update(BSTIM32);
        fm33_timer_update_callback(TYPE_BSTIM32);
    }
}
#endif

#if defined (BSP_USING_LPTIM)
void LPTIM_IRQHandler()
{
    if(FL_LPTIM16_IsEnabledIT_Update(LPTIM16) && FL_LPTIM16_IsActiveFlag_Update(LPTIM16))
    {
        FL_LPTIM16_ClearFlag_Update(LPTIM16);
        fm33_timer_update_callback(TYPE_LPTIM16);
    }

    if(FL_LPTIM32_IsEnabledIT_Update(LPTIM32) && FL_LPTIM32_IsActiveFlag_Update(LPTIM32))
    {
        FL_LPTIM32_ClearFlag_Update(LPTIM32);
        fm33_timer_update_callback(TYPE_LPTIM32);
    }
}
#endif

#if defined (BSP_USING_GPTIM0) || defined (BSP_USING_GPTIM1)
void GPTIM0_1_IRQHandler()
{
    if(FL_GPTIM_IsEnabledIT_Update(GPTIM0) && FL_GPTIM_IsActiveFlag_Update(GPTIM0))
    {
        FL_GPTIM_ClearFlag_Update(GPTIM0);
        fm33_timer_update_callback(TYPE_GPTIM0);
    }

    if(FL_GPTIM_IsEnabledIT_Update(GPTIM1) && FL_GPTIM_IsActiveFlag_Update(GPTIM1))
    {
        FL_GPTIM_ClearFlag_Update(GPTIM1);
        fm33_timer_update_callback(TYPE_GPTIM1);
    }
}
#endif

#if defined (BSP_USING_GPTIM2)
void GPTIM2_IRQHandler()
{
    if(FL_GPTIM_IsEnabledIT_Update(GPTIM2) && FL_GPTIM_IsActiveFlag_Update(GPTIM2))
    {
        FL_GPTIM_ClearFlag_Update(GPTIM2);
        fm33_timer_update_callback(TYPE_GPTIM2);
    }
}
#endif


static void hal_timer_start(struct fm33_timer *timer, os_uint32_t prescaler, os_uint64_t count, os_bool_t irq_enable)
{
    OS_ASSERT(timer != OS_NULL);
    OS_ASSERT(count != 0);

    switch(timer->info->type)
        {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
            {
                 FL_ATIM_InitTypeDef InitStruct;

                 InitStruct.clockSource          = FL_CMU_ATIM_CLK_SOURCE_APBCLK;
                 InitStruct.prescaler            = prescaler - 1;
                 InitStruct.counterMode          = FL_ATIM_COUNTER_DIR_UP;
                 InitStruct.autoReload           = count;
                 InitStruct.autoReloadState      = FL_DISABLE;
                 InitStruct.clockDivision        = FL_ATIM_CLK_DIVISION_DIV1;
                 InitStruct.repetitionCounter    = 0;
                 
                 FL_ATIM_Init(ATIM, &InitStruct);
                 FL_ATIM_ClearFlag_Update(ATIM);
                 if(irq_enable)
                 {
                    FL_ATIM_EnableIT_Update(ATIM);
                    NVIC_EnableIRQ(ATIM_IRQn);
                 }
                 FL_ATIM_Enable(ATIM);
            }
            break;
#endif

#if defined(BSP_USING_BSTIM16)
        case TYPE_BSTIM16:
            {
                  FL_BSTIM16_InitTypeDef defaultInitStruct; 

                 defaultInitStruct.prescaler       = prescaler - 1;
                 defaultInitStruct.autoReload      = count;
                 defaultInitStruct.autoReloadState = FL_ENABLE;
                 defaultInitStruct.clockSource     = FL_CMU_BSTIM16_CLK_SOURCE_APBCLK;

                 FL_BSTIM16_Init(BSTIM16, &defaultInitStruct );
                 FL_BSTIM16_ClearFlag_Update(BSTIM16);
                 if(irq_enable)
                 {
                     FL_BSTIM16_EnableIT_Update(BSTIM16 );
                     NVIC_EnableIRQ(BSTIM_IRQn);
                 }
                 FL_BSTIM16_Enable(BSTIM16);
            }
            break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BSTIM32:
            {
                  FL_BSTIM32_InitTypeDef defaultInitStruct; 

                 defaultInitStruct.prescaler       = prescaler - 1;
                 defaultInitStruct.autoReload      = count;
                 defaultInitStruct.autoReloadState = FL_ENABLE;
                 defaultInitStruct.clockSource     = FL_CMU_BSTIM32_CLK_SOURCE_APBCLK;

                 FL_BSTIM32_Init(BSTIM32, &defaultInitStruct );
                 FL_BSTIM32_ClearFlag_Update(BSTIM32);
                 if(irq_enable)
                 {
                     FL_BSTIM32_EnableIT_Update(BSTIM32 );
                     NVIC_EnableIRQ(BSTIM_IRQn);
                 }
                 FL_BSTIM32_Enable(BSTIM32);
            }
            break;
#endif

#if defined(BSP_USING_GPTIM)
        case TYPE_GPTIM0:
        case TYPE_GPTIM1:
        case TYPE_GPTIM2:
            {
                FL_GPTIM_InitTypeDef    TimerBaseInitStruct;
                GPTIM_Type             *inst;

                inst = (GPTIM_Type *)(timer->info->inst);

                TimerBaseInitStruct.prescaler       = prescaler - 1;
                TimerBaseInitStruct.counterMode     = FL_GPTIM_COUNTER_DIR_UP;
                TimerBaseInitStruct.autoReload      = count;
                TimerBaseInitStruct.autoReloadState = FL_DISABLE;
                TimerBaseInitStruct.clockDivision   = FL_GPTIM_CLK_DIVISION_DIV1;

                FL_GPTIM_Init(inst, &TimerBaseInitStruct );
                FL_GPTIM_ClearFlag_Update(inst);
                if(irq_enable)
                {
                    FL_GPTIM_EnableIT_Update(inst);
                    NVIC_EnableIRQ(timer->info->irqn);
                }
                FL_GPTIM_Enable(inst);
            }
            break;
#endif

#if defined(BSP_USING_LPTIM16)
        case TYPE_LPTIM16:
            {
                FL_LPTIM16_InitTypeDef    defaultInitStruct;

                defaultInitStruct.clockSource          = FL_CMU_LPTIM16_CLK_SOURCE_APBCLK;
                defaultInitStruct.mode                 = FL_LPTIM16_OPERATION_MODE_NORMAL;
                defaultInitStruct.prescalerClockSource = FL_LPTIM16_CLK_SOURCE_INTERNAL;
                defaultInitStruct.prescaler            = FL_LPTIM16_PSC_DIV1;
                defaultInitStruct.autoReload           = count;
                defaultInitStruct.encoderMode          = FL_LPTIM16_ENCODER_MODE_DISABLE;
                defaultInitStruct.onePulseMode         = FL_LPTIM16_ONE_PULSE_MODE_CONTINUOUS;
                defaultInitStruct.triggerEdge          = FL_LPTIM16_ETR_TRIGGER_EDGE_RISING;
                defaultInitStruct.countEdge            = FL_LPTIM16_ETR_COUNT_EDGE_RISING;

                FL_LPTIM16_Init(LPTIM16,&defaultInitStruct );
                FL_LPTIM16_ClearFlag_Update(LPTIM16);
                if(irq_enable)
                {
                    FL_LPTIM16_EnableIT_Update(LPTIM16 );
                    NVIC_EnableIRQ(LPTIMx_IRQn);
                }
                FL_LPTIM16_Enable(LPTIM16);
            }
            break;
#endif

#if defined(BSP_USING_LPTIM32)
        case TYPE_LPTIM32:
            {
                FL_LPTIM32_InitTypeDef    defaultInitStruct;

                defaultInitStruct.clockSource          = FL_CMU_LPTIM32_CLK_SOURCE_APBCLK;
                defaultInitStruct.mode                 = FL_LPTIM32_OPERATION_MODE_NORMAL;
                defaultInitStruct.prescalerClockSource = FL_LPTIM32_CLK_SOURCE_INTERNAL;
                defaultInitStruct.prescaler            = FL_LPTIM32_PSC_DIV128;
                defaultInitStruct.autoReload           = count;
                defaultInitStruct.onePulseMode         = FL_LPTIM32_ONE_PULSE_MODE_CONTINUOUS;
                defaultInitStruct.triggerEdge          = FL_LPTIM32_ETR_TRIGGER_EDGE_RISING;
                defaultInitStruct.countEdge            = FL_LPTIM32_ETR_COUNT_EDGE_RISING;
                
                FL_LPTIM32_Init(LPTIM32,&defaultInitStruct );
                FL_LPTIM32_ClearFlag_Update(LPTIM32);
                if(irq_enable)
                {
                    FL_LPTIM32_EnableIT_Update(LPTIM32 );
                    NVIC_EnableIRQ(LPTIMx_IRQn);
                }
                FL_LPTIM32_Enable(LPTIM32);
            }
            break;
#endif

        default:
            break;
        }
}

static void hal_timer_stop(struct fm33_timer *timer)
{
    OS_ASSERT(timer != OS_NULL);

    switch(timer->info->type)
    {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
            FL_ATIM_Disable(ATIM);
            break;
#endif

#if defined(BSP_USING_BSTIM16)
        case TYPE_BSTIM16:
            FL_BSTIM16_Disable(BSTIM16);
            break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BSTIM32:
            FL_BSTIM32_Disable(BSTIM32);
            break;
#endif

#if defined(BSP_USING_GPTIM)
        case TYPE_GPTIM0:
        case TYPE_GPTIM1:
        case TYPE_GPTIM2:
            FL_GPTIM_Disable((GPTIM_Type *)(timer->info->inst));
            break;
#endif

#if defined(BSP_USING_LPTIM16)
        case TYPE_LPTIM16:
            FL_LPTIM16_Disable(LPTIM16);
            break;
#endif

#if defined(BSP_USING_LPTIM32)
        case TYPE_LPTIM32:
            FL_LPTIM32_Disable(LPTIM32);
            break;
#endif
        default:
            break;
    }
}

static os_uint64_t hal_timer_read(struct fm33_timer *timer)
{
    OS_ASSERT(timer != OS_NULL);

    os_uint64_t timer_counter;
    switch(timer->info->type)
    {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
            timer_counter = FL_ATIM_ReadCounter(ATIM);
            break;
#endif

#if defined(BSP_USING_BSTIM16)
        case TYPE_BSTIM16:
            timer_counter = FL_BSTIM16_ReadCounter(BSTIM16);
            break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BSTIM32:
            timer_counter = FL_BSTIM32_ReadCounter(BSTIM32);
            break;
#endif

#if defined(BSP_USING_GPTIM)
        case TYPE_GPTIM0:
        case TYPE_GPTIM1:
        case TYPE_GPTIM2:
            timer_counter = FL_GPTIM_ReadCounter((GPTIM_Type *)(timer->info->inst));
            break;
#endif

#if defined(BSP_USING_LPTIM16)
        case TYPE_LPTIM16:
            timer_counter = FL_LPTIM16_ReadCounter(LPTIM16);
            break;
#endif

#if defined(BSP_USING_LPTIM32)
        case TYPE_LPTIM32:
            timer_counter = FL_LPTIM32_ReadCounter(LPTIM32);
            break;
#endif
        default:
            break;
    }

    return timer_counter;
}

static os_uint64_t fm33_timer_read(void *clock)
{
    struct fm33_timer *timer;

    timer = (struct fm33_timer *)clock;

    return hal_timer_read(timer);
}

#ifdef OS_USING_CLOCKEVENT
static void fm33_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct fm33_timer *timer;

    timer = (struct fm33_timer *)ce;

    if (timer->info->type != TYPE_LPTIM32)
    {
        OS_ASSERT(prescaler != 0);
    }
    OS_ASSERT(count != 0);

    hal_timer_start(timer, prescaler, count, OS_TRUE);
}

static void fm33_timer_stop(os_clockevent_t *ce)
{
    struct fm33_timer *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct fm33_timer *)ce;

    hal_timer_stop(timer);
}

static const struct os_clockevent_ops fm33_tim_ops =
{
    .start = fm33_timer_start,
    .stop  = fm33_timer_stop,
    .read  = fm33_timer_read,
};
#endif

/**
 ***********************************************************************************************************************
 * @brief           fm33_tim_probe:probe timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int fm33_tim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct fm33_timer       *timer;
    struct fm33_timer_info  *info;

    timer = os_calloc(1, sizeof(struct fm33_timer));
    OS_ASSERT(timer);

    info = (struct fm33_timer_info *)dev->info;

    timer->info = info;
    timer->freq = fm33_timer_get_freq(timer);
    timer->mode = fm33_timer_mode_judge(timer);

#ifdef OS_USING_PWM
    if (timer->mode == TIMER_MODE_PWM)
    {
        fm33_pwm_register(dev->name,timer);
    }
#endif

#ifdef OS_USING_PULSE_ENCODER
    if (timer->mode == TIMER_MODE_PULSE_ENCODER)
    {
        fm33_pulse_encoder_register(dev->name,timer);
    }
#endif

    if (timer->mode == TIMER_MODE_TIM)
    {
#ifdef OS_USING_CLOCKSOURCE
        if (os_clocksource_best() == OS_NULL && fm33_timer_is_32b(timer))
        {
            hal_timer_start(timer, 1, 0xffffffffull, OS_FALSE);
            
            timer->clock.cs.rating  = 320;
            timer->clock.cs.freq    = timer->freq;
            timer->clock.cs.mask    = 0xffffffffull;
            timer->clock.cs.read    = fm33_timer_read;

            os_clocksource_register(dev->name, &timer->clock.cs);
        }
        else
#endif
        {
#ifdef OS_USING_CLOCKEVENT
            timer->clock.ce.rating  = fm33_timer_is_32b(timer) ? 320 : 160;
            timer->clock.ce.freq    = timer->freq;
            timer->clock.ce.mask    = 0xffffffffull;
            
            timer->clock.ce.prescaler_mask = 0xfffful;
            timer->clock.ce.prescaler_bits = 16;
            
            timer->clock.ce.count_mask = fm33_timer_is_32b(timer) ? 0xfffffffful : 0xfffful;
            timer->clock.ce.count_bits = fm33_timer_is_32b(timer) ? 32 : 16;

            timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

            timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
            
            timer->clock.ce.ops     = &fm33_tim_ops;

            /*lptim32 is used for lpmgr*/
            if(timer->info->type == TYPE_LPTIM32)
            {
                timer->freq /= 128;
                timer->clock.ce.freq   = timer->freq;
                timer->clock.ce.prescaler_mask = 0;
                timer->clock.ce.prescaler_bits = 0;
            }
            os_clockevent_register(dev->name, &timer->clock.ce);
#endif
        }
    }

    os_list_add(&fm33_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO fm33_tim_driver = {
    .name   = "TIMER_Type",
    .probe  = fm33_tim_probe,
};

OS_DRIVER_DEFINE(fm33_tim_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);


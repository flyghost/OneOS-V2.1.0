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
//#include <os_irq.h>
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
#define DRV_EXT_TAG "drv.hwtimer "
#include <drv_log.h>


#define TIMER_MODE_TIM              0x00
#define TIMER_MODE_PWM              0x01
#define TIMER_MODE_PULSE_ENCODER    0x02

static os_list_node_t fm33_timer_list = OS_LIST_INIT(fm33_timer_list);

os_bool_t fm33_timer_is_32b(TIM_HandleTypeDef *tim)
{
    if(tim->instance == (void *)BSTIM32)    /* LPTIM32 force to clockevent */
        return OS_TRUE;
    else if(tim->instance == (void *)ATIM || tim->instance == (void *)GPTIM0 || tim->instance == (void *)GPTIM1)
        return OS_FALSE;

    os_kprintf("invalid timer type.\r\n");
    return OS_FALSE;
}

static uint32_t fm33_timer_get_freq(TIM_HandleTypeDef *tim)
{
    os_uint32_t freq = 0;
    
    OS_ASSERT(tim != OS_NULL);
    
#if defined (BSP_USING_ATIM)
    if(tim->instance == ATIM)
    {
       os_uint32_t clk_div;

       clk_div = FL_ATIM_GetClockDivision(ATIM);
       freq    = FL_RCC_GetAPB2ClockFreq() >> (clk_div >> ATIM_CR1_CKD_Pos);
    }
#endif

#if defined (BSP_USING_BSTIM32)
    if(tim->instance == BSTIM32)
    {
       freq    = FL_RCC_GetAPB1ClockFreq();
    }
#endif


#if defined (BSP_USING_GPTIM0)
    if(tim->instance == GPTIM0)
    {
       freq    = FL_RCC_GetAPB1ClockFreq();
    }
#endif

#if defined (BSP_USING_GPTIM1)
    if(tim->instance == GPTIM1 )
    {
       freq    = FL_RCC_GetAPB1ClockFreq();
    }
#endif

#if defined (BSP_USING_LPTIM32)
    if(tim->instance == LPTIM32)
    {
       freq    = FL_RCC_GetAPB1ClockFreq();
    }
#endif
    return freq;
}

static os_uint8_t fm33_timer_mode_judge(TIM_HandleTypeDef *tim)
{
    return TIMER_MODE_TIM;
}

static void fm33_timer_update_callback(void *htim)
{
    struct fm33_timer *timer;

    os_list_for_each_entry(timer, &fm33_timer_list, struct fm33_timer, list)
    {
        if (timer->handle->instance == htim && timer->clock.ce.parent.type == OS_DEVICE_TYPE_CLOCKEVENT)
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
        fm33_timer_update_callback((void *)ATIM);
    }
}
#endif

#if defined (BSP_USING_BSTIM32)
void BSTIM_IRQHandler()
{
    if(FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        FL_BSTIM32_ClearFlag_Update(BSTIM32);
        fm33_timer_update_callback((void *)BSTIM32);
    }
}
#endif

#if defined (BSP_USING_GPTIM0)
void GPTIM0_IRQHandler()
{
    if(FL_GPTIM_IsEnabledIT_Update(GPTIM0) && FL_GPTIM_IsActiveFlag_Update(GPTIM0))
    {
        FL_GPTIM_ClearFlag_Update(GPTIM0);
        fm33_timer_update_callback((void *)GPTIM0);
    }
}
#endif

#if defined (BSP_USING_GPTIM1)
void GPTIM1_IRQHandler()
{
    if(FL_GPTIM_IsEnabledIT_Update(GPTIM1) && FL_GPTIM_IsActiveFlag_Update(GPTIM1))
    {
        FL_GPTIM_ClearFlag_Update(GPTIM1);
        fm33_timer_update_callback((void *)GPTIM1);
    }
}
#endif

#if defined (BSP_USING_LPTIM32)
void LPTIM_IRQHandler()
{
    if(FL_LPTIM32_IsEnabledIT_Update(LPTIM32) && FL_LPTIM32_IsActiveFlag_Update(LPTIM32))
    {
        FL_LPTIM32_ClearFlag_Update(LPTIM32);
        fm33_timer_update_callback((void *)LPTIM32);
    }
}
#endif

static void hal_timer_init(struct fm33_timer *timer)
{
    OS_ASSERT(timer != OS_NULL);
    
#if defined (BSP_USING_ATIM)
    if(timer->handle->instance == ATIM)
    {
       timer->type = TYPE_ATIM;
       return;
    }
#endif

#if defined (BSP_USING_BSTIM32)

    if(timer->handle->instance == BSTIM32)
    {
       timer->type = TYPE_BTIM;
       return;
    }
#endif

#if defined (BSP_USING_GPTIM0)
    if(timer->handle->instance == GPTIM0)
    {
       timer->type = TYPE_GPTIM0;
       return;
    }
#endif

#if defined (BSP_USING_GPTIM1)
    if(timer->handle->instance == GPTIM1 )
    {
       timer->type = TYPE_GPTIM1;
       return;
    }
#endif

#if defined (BSP_USING_LPTIM32)
    if(timer->handle->instance == LPTIM32)
    {
       timer->type = TYPE_LPTIM;
       return;
    }
#endif
    os_kprintf("invalid timer type.\r\n"); 
}

static void hal_timer_start(struct fm33_timer *timer, os_uint32_t prescaler, os_uint64_t count, os_bool_t irq_enable)
{    
    OS_ASSERT(timer != OS_NULL);
    OS_ASSERT(count != 0);

    switch(timer->type)
        {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
            {
                 FL_ATIM_InitTypeDef InitStruct;

                 InitStruct.clockSource          = FL_RCC_ATIM_CLK_SOURCE_APB2CLK;
                 InitStruct.prescaler            = prescaler - 1;
                 InitStruct.counterMode          = FL_ATIM_COUNTER_DIR_UP;
                 InitStruct.autoReload           = count;
                 InitStruct.autoReloadState      = FL_DISABLE;
                 InitStruct.clockDivision        = FL_ATIM_CLK_DIVISION_DIV1;
                 InitStruct.repetitionCounter    = 0;
                 
                 FL_ATIM_Init(ATIM, &InitStruct);
                 if(irq_enable)
                 {
                    FL_ATIM_EnableIT_Update(ATIM);
                    NVIC_EnableIRQ(ATIM_IRQn);
                 }
                 FL_ATIM_Enable(ATIM);
            }
            break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BTIM:
            {
                  FL_BSTIM32_InitTypeDef defaultInitStruct; 

                 defaultInitStruct.prescaler       = prescaler - 1;
                 defaultInitStruct.autoReload      = count;
                 defaultInitStruct.autoReloadState = FL_ENABLE;
                 defaultInitStruct.clockSource     = FL_RCC_BSTIM32_CLK_SOURCE_APB1CLK;

                 FL_BSTIM32_Init(BSTIM32, &defaultInitStruct );
                 if(irq_enable)
                 {
                     FL_BSTIM32_EnableIT_Update(BSTIM32 );
                     NVIC_EnableIRQ(BSTIM_IRQn);
                 }
                 FL_BSTIM32_Enable(BSTIM32);
            }
            break;
#endif

#if defined(BSP_USING_GPTIM0)
        case TYPE_GPTIM0:
            {
                FL_GPTIM_InitTypeDef    TimerBaseInitStruct;

                TimerBaseInitStruct.prescaler       = prescaler - 1;
                TimerBaseInitStruct.counterMode     = FL_GPTIM_COUNTER_DIR_UP;
                TimerBaseInitStruct.autoReload      = count;
                TimerBaseInitStruct.autoReloadState = FL_DISABLE;
                TimerBaseInitStruct.clockDivision   = FL_GPTIM_CLK_DIVISION_DIV1;

                FL_GPTIM_Init(GPTIM0, &TimerBaseInitStruct );
                if(irq_enable)
                {
                    FL_GPTIM_EnableIT_Update(GPTIM0 );
                    NVIC_EnableIRQ(GPTIM0_IRQn);
                }
                FL_GPTIM_Enable(GPTIM0);
            }
            break;
#endif

#if defined(BSP_USING_GPTIM1)
        case TYPE_GPTIM1:
            {
                FL_GPTIM_InitTypeDef    TimerBaseInitStruct;

                TimerBaseInitStruct.prescaler           = prescaler - 1;
                TimerBaseInitStruct.counterMode         = FL_GPTIM_COUNTER_DIR_UP;
                TimerBaseInitStruct.autoReload          = count;
                TimerBaseInitStruct.autoReloadState     = FL_DISABLE;
                TimerBaseInitStruct.clockDivision       = FL_GPTIM_CLK_DIVISION_DIV1;
                
                FL_GPTIM_Init(GPTIM1, &TimerBaseInitStruct);
                if(irq_enable)
                {
                    FL_GPTIM_EnableIT_Update(GPTIM1);
                    NVIC_EnableIRQ(GPTIM1_IRQn);
                }
                FL_GPTIM_Enable(GPTIM1);
            }
            break;
#endif

#if defined(BSP_USING_LPTIM32)
        case TYPE_LPTIM:
            {
                FL_LPTIM32_InitTypeDef    defaultInitStruct;
//                os_uint32_t               psc = 0;
//                prescaler = 128;
//                if(hal_lptim32_prescaler_check(prescaler, &psc) != OS_EOK)
//                {
//                    os_kprintf("lptimer start failed.\r\n");
//                    return;
//                }

                defaultInitStruct.clockSource           = FL_RCC_LPTIM32_CLK_SOURCE_APB1CLK;
                defaultInitStruct.prescalerClockSource  = FL_LPTIM32_CLK_SOURCE_INTERNAL;
                defaultInitStruct.prescaler             = FL_LPTIM32_PSC_DIV128;
                defaultInitStruct.autoReload            = count;
                defaultInitStruct.mode                  = FL_LPTIM32_OPERATION_MODE_NORMAL;
                defaultInitStruct.onePulseMode          = FL_LPTIM32_ONE_PULSE_MODE_CONTINUOUS;
                defaultInitStruct.countEdge             = FL_LPTIM32_ETR_COUNT_EDGE_RISING;
                defaultInitStruct.triggerEdge           = FL_LPTIM32_ETR_TRIGGER_EDGE_RISING;
                
                FL_LPTIM32_Init(LPTIM32,&defaultInitStruct );
                if(irq_enable)
                {
                    FL_LPTIM32_EnableIT_Update(LPTIM32 );
                    NVIC_EnableIRQ(LPTIM_IRQn);
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
    
    switch(timer->type)
            {
#if defined(BSP_USING_ATIM)
            case TYPE_ATIM:
                FL_ATIM_Disable(ATIM);
                break;
#endif

#if defined(BSP_USING_BSTIM32)
            case TYPE_BTIM:
                FL_BSTIM32_Disable(BSTIM32);
                break;
#endif

#if defined(BSP_USING_GPTIM0)
            case TYPE_GPTIM0:
                FL_GPTIM_Disable(GPTIM0);
                break;
#endif

#if defined(BSP_USING_GPTIM1)
            case TYPE_GPTIM1:
                FL_GPTIM_Disable(GPTIM1);
                break;
#endif

#if defined(BSP_USING_LPTIM32)
            case TYPE_LPTIM:
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
    switch(timer->type)
    {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
            timer_counter = FL_ATIM_ReadCounter(ATIM);
            break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BTIM:
            timer_counter = FL_BSTIM32_ReadCounter(BSTIM32);
            break;
#endif

#if defined(BSP_USING_GPTIM0)
        case TYPE_GPTIM0:
            timer_counter = FL_GPTIM_ReadCounter(GPTIM0);
            break;
#endif

#if defined(BSP_USING_GPTIM1)
        case TYPE_GPTIM1:
            timer_counter = FL_GPTIM_ReadCounter(GPTIM1);
            break;
#endif

#if defined(BSP_USING_LPTIM32)
        case TYPE_LPTIM:
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

    if (timer->type != TYPE_LPTIM)
    {
        OS_ASSERT(prescaler != 0);
    }
    OS_ASSERT(count != 0);

    timer = (struct fm33_timer *)ce;

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
    struct fm33_timer *timer;
    TIM_HandleTypeDef  *tim;

    timer = os_calloc(1, sizeof(struct fm33_timer));
    OS_ASSERT(timer);

    tim = (TIM_HandleTypeDef *)dev->info;

    timer->handle = tim;

    hal_timer_init(timer);

    timer->freq = fm33_timer_get_freq(timer->handle);

    timer->mode = fm33_timer_mode_judge(timer->handle);

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
        if (os_clocksource_best() == OS_NULL && fm33_timer_is_32b(timer->handle))
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
            timer->clock.ce.rating  = fm33_timer_is_32b(timer->handle) ? 320 : 160;
            timer->clock.ce.freq    = timer->freq;
            timer->clock.ce.mask    = 0xffffffffull;
            
            timer->clock.ce.prescaler_mask = 0xfffful;
            timer->clock.ce.prescaler_bits = 16;
            
            timer->clock.ce.count_mask = fm33_timer_is_32b(timer->handle) ? 0xfffffffful : 0xfffful;
            timer->clock.ce.count_bits = fm33_timer_is_32b(timer->handle) ? 32 : 16;

            timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

            timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
            
            timer->clock.ce.ops     = &fm33_tim_ops;

            if(timer->type == TYPE_LPTIM)
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
    .name   = "TIM_HandleTypeDef",
    .probe  = fm33_tim_probe,
};

OS_DRIVER_DEFINE(fm33_tim_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);

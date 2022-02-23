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
 * @brief       This file implements PWM driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_hwtimer.h>
#include <string.h>
#include <os_memory.h>
#include "drv_pwm.h"


#define DBG_TAG    "drv.pwm"
#include <dlog.h>

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{

}
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{

}

static os_err_t get_pwm_channel(os_uint16_t channel, os_uint32_t *stm32_channel)
{
    switch(channel)
    {
#ifdef TIM_CHANNEL_1
    case 1:
       *stm32_channel = TIM_CHANNEL_1;
        break;
#endif
#ifdef TIM_CHANNEL_2
    case 2:
       *stm32_channel =  TIM_CHANNEL_2;
        break;
#endif
#ifdef TIM_CHANNEL_3
    case 3:
       *stm32_channel =  TIM_CHANNEL_3;
        break;
#endif
#ifdef TIM_CHANNEL_4
    case 4:
       *stm32_channel =  TIM_CHANNEL_4;
        break;
#endif
#ifdef TIM_CHANNEL_5
    case 5:
       *stm32_channel =  TIM_CHANNEL_5;
        break;
#endif
#ifdef TIM_CHANNEL_6
    case 6:
       *stm32_channel =  TIM_CHANNEL_6;
        break;
#endif
#ifdef TIM_CHANNEL_ALL
    case 0xFFFF:
       *stm32_channel =  TIM_CHANNEL_ALL;
        break;
#endif
    default:
        return OS_ENOSYS;
    }
    return OS_EOK;
}

static os_err_t stm32_pwm_enabled(struct os_pwm_device *dev, os_uint32_t channel, os_bool_t enable)
{
    os_uint32_t st_timer_channel = 0;
    
    struct stm32_pwm *st_pwm;

    st_pwm = os_container_of(dev, struct stm32_pwm, pwm);

    if (get_pwm_channel(channel, &st_timer_channel) != OS_EOK)
    {
        LOG_E(DBG_TAG,"pwm channel %d is illegal!\r\n", channel);
        return OS_ENOSYS;
    }
    
    if (!enable)
    {
        HAL_TIM_PWM_Stop(st_pwm->tim->handle, st_timer_channel);
    }
    else
    {
        HAL_TIM_PWM_Start(st_pwm->tim->handle, st_timer_channel);
    }

    return OS_EOK;
}

static os_err_t stm32_pwm_set_period(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t nsec)
{
    os_uint64_t tim_tick = 0;
    os_uint64_t prescaler = 0;
    os_uint64_t Period = 0;
    
    struct stm32_pwm *st_pwm;

    st_pwm = os_container_of(dev, struct stm32_pwm, pwm);
    
    tim_tick = (os_uint64_t)nsec * st_pwm->tim_mult >> st_pwm->tim_shift;
    prescaler = tim_tick / st_pwm->pwm.max_value;
    Period = tim_tick / (prescaler + 1) - 1;
    
    if ((st_pwm->tim->handle->Init.Prescaler != prescaler) || (st_pwm->tim->handle->Init.Period != Period))
    {
        st_pwm->tim->handle->Init.Prescaler = prescaler;
        st_pwm->tim->handle->Init.Period = Period;
        
        st_pwm->freq = st_pwm->tim->freq / (st_pwm->tim->handle->Init.Prescaler + 1);
        
        HAL_TIM_Base_Stop(st_pwm->tim->handle);
        __HAL_TIM_SET_PRESCALER(st_pwm->tim->handle, st_pwm->tim->handle->Init.Prescaler);
        __HAL_TIM_SET_AUTORELOAD(st_pwm->tim->handle, st_pwm->tim->handle->Init.Period);
    }

    return OS_EOK;
}

static os_err_t stm32_pwm_set_pulse(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t buffer)
{
    os_uint32_t pwm_pulse = 0;
    os_uint32_t st_timer_channel = 0;

    struct stm32_pwm *st_pwm;

    st_pwm = os_container_of(dev, struct stm32_pwm, pwm);

    if (get_pwm_channel(channel, &st_timer_channel) != OS_EOK)
    {
        LOG_E(DBG_TAG,"pwm channel %d is illegal!\r\n", channel);
        return OS_ENOSYS;
    }
    
    if (buffer > dev->period)
    {
        LOG_E(DBG_TAG,"pwm pulse value over range!\r\n");
        return OS_ERROR;
    }
    else
    {
        pwm_pulse = ((os_uint64_t)buffer * st_pwm->tim_mult >> st_pwm->tim_shift) / (st_pwm->tim->handle->Init.Prescaler + 1);
    }

    
    if (pwm_pulse > dev->max_value)
    {
        LOG_E(DBG_TAG,"pwm pulse value over range!\r\n");
        return OS_EFULL;
    }
    
    __HAL_TIM_SET_COMPARE(st_pwm->tim->handle, st_timer_channel, pwm_pulse);
    __HAL_TIM_SET_COUNTER(st_pwm->tim->handle, 0);

    HAL_TIM_GenerateEvent(st_pwm->tim->handle, TIM_EVENTSOURCE_UPDATE);

    return OS_EOK;
}

static const struct os_pwm_ops stm32_pwm_ops =
{
    .enabled = stm32_pwm_enabled,
    .set_period = stm32_pwm_set_period,
    .set_pulse = stm32_pwm_set_pulse,
    .control  = OS_NULL,
};

os_err_t stm32_pwm_register(const char *name, struct stm32_timer *tim)
{
    char *pwm_name = os_calloc(1, sizeof(name) + 14);

    strcpy(pwm_name, "pwm_");
    
    pwm_name = strcat(pwm_name,name);

    struct stm32_pwm *st_pwm = os_calloc(1, sizeof(struct stm32_pwm));
    
    OS_ASSERT(st_pwm);
    
    st_pwm->tim = tim;
    
    st_pwm->freq = st_pwm->tim->freq / (st_pwm->tim->handle->Init.Prescaler + 1);
    
    if (stm32_timer_is_32b(st_pwm->tim->handle->Instance))
    {
        st_pwm->pwm.max_value = 0xFFFFFFFF;
    }
    else
    {
        st_pwm->pwm.max_value = 0xFFFF;
    }
    
    calc_mult_shift(&st_pwm->tim_mult, &st_pwm->tim_shift, NSEC_PER_SEC, st_pwm->tim->freq, st_pwm->pwm.max_value / st_pwm->tim->freq);
   
    st_pwm->pwm.ops = &stm32_pwm_ops;
   
    os_device_pwm_register(&st_pwm->pwm,
                            pwm_name);
    return OS_EOK;
}


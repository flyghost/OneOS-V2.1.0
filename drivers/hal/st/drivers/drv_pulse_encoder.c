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
 * @file        drv_pulse_encoder.c
 *
 * @brief       This file implements pulse_encoder driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <string.h>
#include "drv_hwtimer.h"
#include "drv_pulse_encoder.h"
#include <os_memory.h>


#define DBG_TAG        "drv.pulse_encoder"
#include <drv_log.h>

void pulse_encoder_update_isr(struct stm32_timer *timer)
{
    struct stm32_pulse_encoder *st_pulse_encoder;

    st_pulse_encoder = (struct stm32_pulse_encoder *)timer->user_data;

    if (__HAL_TIM_IS_TIM_COUNTING_DOWN(timer->handle))
    {
        st_pulse_encoder->pulse_encoder.over_under_flowcount--;
    }
    else
    {
        st_pulse_encoder->pulse_encoder.over_under_flowcount++;
    }
}

static os_err_t stm32_pulse_encoder_enabled(struct os_pulse_encoder_device *dev, os_bool_t enable)
{
    struct stm32_pulse_encoder *st_pulse_encoder;

    st_pulse_encoder = os_container_of(dev, struct stm32_pulse_encoder, pulse_encoder);
    
    if (!enable)
    {
        HAL_TIM_Encoder_Stop(st_pulse_encoder->tim->handle, TIM_CHANNEL_ALL);
        __HAL_TIM_DISABLE_IT(st_pulse_encoder->tim->handle, TIM_IT_UPDATE);    
    }
    else
    {
        HAL_TIM_Encoder_Start(st_pulse_encoder->tim->handle, TIM_CHANNEL_ALL);
        __HAL_TIM_CLEAR_FLAG(st_pulse_encoder->tim->handle, TIM_FLAG_UPDATE);
        __HAL_TIM_ENABLE_IT(st_pulse_encoder->tim->handle, TIM_IT_UPDATE);
    }

    return OS_EOK;
}

static os_err_t stm32_pulse_encoder_set_period(struct os_pulse_encoder_device *dev, os_uint32_t period)
{
    struct stm32_pulse_encoder *st_pulse_encoder;

    st_pulse_encoder = os_container_of(dev, struct stm32_pulse_encoder, pulse_encoder);

    if (period <= st_pulse_encoder->pulse_encoder.max_period)
    {
        __HAL_TIM_SET_AUTORELOAD(st_pulse_encoder->tim->handle, period);
        st_pulse_encoder->pulse_encoder.period = period + 1;
    }
    else
    {
        LOG_E(DBG_TAG,"pulse_encoder period over range!\r\n");
        return OS_EFULL;
    }
    
    return OS_EOK;
}


static os_err_t stm32_pulse_encoder_get_count(struct os_pulse_encoder_device *dev, os_int16_t *buffer)
{
    struct stm32_pulse_encoder *st_pulse_encoder;
    
    st_pulse_encoder = os_container_of(dev, struct stm32_pulse_encoder, pulse_encoder);

   if (st_pulse_encoder->pulse_encoder.over_under_flowcount < 0)
   {
       
       *buffer = -(st_pulse_encoder->pulse_encoder.period - __HAL_TIM_GET_COUNTER(st_pulse_encoder->tim->handle));
   }
   else
   {
       *buffer = (__HAL_TIM_GET_COUNTER(st_pulse_encoder->tim->handle));
   }

    return OS_EOK;
}

static os_err_t stm32_pulse_encoder_clear_count(struct os_pulse_encoder_device *dev)
{
    struct stm32_pulse_encoder *st_pulse_encoder;

    st_pulse_encoder = os_container_of(dev, struct stm32_pulse_encoder, pulse_encoder);

    __HAL_TIM_SET_COUNTER(st_pulse_encoder->tim->handle, 0);

    st_pulse_encoder->pulse_encoder.over_under_flowcount = 0;
    
    return OS_EOK;
}

static os_err_t stm32_pulse_encoder_control(struct os_pulse_encoder_device *dev, os_uint32_t cmd, void *args)
{
    switch (cmd)
    {
    case PULSE_ENCODER_CMD_CLEAR_COUNT:
        return stm32_pulse_encoder_clear_count(dev);
    case PULSE_ENCODER_CMD_SET_PERIOD:
        return stm32_pulse_encoder_set_period(dev, *(os_uint32_t *)args);
    default:
        return OS_ENOSYS;
    }
}

static const struct os_pulse_encoder_ops stm32_pulse_encoder_ops =
{
    .enabled = stm32_pulse_encoder_enabled,
    .get_count = stm32_pulse_encoder_get_count,
    .control  = stm32_pulse_encoder_control,
};

os_err_t stm32_pulse_encoder_register(const char *name, struct stm32_timer *timer)
{
    char *pulse_encoder_name = os_calloc(1, sizeof(name) + 10);

    strcpy(pulse_encoder_name, "encoder_");
    
    pulse_encoder_name = strcat(pulse_encoder_name,name);

    struct stm32_pulse_encoder *st_pulse_encoder = os_calloc(1, sizeof(struct stm32_pulse_encoder));
    
    OS_ASSERT(st_pulse_encoder);
    
    st_pulse_encoder->tim = timer;

    if (stm32_timer_is_32b(st_pulse_encoder->tim->handle->Instance))
    {
        st_pulse_encoder->pulse_encoder.max_period = 0xFFFFFFFF;
    }
    else
    {
        st_pulse_encoder->pulse_encoder.max_period = 0xFFFF;
    }

    st_pulse_encoder->pulse_encoder.ops = &stm32_pulse_encoder_ops;

    st_pulse_encoder->pulse_encoder.period = st_pulse_encoder->tim->handle->Init.Period + 1;

    st_pulse_encoder->tim->user_data = st_pulse_encoder;
    
    os_device_pulse_encoder_register(&st_pulse_encoder->pulse_encoder,
                                pulse_encoder_name);
    return OS_EOK;
}


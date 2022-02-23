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
 * @file        drv_adc.c
 *
 * @brief       This file implements adc driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_stddef.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <string.h>
#include <drv_cfg.h>

#include <dlog.h>
#define DBG_TAG "drv.adc"

struct stm32_adc
{
    struct os_adc_device adc;
    ADC_HandleTypeDef *hadc;
    os_uint32_t timeout;
    os_uint8_t status;
};

os_err_t stm32_adc_get_channel(os_uint32_t channel, os_uint32_t *adc_channel)
{
    switch(channel)
    {
#ifdef ADC_CHANNEL_0
    case 0:
        *adc_channel = ADC_CHANNEL_0;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_1
    case 1:
        *adc_channel = ADC_CHANNEL_1;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_2
    case 2:
        *adc_channel = ADC_CHANNEL_2;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_3
    case 3:
        *adc_channel = ADC_CHANNEL_3;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_4
    case 4:
        *adc_channel = ADC_CHANNEL_4;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_5
    case 5:
        *adc_channel = ADC_CHANNEL_5;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_6
    case 6:
        *adc_channel = ADC_CHANNEL_6;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_7
    case 7:
        *adc_channel = ADC_CHANNEL_7;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_8
    case 8:
        *adc_channel = ADC_CHANNEL_8;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_9
    case 9:
        *adc_channel = ADC_CHANNEL_9;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_10
    case 10:
        *adc_channel = ADC_CHANNEL_10;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_11
    case 11:
        *adc_channel = ADC_CHANNEL_11;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_12
    case 12:
        *adc_channel = ADC_CHANNEL_12;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_13
    case 13:
        *adc_channel = ADC_CHANNEL_13;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_14
    case 14:
        *adc_channel = ADC_CHANNEL_14;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_15
    case 15:
        *adc_channel = ADC_CHANNEL_15;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_16
    case 16:
        *adc_channel = ADC_CHANNEL_16;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_17
    case 17:
        *adc_channel = ADC_CHANNEL_17;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_18
    case 18:
        *adc_channel = ADC_CHANNEL_18;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_19
    case 19:
        *adc_channel = ADC_CHANNEL_19;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_20
    case 20:
        *adc_channel = ADC_CHANNEL_20;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_21
    case 21:
        *adc_channel = ADC_CHANNEL_21;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_22
    case 22:
        *adc_channel = ADC_CHANNEL_22;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_23
    case 23:
        *adc_channel = ADC_CHANNEL_23;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_24
    case 24:
        *adc_channel = ADC_CHANNEL_24;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_25
    case 25:
        *adc_channel = ADC_CHANNEL_25;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_26
    case 26:
        *adc_channel = ADC_CHANNEL_26;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_27
    case 27:
        *adc_channel = ADC_CHANNEL_27;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_28
    case 28:
        *adc_channel = ADC_CHANNEL_28;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_29
    case 29:
        *adc_channel = ADC_CHANNEL_29;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_30
    case 30:
        *adc_channel = ADC_CHANNEL_30;
        return OS_EOK;
#endif
#ifdef ADC_CHANNEL_31
    case 31:
        *adc_channel = ADC_CHANNEL_31;
        return OS_EOK;
#endif
    default:
    return OS_ERROR;
    }
}

os_err_t stm32_adc_get_channel_config(struct stm32_adc *adc, os_uint32_t channel, ADC_ChannelConfTypeDef *sConfig)
{
    LOG_D(DBG_TAG, "This function use to get adc channel param! Here just give a demo about adc1.");
    LOG_D(DBG_TAG, "More param should config when use!");
#if 0
#ifdef ADC1
    if (adc->hadc == (ADC_HandleTypeDef *)ADC1)
    {
        switch(channel)
        {
        case ADC_CHANNEL_0:
            adc->timeout = 500;
            sConfig->Channel = channel;
            sConfig->Rank = ADC_REGULAR_RANK_1;
            sConfig->SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
        break;
        default:
            adc->timeout = 500;
            sConfig->Channel = channel;
            sConfig->Rank = ADC_REGULAR_RANK_1;
            sConfig->SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
        break;
        }
        return OS_EOK;
    }
#endif
#endif
    LOG_D(DBG_TAG, "Use default param!");
    adc->timeout = 500;
    sConfig->Channel = channel;
#if defined(ADC_REGULAR_RANK_1)
    sConfig->Rank = ADC_REGULAR_RANK_1;
#else
    sConfig->Rank = 1;
#endif
#if defined(ADC_SAMPLETIME_47CYCLES_5)
    sConfig->SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
#elif defined(ADC_SAMPLETIME_48CYCLES)
    sConfig->SamplingTime = ADC_SAMPLETIME_48CYCLES;
#elif defined(ADC_SAMPLETIME_55CYCLES_5)
    sConfig->SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
#elif defined(ADC_SAMPLETIME_56CYCLES)
    sConfig->SamplingTime = ADC_SAMPLETIME_56CYCLES;
#elif defined(ADC_SAMPLETIME_61CYCLES_5)
    sConfig->SamplingTime = ADC_SAMPLETIME_61CYCLES_5;
#elif defined(ADC_SAMPLETIME_64CYCLES_5)
    sConfig->SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
#elif defined(ADC_SAMPLETIME_79CYCLES_5)
    sConfig->SamplingTime = ADC_SAMPLETIME_79CYCLES_5;
#else
    LOG_E(DBG_TAG, "adc SamplingTime cannot find suitable param!please check!");
#endif

#if defined(ADC_SINGLE_ENDED) && defined(ADC_OFFSET_NONE)
    sConfig->SingleDiff = ADC_SINGLE_ENDED;
    sConfig->OffsetNumber = ADC_OFFSET_NONE;
    sConfig->Offset = 0;
#endif

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           stm32_adc_poll_convert_then_read: start adc convert in poll
 *
 * @details         channel and order config in stm32cubeMX,"channell" is mapping of rank configed in cube,
 *
 * @attention       Attention_description_Optional
 *
 ***********************************************************************************************************************
 */
static os_err_t stm32_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{  
    os_uint32_t adc_channel = 0;
    ADC_ChannelConfTypeDef sConfig = {0};
    
    struct stm32_adc *dev_adc;
    
    OS_ASSERT(dev != OS_NULL);
    dev_adc = os_container_of(dev, struct stm32_adc, adc);

    LOG_D(DBG_TAG,"adc not support channel detection! user should makesure channel is right!");
    
    if (stm32_adc_get_channel(channel, &adc_channel) == OS_ERROR)
    {
        LOG_E(DBG_TAG,"adc channel not support!");
        return OS_ERROR;
    }

    if (dev_adc->status != OS_ADC_ENABLE)
    {
        LOG_E(DBG_TAG,"adc disabled! please enable adc first!");
        return OS_ERROR;
    }

    stm32_adc_get_channel_config(dev_adc, adc_channel, &sConfig);

    if (HAL_ADC_ConfigChannel(dev_adc->hadc, &sConfig) != HAL_OK)
    {
        LOG_E(DBG_TAG,"adc channel config failed!!");
        return OS_ERROR;
    }
  
    HAL_ADC_Start(dev_adc->hadc);
    if (HAL_ADC_PollForConversion(dev_adc->hadc, dev_adc->timeout) != HAL_OK)
    {
        LOG_E(DBG_TAG,"adc conversion failed! check adc hardware config and enable!");
        return OS_ERROR;
    }
    *buff = (os_int32_t)((os_uint64_t)HAL_ADC_GetValue(dev_adc->hadc) * dev->mult >> dev->shift);
    
    HAL_ADC_Stop(dev_adc->hadc);

    return OS_EOK;
}

static os_err_t stm32_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    struct stm32_adc *dev_adc;

    dev_adc = os_container_of(dev, struct stm32_adc, adc);
    
    if (!enable)
    {
        if (HAL_ADC_DeInit(dev_adc->hadc) != HAL_OK)
        {
            return OS_ERROR;
        }
        dev_adc->status = OS_ADC_DISABLE;
    }
    else
    {
        if (HAL_ADC_Init(dev_adc->hadc) != HAL_OK)
        {
            return OS_ERROR;
        }
        dev_adc->status = OS_ADC_ENABLE;
    }
    
    return OS_EOK;
}

static os_err_t stm32_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static os_err_t stm32_adc_probe_check(struct os_adc_device *adc, ADC_HandleTypeDef *hadc)
{
    os_uint32_t value = 0;

#ifdef ADC_DATAALIGN_RIGHT
    if (hadc->Init.DataAlign != ADC_DATAALIGN_RIGHT)
    {
        LOG_E(DBG_TAG,"adc dataalign must be right-align!");
        return OS_ERROR;
    }
#endif
    
#ifdef ADC_RESOLUTION_12B    
    switch(hadc->Init.Resolution)
    {
#ifdef ADC_RESOLUTION_10B
    case ADC_RESOLUTION_10B:
        value = (1UL << 10) - 1;
        break;
#endif
#ifdef ADC_RESOLUTION_12B
    case ADC_RESOLUTION_12B:
        value = (1UL << 12) - 1;
        break;
#endif
#ifdef ADC_RESOLUTION_14B
    case ADC_RESOLUTION_14B:
        value = (1UL << 14) - 1;
        break;
#endif
#ifdef ADC_RESOLUTION_16B
    case ADC_RESOLUTION_16B:
        value = (1UL << 16) - 1;
        break;
#endif
    default:
        LOG_E(DBG_TAG,"adc resolution just support 10/12/14/16bits!");
        return OS_ERROR;
    }
#else
    value = (1UL << 12) - 1;
#endif

    if (adc->max_value == 0)
    {
        adc->max_value = value;
    }
    else
    {
        if (adc->max_value != value)
        {
            LOG_E(DBG_TAG,"all adc must use same resolution in 10/12/14/16bits!");
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static const struct os_adc_ops stm32_adc_ops = {
    .adc_enabled            = stm32_adc_enabled,
    .adc_control            = stm32_adc_control,
    .adc_read               = stm32_adc_read,
};

static int stm32_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;

    struct stm32_adc *st_adc = OS_NULL;
    st_adc = os_calloc(1, sizeof(struct stm32_adc)); 
    OS_ASSERT(st_adc);
    
    st_adc->hadc = (ADC_HandleTypeDef *)dev->info;
    st_adc->status = OS_ADC_DISABLE;
    if (HAL_ADC_DeInit(st_adc->hadc) != HAL_OK)
    {
        os_free(st_adc);
        return OS_ERROR;
    }

    struct os_adc_device *dev_adc = &st_adc->adc;
    dev_adc->ops    = &stm32_adc_ops;
    dev_adc->max_value = 0;
    dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;

    result = stm32_adc_probe_check(&st_adc->adc, st_adc->hadc);
    if (result != OS_EOK)
    {
        os_free(st_adc);
        LOG_E(DBG_TAG,"adc config check has errors! not register!");
        return OS_ERROR;
    }
    result = os_hw_adc_register(dev_adc, dev->name, NULL);
    if (result != OS_EOK)
    {
        os_free(st_adc);
        LOG_E(DBG_TAG,"%s register fialed!", dev->name);
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO stm32_adc_driver = {
    .name   = "ADC_HandleTypeDef",
    .probe  = stm32_adc_probe,
};

OS_DRIVER_DEFINE(stm32_adc_driver, DEVICE,OS_INIT_SUBLEVEL_HIGH);

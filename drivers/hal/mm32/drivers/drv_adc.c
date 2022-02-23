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
 * @brief       This file implements adc driver for mm32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mm32_hal.h"

#include <board.h>
#include <os_stddef.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <string.h>
#include <drv_cfg.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include <drv_log.h>

#include "drv_adc.h"

struct mm32_adc_device
{
    struct os_adc_device    adc;
    struct mm32_adc_info   *adc_info;
    os_uint32_t             timeout;
    os_uint8_t              status;
};

static void mm32_adc_init(struct mm32_adc_info *adc_info)
{
    os_uint32_t i = 0;
    GPIO_InitTypeDef GPIO_InitStruct;
    
    adc_info->adc_rcc_clkcmd(adc_info->adc_rcc_clk, ENABLE);
    ADC_Init(adc_info->hadc, &adc_info->init_struct);
    
    for (i = 0;i < adc_info->pin_num;i++)
    {
        adc_info->gpio_rcc_clkcmd(adc_info->pin[i].gpio_clk, ENABLE);
        GPIO_InitStruct.GPIO_Pin    = adc_info->pin[i].gpio;
        GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_50MHz;                           //Output speed
        GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AIN;                               //GPIO mode
        GPIO_Init(adc_info->pin[i].port, &GPIO_InitStruct);
    }
}

static os_err_t mm32_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{
    struct mm32_adc_device *dev_adc;
    
    OS_ASSERT(dev != OS_NULL);
    dev_adc = os_container_of(dev, struct mm32_adc_device, adc);

    if (dev_adc->status != OS_ADC_ENABLE)
    {
        LOG_E(DRV_EXT_TAG,"adc disabled! please enable adc first!");
        return OS_ERROR;
    }

    if (channel >= dev_adc->adc_info->pin_num)
    {
        LOG_E(DRV_EXT_TAG,"adc channel %d not support!", channel);
        return OS_ERROR;
    }
    
    dev_adc->adc_info->hadc->ADCHS &= (1 << channel);
    
    ADC_RegularChannelConfig(dev_adc->adc_info->hadc, dev_adc->adc_info->pin[channel].channel, 0, dev_adc->adc_info->pin[channel].sample_time);
    ADC_Cmd(dev_adc->adc_info->hadc, ENABLE);
  
    ADC_SoftwareStartConvCmd(dev_adc->adc_info->hadc, ENABLE);                                     //Software start conversion
    while(ADC_GetFlagStatus(dev_adc->adc_info->hadc, ADC_IT_EOC) == 0);
    ADC_ClearFlag(dev_adc->adc_info->hadc, ADC_IT_EOC);
    *buff = (os_int32_t)((os_uint64_t)ADC_GetConversionValue(dev_adc->adc_info->hadc) * dev->mult >> dev->shift);
    ADC_Cmd(dev_adc->adc_info->hadc, DISABLE);
    return OS_EOK;
}

static os_err_t mm32_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    struct mm32_adc_device *dev_adc;

    dev_adc = os_container_of(dev, struct mm32_adc_device, adc);
    
    if (!enable)
        dev_adc->status = OS_ADC_DISABLE;
    else
        dev_adc->status = OS_ADC_ENABLE;
    
    return OS_EOK;
}

static os_err_t mm32_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static const struct os_adc_ops mm32_adc_ops = {
    .adc_enabled            = mm32_adc_enabled,
    .adc_control            = mm32_adc_control,
    .adc_read               = mm32_adc_read,
};

static os_err_t mm32_adc_probe_check(struct mm32_adc_device *mm32_adc)
{
    struct os_adc_device *dev_adc = &mm32_adc->adc;
    dev_adc->ops    = &mm32_adc_ops;
    dev_adc->max_value = 0;
    dev_adc->ref_low   = mm32_adc->adc_info->ref_low;
    dev_adc->ref_hight = mm32_adc->adc_info->ref_high;
    
    switch(mm32_adc->adc_info->init_struct.ADC_Resolution)
    {
    case ADC_Resolution_12b:
        dev_adc->max_value = (1UL << 12) - 1;
    break;
    case ADC_Resolution_11b:
        dev_adc->max_value = (1UL << 11) - 1;
    break;
    case ADC_Resolution_10b:
        dev_adc->max_value = (1UL << 10) - 1;
    break;
    case ADC_Resolution_9b:
        dev_adc->max_value = (1UL << 9) - 1;
    break;
    case ADC_Resolution_8b:
        dev_adc->max_value = (1UL << 8) - 1;
    break;
    default:
    break;
    }
    if (dev_adc->max_value == 0)
    {
        LOG_E(DRV_EXT_TAG, "ADC_Resolution not support!");
        return OS_ERROR;
    }
    return OS_EOK;
}

static int mm32_adc_device_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;

    struct mm32_adc_device *mm32_adc = OS_NULL;
    mm32_adc = os_calloc(1, sizeof(struct mm32_adc_device)); 
    OS_ASSERT(mm32_adc);
    
    mm32_adc->adc_info = (struct mm32_adc_info *)dev->info;
    mm32_adc->status = OS_ADC_DISABLE;
    
    mm32_adc_init(mm32_adc->adc_info);
    
    mm32_adc_probe_check(mm32_adc);
    
    result = os_hw_adc_register(&mm32_adc->adc, dev->name, NULL);
    if (result != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG,"%s register fialed!\r\n", dev->name);
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO mm32_adc_device_driver = {
    .name   = "ADC_TypeDef",
    .probe  = mm32_adc_device_probe,
};

OS_DRIVER_DEFINE(mm32_adc_device_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);

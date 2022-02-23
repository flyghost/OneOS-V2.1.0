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
 * @brief       This file implements adc driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_task.h>

#ifdef OS_USING_ADC

#include <os_memory.h>
#include "peripherals.h"
#include <drv_log.h>
#include "drv_adc.h"
#include "fsl_adc.h"
#include <os_device.h>
#include "MIMXRT1052.h"
#include "bus.h"

#if !defined(ADC1_PERIPHERAL) && !defined(ADC2_PERIPHERAL)
#error "Please define at least one BSP_USING_ADCx"
#endif

#if (defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
#error "Please don't define 'FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL'!"
#endif

volatile bool g_AdcConversionDoneFlag;

void ADC_ETC_ADC_ETC_0_IRQHANDLER(void)
{
    ADC_ETC_ClearInterruptStatusFlags(ADC_ETC_PERIPHERAL, kADC_ETC_Trg0TriggerSource, kADC_ETC_Done0StatusFlagMask);
    g_AdcConversionDoneFlag = true;

    return;
}

void ADC_ETC_ADC_ETC_1_IRQHANDLER(void)
{
    ADC_ETC_ClearInterruptStatusFlags(ADC_ETC_PERIPHERAL, kADC_ETC_Trg1TriggerSource, kADC_ETC_Done1StatusFlagMask);
    g_AdcConversionDoneFlag = true;

    return;
}

void ADC_ETC_ADC_ETC_ERROR_IRQHANDLER(void)
{
    os_kprintf("ADC_ETC_ADC_ETC_ERROR_IRQHANDLER\n");

    return;
}


static os_err_t imxrt_adc_enabled(struct os_adc_device *device, os_bool_t enabled)
{
    os_err_t result = OS_EOK;

    if (enabled == OS_TRUE)
    {
        EnableIRQ(ADC_ETC_ADC_ETC_0_IRQN);
    }
    else if (enabled == OS_FALSE)
    {
        DisableIRQ(ADC_ETC_ADC_ETC_0_IRQN);
    }
    else
    {
        result = OS_ENOSYS;
    }
    
    return result;
}

static os_err_t imxrt_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    os_err_t result = OS_EOK;
    
    if (cmd == OS_ADC_CMD_ENABLE)
    {
        imxrt_adc_enabled(dev, OS_TRUE);
    }
    else if (cmd == OS_ADC_CMD_DISABLE)
    {
        imxrt_adc_enabled(dev, OS_FALSE);
    }
    else
    {
        os_kprintf("adc set able fail.\n");
        result = OS_ERROR;
    }

    return result;
}

os_uint32_t imxrt_get_channel_group_by_adc_channel(os_uint32_t channel)
{
    os_uint32_t triggerGroup = ADC_ETC_TC_0; // TODO: 03903918
    os_uint32_t channelNum = 0;
    os_uint32_t channelIdx;
    os_uint32_t i;

    // trigger group 0
    channelNum = ADC_ETC_trigger_config[triggerGroup].triggerChainLength + 1;
    for (channelIdx = 0; channelIdx < channelNum; channelIdx++)
    {
        if (ADC_ETC_TC_0_chain_config[channelIdx].ADCChannelSelect == channel)
        {
            for (i = 0; i < ADC_HC_COUNT; i++)
            {
                if ((ADC_ETC_TC_0_chain_config[channelIdx].ADCHCRegisterSelect >> i) & 0x1)
                {
                    return i;
                }
            }
        }
    }
    
    os_kprintf("find channel group fail\n");

    return 0;
}


static os_err_t imxrt_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{
    adc_channel_config_t adc_channel;
    ADC_Type *base;
    imxrt_adc *imxrtAdc;    
    os_uint32_t channelGroup;
    os_uint32_t triggerGroup = ADC_ETC_TC_0; // TODO: 03903918
    
    imxrtAdc = os_container_of(dev, imxrt_adc, adc);
    base = imxrtAdc->info->adc_base;
    
    ADC_EnableHardwareTrigger(base, true);

    // specify the adc channel & channel group.
    adc_channel.channelNumber = channel;
    adc_channel.enableInterruptOnConversionCompleted = false;
    channelGroup = imxrt_get_channel_group_by_adc_channel(channel);
    ADC_SetChannelConfig(base, channelGroup, &adc_channel);

    // adc convert start
    g_AdcConversionDoneFlag = false;
    ADC_ETC_DoSoftwareTrigger(ADC_ETC_PERIPHERAL, triggerGroup);
    while (g_AdcConversionDoneFlag == false);

    *buff = ADC_ETC_GetADCConversionValue(ADC_ETC_PERIPHERAL, triggerGroup, channelGroup);
    __DSB();

    return OS_EOK;
}

static struct os_adc_ops imxrt_adc_ops =
{
    .adc_enabled = imxrt_adc_enabled,
    .adc_control = imxrt_adc_control,
    .adc_read = imxrt_adc_read
};

static int imxrt_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    int result = OS_EOK;    
    ADC_Type *adc_base;
    imxrt_adc *imxrtDac = NULL;
    struct os_adc_device *adcDevice = NULL;

    imxrtDac = (imxrt_adc *)os_calloc(1, sizeof(imxrt_adc));
    imxrtDac->info = (struct nxp_adc_info *)dev->info;
    adc_base = imxrtDac->info->adc_base;
    adcDevice = &imxrtDac->adc;
    adcDevice->ops = &imxrt_adc_ops;

    if (kStatus_Success != ADC_DoAutoCalibration(adc_base))
    {
        os_kprintf("ADC_DoAutoCalibration Failed.\r\n");
    }
    
    result = os_hw_adc_register(adcDevice, dev->name, OS_DEVICE_FLAG_RDWR, adc_base);
    if (result != OS_EOK)
    {
        os_kprintf("register %s device failed\n", dev->name);
    }

    return result;
}

OS_DRIVER_INFO imxrt_adc_driver = {
    .name   = "ADC_Type",
    .probe  = imxrt_adc_probe,
};

OS_DRIVER_DEFINE(imxrt_adc_driver,"2");

#endif


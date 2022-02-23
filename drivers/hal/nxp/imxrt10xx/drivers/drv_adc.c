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
 
#include <board.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>

#include "fsl_common.h"
#include "fsl_adc.h"
#include "drv_adc.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include <drv_log.h>

struct os_imxrt_adc
{
    struct os_adc_device    adc;
    struct nxp_adc_info    *info;
    os_uint8_t              status;
};

static os_err_t imxrt_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    struct os_imxrt_adc *imxrt_adc = (struct os_imxrt_adc *)dev;
    
    if (!enable)
    {
        imxrt_adc->status = OS_ADC_DISABLE;
    }
    else
    {
        imxrt_adc->status = OS_ADC_ENABLE;
    }
    
    return OS_EOK;
}

static os_err_t imxrt_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static os_err_t imxrt_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{
    os_uint64_t value = 0;
    uint32_t channelGroup = 0xFFFF;
    adc_channel_config_t config;
    
    struct os_imxrt_adc *imxrt_adc = (struct os_imxrt_adc *)dev;

    if (imxrt_adc->status != OS_ADC_ENABLE)
    {
        LOG_E(DRV_EXT_TAG,"adc disabled! please enable adc first!");
        return OS_ERROR;
    }
#ifdef ADC1
    if (imxrt_adc->info->base == ADC1)
    {
        channelGroup = 0;
    }
#endif

#ifdef ADC2
        if (imxrt_adc->info->base == ADC2)
        {
            channelGroup = 1;
        }
#endif

    if (channelGroup == 0xFFFF)
    {
        LOG_E(DRV_EXT_TAG,"adc device not support!");
        return OS_ERROR;
    }
    
    config.channelNumber = channel;
    config.enableInterruptOnConversionCompleted = false;

    ADC_SetChannelConfig(imxrt_adc->info->base, 0, &config);

    while (0U == ADC_GetChannelStatusFlags(imxrt_adc->info->base, 0))
    {
        continue;
    }

    value = ADC_GetChannelConversionValue(imxrt_adc->info->base, 0);
    
    *buff = (os_int32_t)(value * dev->mult >> dev->shift);

    return OS_EOK;
}

static const struct os_adc_ops imxrt_adc_ops = {
    .adc_enabled            = imxrt_adc_enabled,
    .adc_control            = imxrt_adc_control,
    .adc_read               = imxrt_adc_read,
};

static os_err_t imxrt_adc_probe_check(struct os_imxrt_adc *imxrt_adc)
{
    os_uint32_t value = 0;

    switch(imxrt_adc->info->config->resolution)
    {
    case kADC_Resolution8Bit:
        value = (1UL << 8) - 1;
    break;
    case kADC_Resolution10Bit:
        value = (1UL << 10) - 1;
    break;
    case kADC_Resolution12Bit:
        value = (1UL << 12) - 1;
    break;
    default:
        LOG_E(DRV_EXT_TAG,"adc resolution just support 8/10/12bits!");
        return OS_ERROR;
    }
    
    imxrt_adc->adc.max_value = value;

    return OS_EOK;
}

static int imxrt_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    int result = OS_EOK;

    struct os_imxrt_adc *imxrt_adc = OS_NULL;
    
    imxrt_adc = os_calloc(1, sizeof(struct os_imxrt_adc)); 
    OS_ASSERT(imxrt_adc);

    imxrt_adc->info = (struct nxp_adc_info *)dev->info;
    imxrt_adc->status = OS_ADC_DISABLE;

    imxrt_adc->adc.ops         = &imxrt_adc_ops;
    imxrt_adc->adc.max_value   = 0;
    imxrt_adc->adc.ref_low     = 0;                 /* ref 0 - 3.3v */
    imxrt_adc->adc.ref_hight   = 3300;

    result = imxrt_adc_probe_check(imxrt_adc);
    if (result != OS_EOK)
    {
        os_free(imxrt_adc);
        LOG_E(DRV_EXT_TAG,"adc config check has errors! not register!");
        return OS_ERROR;
    }

    if (kStatus_Success != ADC_DoAutoCalibration(imxrt_adc->info->base))
    {
        LOG_E(DRV_EXT_TAG, "ADC_DoAutoCalibration Failed!");
    }
    
    result = os_hw_adc_register(&imxrt_adc->adc, dev->name, OS_NULL);
    if (result != OS_EOK)
    {
        os_free(imxrt_adc);
        LOG_E(DRV_EXT_TAG, "register %s device failed", dev->name);
    }

    return result;
}

OS_DRIVER_INFO imxrt_adc_driver = {
    .name   = "ADC_Type",
    .probe  = imxrt_adc_probe,
};

OS_DRIVER_DEFINE(imxrt_adc_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);



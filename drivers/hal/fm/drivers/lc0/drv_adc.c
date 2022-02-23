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
 * @brief       This file implements adc driver for ht.
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

#define DBG_TAG "drv.adc"
#include <dlog.h>

#define ADC_VREF     (*((os_uint16_t *)(0x1FFFFB08)))
#define ADC_TS       (*((os_uint16_t *)(0x1FFFFa92)))


/*max external adc channel number*/
#define FM33_EXT_ADC_CHANNEL_MAX   11

/*default:period = 1s*/
#define ADC_WAIT_TIMEOUT ((uint32_t)0x8000)
#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

struct fm33_adc
{
    struct os_adc_device  adc;
    ADC_HandleTypeDef    *hadc;
    os_uint32_t           timeout;
    os_uint8_t            status;
};

struct adc_hw_info
{
    os_uint8_t         index;
    os_uint32_t        channel;
    GPIO_Type         *port;
    os_uint32_t        pin;
};


const struct adc_hw_info adcs[] = 
{
    /*index | channel              | port | pin           */
    {0,     FL_ADC_EXTERNAL_CH0    , GPIOC, FL_GPIO_PIN_9 },
    {1,     FL_ADC_EXTERNAL_CH1    , GPIOC, FL_GPIO_PIN_10},
    {2,     FL_ADC_EXTERNAL_CH2    , GPIOC, FL_GPIO_PIN_11},
    {3,     FL_ADC_EXTERNAL_CH3    , GPIOD, FL_GPIO_PIN_0 },
    {4,     FL_ADC_EXTERNAL_CH4    , GPIOD, FL_GPIO_PIN_1 },
    {5,     FL_ADC_EXTERNAL_CH5    , GPIOD, FL_GPIO_PIN_2 },
    {6,     FL_ADC_EXTERNAL_CH6    , GPIOA, FL_GPIO_PIN_13},
    {7,     FL_ADC_EXTERNAL_CH7    , GPIOA, FL_GPIO_PIN_14},
    {8,     FL_ADC_EXTERNAL_CH8    , GPIOC, FL_GPIO_PIN_7 },
    {9,     FL_ADC_EXTERNAL_CH9    , GPIOC, FL_GPIO_PIN_8 },
    {10,    FL_ADC_EXTERNAL_CH10   , GPIOA, FL_GPIO_PIN_15},
    {11,    FL_ADC_EXTERNAL_CH11   , GPIOC, FL_GPIO_PIN_6 },
    {12,    FL_ADC_INTERNAL_TS     , NULL,  NULL          },
    {13,    FL_ADC_INTERNAL_VREF1P2, NULL,  NULL          },
  /*{14,    FL_ADC_INTERNAL_OPA1   , NULL,  NULL          },*/
  /*{15,    FL_ADC_INTERNAL_OPA2   , NULL,  NULL          },*/
};
    
static const struct adc_hw_info *get_adc_index(os_uint32_t channel)
{
    const struct adc_hw_info *index = OS_NULL;

    if(channel < ITEM_NUM(adcs))
    {
        index = &adcs[channel];
    }
    return index;
}

static void fm33_adc_common_init(void)
{
    FL_ADC_CommonInitTypeDef    CommonInitStruct;

    CommonInitStruct.clockSource = FL_RCC_ADC_CLK_SOURCE_RCHF;
    CommonInitStruct.clockPrescaler = FL_RCC_ADC_PSC_DIV8;

    FL_ADC_CommonInit(&CommonInitStruct );
}

static os_err_t fm33_adc_init(os_uint32_t channel)
{
    FL_GPIO_InitTypeDef       GPIO_InitStruct;
    FL_ADC_InitTypeDef        defaultInitStruct;
    const struct adc_hw_info *adc_info;

    if(channel >= ITEM_NUM(adcs))
    {
        LOG_E(DBG_TAG, " invalid adc channel[%d/%d]",channel, ITEM_NUM(adcs));
        return OS_ERROR;
    }

    adc_info = get_adc_index(channel);
    if(adc_info == OS_NULL)
    {
        LOG_E(DBG_TAG, "invalid adc info.");
        return OS_ERROR;
    }
    fm33_adc_common_init();
    
    if(channel <= FM33_EXT_ADC_CHANNEL_MAX)
    {
        GPIO_InitStruct.pin                 = adc_info->pin;
        GPIO_InitStruct.mode                = FL_GPIO_MODE_ANALOG;
        GPIO_InitStruct.outputType          = FL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.pull                = FL_DISABLE;
        GPIO_InitStruct.remapPin            = FL_DISABLE;

        FL_GPIO_Init(adc_info->port, &GPIO_InitStruct );
    }

    defaultInitStruct.conversionMode        = FL_ADC_CONV_MODE_SINGLE;
    defaultInitStruct.autoMode              = FL_ADC_SINGLE_CONV_MODE_AUTO;
    defaultInitStruct.waitMode              = FL_ENABLE;
    defaultInitStruct.overrunMode           = FL_ENABLE;
    defaultInitStruct.scanDirection         = FL_ADC_SEQ_SCAN_DIR_BACKWARD;
    defaultInitStruct.externalTrigConv      = FL_ADC_TRIGGER_EDGE_NONE;
    defaultInitStruct.triggerSource         = FL_ADC_TRGI_PA8;
    defaultInitStruct.fastChannelTime       = FL_ADC_FAST_CH_SAMPLING_TIME_4_ADCCLK;
    defaultInitStruct.lowChannelTime        = FL_ADC_SLOW_CH_SAMPLING_TIME_192_ADCCLK;
    defaultInitStruct.oversamplingMode      = FL_ENABLE;
    defaultInitStruct.overSampingMultiplier = FL_ADC_OVERSAMPLING_MUL_16X;
    defaultInitStruct.oversamplingShift     = FL_ADC_OVERSAMPLING_SHIFT_4B;

    FL_ADC_Init(ADC,&defaultInitStruct ); 

    return OS_EOK;
}

static uint32_t HAL_GetVREF1P2Sample_POLL(void)
{
    uint32_t ADCRdresult;
    uint32_t  i=0;
    FL_RCC_SetADCPrescaler(FL_RCC_ADC_PSC_DIV8);
    FL_VREF_EnableVREFBuffer(VREF);
    FL_ADC_EnableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);

    FL_ADC_ClearFlag_EndOfConversion(ADC);  
    FL_ADC_Enable(ADC);
    FL_ADC_EnableSWConversion(ADC);
    while (FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET)
    {
        if(i >= ADC_WAIT_TIMEOUT)
        {
            break;
        }
        i++;
    }
    FL_ADC_ClearFlag_EndOfConversion(ADC);
    ADCRdresult =FL_ADC_ReadConversionData(ADC);

    FL_ADC_Disable(ADC);
    FL_ADC_DisableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);
    FL_VREF_DisableVREFBuffer(VREF);

    return ADCRdresult;
}

static uint32_t HAL_GetSingleChannelSample_POLL(uint32_t channel)
{
    uint32_t ADCRdresult;
    uint32_t i=0;
    FL_RCC_SetADCPrescaler(FL_RCC_ADC_PSC_DIV1);
    FL_ADC_EnableSequencerChannel(ADC, channel);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    FL_ADC_Enable(ADC);
    FL_ADC_EnableSWConversion(ADC);

    while (FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET)
    {
        if(i >= ADC_WAIT_TIMEOUT)
        {
            break;
        }
        i++;
    }
    FL_ADC_ClearFlag_EndOfConversion(ADC);
    ADCRdresult =FL_ADC_ReadConversionData(ADC);

    FL_ADC_Disable(ADC);
    FL_ADC_DisableSequencerChannel(ADC, channel);

    return ADCRdresult;
}

static uint32_t HAL_GetSingleChannelVoltage_POLL(uint32_t channel)
{
    uint32_t Get122VSample,GetChannelVoltage;
    uint64_t GetVSample; 
    
    Get122VSample = HAL_GetVREF1P2Sample_POLL();
    GetVSample =HAL_GetSingleChannelSample_POLL(channel);
    GetChannelVoltage =  (GetVSample *3000*(ADC_VREF))/(Get122VSample*4095);

    return GetChannelVoltage;
}

static uint32_t HAL_Get_InternalSample_POLL(uint32_t channel)
{
    uint32_t ADCRdresult;
    uint32_t i=0;
    FL_RCC_SetADCPrescaler(FL_RCC_ADC_PSC_DIV8);
    FL_VREF_EnableVPTATBuffer(VREF);
    FL_ADC_EnableSequencerChannel(ADC, channel);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    FL_ADC_Enable(ADC);
    FL_ADC_EnableSWConversion(ADC);
    while (FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET)
    {
        if(i >= ADC_WAIT_TIMEOUT)
        {
            break;
        }
        i++;
    }
    FL_ADC_ClearFlag_EndOfConversion(ADC);
    ADCRdresult =FL_ADC_ReadConversionData(ADC);

    FL_ADC_Disable(ADC);
    FL_ADC_DisableSequencerChannel(ADC, channel);
    FL_VREF_DisableVPTATBuffer(VREF);

    return ADCRdresult;
}



/**
 ***********************************************************************************************************************
 * @brief           fm33_adc_poll_convert_then_read: start adc convert in poll
 *
 * @details         channel and order config in htcubeMX,"channell" is mapping of rank configed in cube,
 *
 * @attention       Attention_description_Optional
 *
 ***********************************************************************************************************************
 */
static os_err_t fm33_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{ 
    struct fm33_adc          *dev_adc;
    const struct adc_hw_info *adc_info;
    
    OS_ASSERT(dev != OS_NULL);
    dev_adc = os_container_of(dev, struct fm33_adc, adc);
    
    if (channel >= ITEM_NUM(adcs))
    {
        LOG_E(DBG_TAG, "adc channel not support![max channel = %d]",ITEM_NUM(adcs));
        return OS_ERROR;
    }

    if (dev_adc->status != OS_ADC_ENABLE)
    {
        LOG_W(DBG_TAG, "adc disabled! please enable adc first!");
        return OS_ERROR;
    }

    fm33_adc_init(channel);

    adc_info = get_adc_index(channel);

    if(channel <= FM33_EXT_ADC_CHANNEL_MAX)
    {
        *buff = (os_int32_t) HAL_GetSingleChannelVoltage_POLL(adc_info->channel);
    }
    else
    {
        *buff = (os_int32_t) HAL_Get_InternalSample_POLL(adc_info->channel);
    }

    return OS_EOK;
}

static os_err_t fm33_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    struct fm33_adc *dev_adc;

    dev_adc = os_container_of(dev, struct fm33_adc, adc);
    
    if (!enable)
        dev_adc->status = OS_ADC_DISABLE;
    else
        dev_adc->status = OS_ADC_ENABLE;
    
    return OS_EOK;
}

static os_err_t fm33_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static const struct os_adc_ops fm33_adc_ops = {
    .adc_enabled            = fm33_adc_enabled,
    .adc_control            = fm33_adc_control,
    .adc_read               = fm33_adc_read,
};

static int fm33_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;

    struct fm33_adc *h_adc = OS_NULL;
    h_adc = os_calloc(1, sizeof(struct fm33_adc)); 
    OS_ASSERT(h_adc);
    
    h_adc->hadc = (ADC_HandleTypeDef *)dev->info;
    h_adc->status = OS_ADC_DISABLE;
    
    struct os_adc_device *dev_adc = &h_adc->adc;
    dev_adc->ops       = &fm33_adc_ops;
    dev_adc->max_value = 0;
    dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;

    result = os_hw_adc_register(dev_adc,
                                dev->name,
                                NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG, "%s register fialed!", dev->name);
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO fm33_adc_driver = {
    .name   = "ADC_HandleTypeDef",
    .probe  = fm33_adc_probe,
};

OS_DRIVER_DEFINE(fm33_adc_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);

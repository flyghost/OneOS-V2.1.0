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
 * @brief       This file implements adc driver for fm33.
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
#include <drv_adc.h>

#include <dlog.h>
#define DBG_TAG "drv.adc"

#define FM33_ADC_EXT_CHANNEL_IDX_MAX    19
#define FM33_ADC_ALL_CHANNEL_IDX_MAX    26

static const struct fm33_adc_map adc_map[] = 
{
    /*|index |      channel name         | port |     pin  */
    /*external adc channel 0~19*/
    {0 ,     FL_ADC_EXTERNAL_CH0      , GPIOD, FL_GPIO_PIN_11 },
    {1 ,     FL_ADC_EXTERNAL_CH1      , GPIOD, FL_GPIO_PIN_1  },
    {2 ,     FL_ADC_EXTERNAL_CH2      , GPIOD, FL_GPIO_PIN_3  },
    {3 ,     FL_ADC_EXTERNAL_CH3      , GPIOD, FL_GPIO_PIN_5  },
    {4 ,     FL_ADC_EXTERNAL_CH4      , GPIOA, FL_GPIO_PIN_13 },
    {5 ,     FL_ADC_EXTERNAL_CH5      , GPIOA, FL_GPIO_PIN_0  },
    {6 ,     FL_ADC_EXTERNAL_CH6      , GPIOC, FL_GPIO_PIN_7  },
    {7 ,     FL_ADC_EXTERNAL_CH7      , GPIOD, FL_GPIO_PIN_0  },
    {8 ,     FL_ADC_EXTERNAL_CH8      , GPIOD, FL_GPIO_PIN_2  },
    {9 ,     FL_ADC_EXTERNAL_CH9      , GPIOD, FL_GPIO_PIN_4  },
    {10,     FL_ADC_EXTERNAL_CH10     , GPIOD, FL_GPIO_PIN_6  },
    {11,     FL_ADC_EXTERNAL_CH11     , GPIOA, FL_GPIO_PIN_14 },
    {12,     FL_ADC_EXTERNAL_CH12     , GPIOA, FL_GPIO_PIN_1  },
    {13,     FL_ADC_EXTERNAL_CH13     , GPIOC, FL_GPIO_PIN_8  },
    {14,     FL_ADC_EXTERNAL_CH14     , GPIOC, FL_GPIO_PIN_9  },
    {15,     FL_ADC_EXTERNAL_CH15     , GPIOC, FL_GPIO_PIN_10 },
    {16,     FL_ADC_EXTERNAL_CH16     , GPIOC, FL_GPIO_PIN_11 },
    {17,     FL_ADC_EXTERNAL_CH17     , GPIOC, FL_GPIO_PIN_12 },
    {18,     FL_ADC_EXTERNAL_CH18     , GPIOD, FL_GPIO_PIN_10 },
    {19,     FL_ADC_EXTERNAL_CH19     , GPIOE, FL_GPIO_PIN_9  },
    /*internal adc channel 20~26*/
    {20,     FL_ADC_INTERNAL_VREF1P2  , OS_NULL,  NULL        },
    {21,     FL_ADC_INTERNAL_TS       , OS_NULL,  NULL        },
    {22,     FL_ADC_INTERNAL_AVREF    , OS_NULL,  NULL        },
    {23,     FL_ADC_INTERNAL_VBAT_DIV3, OS_NULL,  NULL        },
    {24,     FL_ADC_INTERNAL_VDD_DIV3 , OS_NULL,  NULL        },
    {25,     FL_ADC_INTERNAL_DAC      , OS_NULL,  NULL        },
    {26,     FL_ADC_INTERNAL_OPA      , OS_NULL,  NULL        },
};
static os_err_t fm33_get_adc_channel_map(os_uint32_t channel, const struct fm33_adc_map **map_info)
{
    if(channel > FM33_ADC_ALL_CHANNEL_IDX_MAX)
    {
        return OS_ERROR;
    }

    *map_info = &adc_map[channel];

    OS_ASSERT(channel == adc_map[channel].index);

    return OS_EOK;
}
static os_uint32_t fm33_hal_sample_vref(void)
{
    os_uint16_t ADCRdresult;
    FL_CMU_SetADCPrescaler(FL_CMU_ADC_PSC_DIV8);
    FL_VREF_EnableVREFBuffer(VREF);
    FL_ADC_EnableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    FL_ADC_Enable(ADC);
    FL_ADC_EnableSWConversion(ADC);


    while(FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    ADCRdresult = FL_ADC_ReadConversionData(ADC); 

    FL_ADC_Disable(ADC);
    FL_ADC_DisableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);
    FL_VREF_DisableVREFBuffer(VREF);

    return ADCRdresult;
}

static os_uint32_t fm33_hal_sample_single_channel(os_uint32_t channel_name)
{
    os_uint16_t ADCRdresult;
    FL_CMU_SetADCPrescaler(FL_CMU_ADC_PSC_DIV1);
    FL_ADC_EnableSequencerChannel(ADC, channel_name);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    FL_ADC_Enable(ADC);
    FL_ADC_EnableSWConversion(ADC);


    while(FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    ADCRdresult = FL_ADC_ReadConversionData(ADC);

    FL_ADC_Disable(ADC);
    FL_ADC_DisableSequencerChannel(ADC, channel_name);

    return ADCRdresult;
}

static os_uint32_t fm33_hal_sample_internal(os_uint32_t channel_name)
{
    os_uint16_t ADCRdresult;

    FL_CMU_SetADCPrescaler(FL_CMU_ADC_PSC_DIV8);
    FL_VREF_EnableVPTATBuffer(VREF);
    FL_ADC_EnableSequencerChannel(ADC, channel_name);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    FL_ADC_Enable(ADC);
    FL_ADC_EnableSWConversion(ADC);

    while(FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET);

    FL_ADC_ClearFlag_EndOfConversion(ADC);
    ADCRdresult = FL_ADC_ReadConversionData(ADC);

    FL_ADC_Disable(ADC);
    FL_ADC_DisableSequencerChannel(ADC, channel_name);
    FL_VREF_DisableVPTATBuffer(VREF);

    return ADCRdresult;
}

static os_uint32_t fm33_adc_read_channel(os_uint32_t channel)
{
    os_uint32_t                  vref_sample;
    os_uint64_t                  ch_sample;
    os_uint32_t                  ret;
    const struct fm33_adc_map   *map_info;

    fm33_get_adc_channel_map(channel, &map_info);

    vref_sample = fm33_hal_sample_vref();

    if(map_info->index <= FM33_ADC_EXT_CHANNEL_IDX_MAX)
    {
        ch_sample = fm33_hal_sample_single_channel(map_info->name);
    }
    else
    {
        ch_sample = fm33_hal_sample_internal(map_info->name);
    }

    ret = (ch_sample * 3000 * (ADC_VREF)) / (vref_sample * 4095);

    return ret;
}
static void fm33_adc_channel_init(struct fm33_adc *dev, os_uint32_t channel)
{
    FL_GPIO_InitTypeDef         GPIO_InitStruct = {0};
    const struct fm33_adc_map  *map_info;

    fm33_get_adc_channel_map(channel, &map_info);

    GPIO_InitStruct.pin            = map_info->pin;
    GPIO_InitStruct.mode           = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType     = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull           = FL_DISABLE;
    GPIO_InitStruct.remapPin       = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;

    FL_GPIO_Init(map_info->port, &GPIO_InitStruct);

    FL_ADC_CommonInit(&(dev->info->clk));

    FL_ADC_Init(ADC, &(dev->info->init));
}

#ifdef OS_USING_LPMGR
static os_err_t fm33_adc_init(os_uint32_t channel)
{
    const struct fm33_adc_map   *map_info;
    FL_GPIO_InitTypeDef          GPIO_InitStruct = {0};

    if(fm33_get_adc_channel_map(channel, &map_info) != OS_EOK)
    {
        return OS_ERROR;
    }

    GPIO_InitStruct.pin            = map_info->pin;
    GPIO_InitStruct.mode           = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType     = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull           = FL_DISABLE;
    GPIO_InitStruct.remapPin       = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;

    FL_GPIO_Init(map_info->port, &GPIO_InitStruct);
    return OS_EOK;
}

static os_err_t fm33_adc_deinit(os_uint32_t channel)
{
    const struct fm33_adc_map   *map_info;

    if(fm33_get_adc_channel_map(channel, &map_info) != OS_EOK)
    {
        return OS_ERROR;
    }

    FL_GPIO_DeInit(map_info->port, map_info->pin);
    FL_ADC_CommonDeInit();
    FL_ADC_DeInit(ADC);
    
    return OS_EOK;
}
#endif
/**
 ***********************************************************************************************************************
 * @brief           fm33_adc_poll_convert_then_read: start adc convert in poll
 *
 * @details
 *
 * @attention       Attention_description_Optional
 *
 ***********************************************************************************************************************
 */
static os_err_t fm33_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{
    struct fm33_adc             *dev_adc;
    const struct fm33_adc_map   *map_info;

    OS_ASSERT(dev != OS_NULL);
    dev_adc = os_container_of(dev, struct fm33_adc, adc);
    
    if(fm33_get_adc_channel_map(channel, &map_info) != OS_EOK)
    {
        LOG_E(DBG_TAG,"adc channel not support![max channel = %d]", FM33_ADC_ALL_CHANNEL_IDX_MAX);
        return OS_ERROR;
    }

    if (dev_adc->status != OS_ADC_ENABLE)
    {
        LOG_E(DBG_TAG,"adc disabled! please enable adc first!");
        return OS_ERROR;
    }

    fm33_adc_channel_init(dev_adc, channel);

    *buff = (os_int32_t) fm33_adc_read_channel(channel);

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

    struct fm33_adc *fm_adc = OS_NULL;
    fm_adc = os_calloc(1, sizeof(struct fm33_adc)); 
    OS_ASSERT(fm_adc);
    
    fm_adc->info = (struct fm33_adc_info *)dev->info;
    fm_adc->status = OS_ADC_DISABLE;
    
    struct os_adc_device *dev_adc = &fm_adc->adc;
    dev_adc->ops    = &fm33_adc_ops;
    dev_adc->max_value = 0;
    dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;

    result = os_hw_adc_register(dev_adc, dev->name, NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"%s register fialed!\r\n", dev->name);
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO fm33_adc_driver = {
    .name   = "ADC_Type",
    .probe  = fm33_adc_probe,
};

OS_DRIVER_DEFINE(fm33_adc_driver, DEVICE,OS_INIT_SUBLEVEL_HIGH);

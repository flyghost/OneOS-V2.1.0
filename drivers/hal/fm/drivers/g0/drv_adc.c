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


/*external adc channel number*/
#define FM33_EXT_ADC_CHANNEL_BEGIN  1
#define FM33_EXT_ADC_CHANNEL_END    8

#define ADC_WAIT_TIMEOUT ((uint32_t)0x8000)

/*Sample averaging window check*/
#if (ADC_MEAN_WINDOW < 1)
#error "adc mean window min value is 1."
#endif

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
    GPIOx_Type        *port;
    os_uint32_t        pin;
};


const struct adc_hw_info adcs[] = 
{
    /*index | channel    | port | pin       */
    /*power supply*/
    {0,     CH_VDD      , NULL,         NULL},
    /*external channel 1~8*/
    {1,     CH_IN1      , GPIOC, GPIO_Pin_12},
    {2,     CH_IN2      , GPIOC, GPIO_Pin_13},
    {3,     CH_IN3      , GPIOD, GPIO_Pin_0 },
    {4,     CH_IN4      , GPIOD, GPIO_Pin_1 },
    {5,     CH_IN5      , GPIOF, GPIO_Pin_6 },
    {6,     CH_IN6      , GPIOC, GPIO_Pin_15},
    {7,     CH_IN7      , GPIOB, GPIO_Pin_2 },
    {8,     CH_IN8      , GPIOB, GPIO_Pin_3 },
    /*internal temprature:software calibration*/
    {9,     CH_PTAT     , NULL,         NULL},
    /*internal temprature:auto calibration*/
    {10,    CH_PTAT_Auto, NULL,         NULL},
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

static void fm33_adc_channel_init(os_uint32_t channel)
{
    RCC_PERCLK_SetableEx(ANACCLK, ENABLE);
    RCC_PERCLK_SetableEx(ADCCLK, ENABLE);
    RCC_PERCLKCON2_ADCCKSEL_Set(RCC_PERCLKCON2_ADCCKSEL_RCHFDIV16);
    ANAC_ADC_Channel_SetEx(channel);
    if((channel >= FM33_EXT_ADC_CHANNEL_BEGIN) && (channel <= FM33_EXT_ADC_CHANNEL_END))
    {
        ANAC_ADCTRIM_Write(const_adc_TrimV_3FF);
    }

    ANAC_ADCCON_ADC_IE_Setable(DISABLE);
    ANAC_ADCCON_ADC_EN_Setable(DISABLE);
}
static os_err_t fm33_adc_wait_complete(void)
{
    os_err_t    ret = OS_EOK;
    os_uint32_t timeout = ADC_WAIT_TIMEOUT;

    while(SET != ANAC_ADCIF_ADC_IF_Chk())
    {
        timeout--;
        if(timeout == 0)
            break;
    }
    
    if(timeout == 0)
    {
        LOG_W(DBG_TAG, "wait timeout.");
        ret = OS_ETIMEOUT;
    }

    return ret;

}

static os_err_t fm33_adc_init(os_uint32_t channel)
{
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

    if((channel >= FM33_EXT_ADC_CHANNEL_BEGIN) && (channel <= FM33_EXT_ADC_CHANNEL_END))
    {
        AnalogIO(adc_info->port, adc_info->pin);
    }

    fm33_adc_channel_init(adc_info->channel);

    return OS_EOK;
}


static os_err_t fm33_adc_deinit(os_uint32_t channel)
{
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

    if((channel >= FM33_EXT_ADC_CHANNEL_BEGIN) && (channel <= FM33_EXT_ADC_CHANNEL_END))
    {
        CloseeIO(adc_info->port, adc_info->pin);
    }

    ANAC_Deinit();

    return OS_EOK;
}
static os_uint32_t fm33_adc_sample_once(void)
{
    os_uint32_t ret = 0;

    ANAC_ADCIF_ADC_IF_Clr();

    if(OS_EOK == fm33_adc_wait_complete())
    {
        ret = ANAC_ADCDATA_Read();
    }

    return ret;
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
    os_uint32_t               sample_sum;
    os_size_t                 i;
    
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

    ANAC_ADCCON_ADC_EN_Setable(ENABLE);

    /*drop firt sample*/
    fm33_adc_sample_once();

    sample_sum = 0;
    for(i = 0; i < ADC_MEAN_WINDOW; i++)
    {
        sample_sum += fm33_adc_sample_once();
    }
    sample_sum /= ADC_MEAN_WINDOW;

    if(channel > FM33_EXT_ADC_CHANNEL_END)
    {
        *buff = (os_int32_t) ANAC_ADC_TemperatureCalc((float)sample_sum, 5);
    }
    else
    {
        *buff = (os_int32_t) ANAC_ADC_VoltageCalc(sample_sum, 5);
    }

    fm33_adc_deinit(channel);
    ANAC_ADCCON_ADC_EN_Setable(DISABLE);

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

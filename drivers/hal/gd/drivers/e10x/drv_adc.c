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
 * @brief       This file implements adc driver for gd32.
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
#include <os_irq.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include <drv_log.h>
#include <gd32f30x_dma.h>
#include <gd32f30x_adc.h>
#include <drv_adc.h>

//#define ADC_REGULAR_TEST
#define GD32_ADC_MAX_NUM 3

#ifdef ADC_REGULAR_TEST
os_uint16_t adc_test_buf[30];
#endif

struct gd32_adc
{
    struct os_adc_device adc;
    ADC_HandleTypeDef *adc_periph[GD32_ADC_MAX_NUM];
};

struct gd32_adc *gd_adc = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           gd32_adc_poll_convert_then_read: start adc convert in poll
 *
 * @details         channel and order config in gd32cubeMX,"channell" is mapping of rank configed in cube,
 *
 * @attention       Attention_description_Optional
 *
 ***********************************************************************************************************************
 */
static os_err_t gd32_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{  
    struct gd32_adc *dev_adc;

    OS_ASSERT(dev != OS_NULL);

    dev_adc = os_container_of(dev, struct gd32_adc, adc);

    for (os_uint8_t i = 0; i < GD32_ADC_MAX_NUM;i++)
    {
        if (dev_adc->adc_periph[i] != OS_NULL)
        {
            dev_adc->adc_periph[i]->channel = ADC0;
            channel = ADC_CHANNEL_1;
            if (channel > ADC_CHANNEL_17)
            {
                return OS_EINVAL;
            }

            if (channel > ADC_CHANNEL_15)
            {
                adc_regular_channel_config(dev_adc->adc_periph[i]->channel, 0, channel, ADC_SAMPLETIME_239POINT5);
            }
            else
            {
                adc_regular_channel_config(dev_adc->adc_periph[i]->channel, 0, channel, ADC_SAMPLETIME_7POINT5);
            }
            adc_software_trigger_enable(dev_adc->adc_periph[i]->channel, ADC_REGULAR_CHANNEL);
            while (SET != adc_flag_get(dev_adc->adc_periph[i]->channel, ADC_FLAG_EOC));
            adc_flag_clear(dev_adc->adc_periph[i]->channel, ADC_FLAG_EOC);
            *buff = adc_regular_data_read(dev_adc->adc_periph[i]->channel);
        }
    }
    return OS_EOK;

}

static os_err_t gd32_adc_open(struct os_adc_device *dev)
{
    struct gd32_adc *dev_adc;

    dev_adc = os_container_of(dev, struct gd32_adc, adc);

    for (os_int8_t i = GD32_ADC_MAX_NUM - 1; i >= 0; i--)
    {
        dev_adc->adc_periph[i]->channel = ADC0;
        if (dev_adc->adc_periph[i] != OS_NULL)
        {
            adc_deinit(dev_adc->adc_periph[i]->channel);
            
            /* ADC mode config */
            adc_mode_config(ADC_MODE_FREE);
            /* ADC data alignment config */
            adc_data_alignment_config(dev_adc->adc_periph[i]->channel, ADC_DATAALIGN_RIGHT);
            /* ADC channel length config */
            adc_channel_length_config(dev_adc->adc_periph[i]->channel, ADC_REGULAR_CHANNEL, 1U);
            
            /* ADC trigger config */
            adc_external_trigger_source_config(dev_adc->adc_periph[i]->channel, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); 
            /* ADC external trigger config */
            adc_external_trigger_config(dev_adc->adc_periph[i]->channel, ADC_REGULAR_CHANNEL, ENABLE);

            /* enable ADC interface */
            adc_enable(dev_adc->adc_periph[i]->channel);
            os_task_msleep(1U);
            /* ADC calibration and reset calibration */
            adc_calibration_enable(dev_adc->adc_periph[i]->channel);
        }
    }
    return OS_EOK;
}

static os_err_t gd32_adc_close(struct os_adc_device *dev)
{
    struct gd32_adc *dev_adc;

    dev_adc = os_container_of(dev, struct gd32_adc, adc);

    for (os_int8_t i = GD32_ADC_MAX_NUM - 1; i >= 0;i--)
    {
        dev_adc->adc_periph[i]->channel = ADC0;
        if (dev_adc->adc_periph[i] != OS_NULL)
        {
            adc_disable(dev_adc->adc_periph[i]->channel);
        }
    }
    return OS_EOK;

}

static os_err_t gd32_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    if (!enable)
    {
        return gd32_adc_close(dev);
    }
    else
    {
        return gd32_adc_open(dev);
    }
}

static os_err_t gd32_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}


static const struct os_adc_ops gd32_adc_ops = {
    .adc_enabled            = gd32_adc_enabled,
    .adc_control            = gd32_adc_control,
    .adc_read               = gd32_adc_read,
};

void gd32_adc_gpio_config(void)
{
        /* enable GPIOA clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC0);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);
    /* config the GPIO as analog mode */
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_1);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_3);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_4);
}

void gd32_adc_config(void)
{
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1U);
    
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); 
    /* ADC external trigger config */
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

    /* enable ADC interface */
    adc_enable(ADC0);
    os_task_msleep(1U);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
}

static int gd32_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if (gd_adc == OS_NULL)
    {
        gd_adc = os_calloc(1, sizeof(struct gd32_adc)); 
        OS_ASSERT(gd_adc);
    
        gd_adc->adc_periph[0] = OS_NULL;
        gd_adc->adc_periph[1] = OS_NULL;
        gd_adc->adc_periph[2] = OS_NULL;

        struct os_adc_device *dev_adc = &gd_adc->adc;
        dev_adc->ops    = &gd32_adc_ops;
        dev_adc->max_value = 0;
        dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
        dev_adc->ref_hight = 3300;

        gd32_adc_gpio_config();
        gd32_adc_config();

        os_hw_adc_register(dev_adc,
                                    "adc",
                                    OS_DEVICE_FLAG_RDWR,
                                    NULL);
        }
        for (os_uint8_t i = 0; i < GD32_ADC_MAX_NUM;i++)
        {
            if (gd_adc->adc_periph[i] == OS_NULL)
            {
                gd_adc->adc_periph[i] = (ADC_HandleTypeDef *)dev->info;
                break;
            } 
        }

        return OS_EOK;
}

OS_DRIVER_INFO gd32_adc_driver = {
    .name   = "ADC_HandleTypeDef",
    .probe  = gd32_adc_probe,
};

OS_DRIVER_DEFINE(gd32_adc_driver,"2");

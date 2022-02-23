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
#include <gd32f4xx_dma.h>
#include <gd32f4xx_adc.h>
#include <drv_adc.h>


#ifdef ADC_REGULAR_TEST
os_uint16_t adc_test_buf[30];
#endif

struct gd32_adc
{
    struct os_adc_device adc;
    os_uint32_t periph;
    os_list_node_t list;
};

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

    if (channel > ADC_CHANNEL_17)
    {
        return OS_EINVAL;
    }

    if (channel > ADC_CHANNEL_15)
    {
        adc_regular_channel_config(dev_adc->periph, 0, channel, ADC_SAMPLETIME_480);
    }
    else
    {
        adc_regular_channel_config(dev_adc->periph, 0, channel, ADC_SAMPLETIME_15);
    }
    adc_software_trigger_enable(dev_adc->periph, ADC_REGULAR_CHANNEL);
    while (SET != adc_flag_get(dev_adc->periph, ADC_FLAG_EOC));
    adc_flag_clear(dev_adc->periph, ADC_FLAG_EOC);
    *buff = adc_regular_data_read(dev_adc->periph);
    
    return OS_EOK;

}

static os_err_t gd32_adc_open(struct os_adc_device *dev)
{
    struct gd32_adc *dev_adc;

    OS_ASSERT(dev != OS_NULL);

    dev_adc = os_container_of(dev, struct gd32_adc, adc);

    adc_deinit();
    
    /* ADC mode config */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    /* ADC data alignment config */
    adc_data_alignment_config(dev_adc->periph, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(dev_adc->periph, ADC_REGULAR_CHANNEL, 1U);
    
    /* ADC trigger config */
    adc_external_trigger_source_config(dev_adc->periph, ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_T0_CH0); 
    /* ADC external trigger config */
    adc_external_trigger_config(dev_adc->periph, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    /* enable ADC interface */
    adc_enable(dev_adc->periph);
    os_task_msleep(1U);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(dev_adc->periph);
    
    return OS_EOK;
}

static os_err_t gd32_adc_close(struct os_adc_device *dev)
{
    struct gd32_adc *dev_adc;

    dev_adc = os_container_of(dev, struct gd32_adc, adc);

    adc_disable(dev_adc->periph);

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

void gd32_adc_gpio_config(os_uint32_t periph)
{
        /* enable GPIOA clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC0);
    /* config ADC clock */
    adc_clock_config(ADC_ADCCK_PCLK2_DIV6);
    
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
}

void gd32_adc_config(os_uint32_t periph)
{
    /* ADC mode config */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    /* ADC data alignment config */
    adc_data_alignment_config(periph, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(periph, ADC_REGULAR_CHANNEL, 1U);
    
    /* ADC trigger config */
    adc_external_trigger_config(periph, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    /* ADC external trigger config */
    adc_external_trigger_config(periph, ADC_REGULAR_CHANNEL, ENABLE);

    /* enable ADC interface */
    adc_enable(periph);
    os_task_msleep(1U);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(periph);
}

static os_list_node_t gd32_adc_list = OS_LIST_INIT(gd32_adc_list);

static int gd32_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct gd32_adc_info *adc_info = (struct gd32_adc_info*)dev->info;
    struct gd32_adc *gd_adc = os_calloc(1, sizeof(struct gd32_adc)); 
    OS_ASSERT(gd_adc);

    gd_adc->periph = adc_info->adc_periph;

    struct os_adc_device *dev_adc = &gd_adc->adc;
    dev_adc->ops = &gd32_adc_ops;

    dev_adc->max_value = 0;
    dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;

    gd32_adc_gpio_config(adc_info->adc_periph);
    gd32_adc_config(adc_info->adc_periph);

    os_list_add_tail(&gd32_adc_list, &gd_adc->list);
    os_hw_adc_register(dev_adc,
                                dev->name,
                                OS_DEVICE_FLAG_RDWR,
                                NULL);

    return OS_EOK;
}

OS_DRIVER_INFO gd32_adc_driver = {
    .name   = "ADC_HandleTypeDef",
    .probe  = gd32_adc_probe,
};

OS_DRIVER_DEFINE(gd32_adc_driver,"2");

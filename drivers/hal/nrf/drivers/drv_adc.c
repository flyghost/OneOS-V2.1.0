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
 * @brief       This file implements adc driver for nrf5.
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

#include <drv_adc.h>
#include "nrf_delay.h"

#include <dlog.h>
#define DBG_TAG "drv.adc"

#define NRF5_ADC_MAX_CHANNEL 8

static nrf_saadc_value_t s_bufferPool[SAMPLES_IN_BUFFER];

struct nrf5_adc
{
    struct os_adc_device adc;
    struct nrf5_adc_info *info;
    os_list_node_t list;
};

/**
 ***********************************************************************************************************************
 * @brief           nrf5_adc_poll_convert_then_read: start adc convert in poll
 *
 * @details         channel and order config in nrf5cubeMX,"channell" is mapping of rank configed in cube,
 *
 * @attention       Attention_description_Optional
 *
 ***********************************************************************************************************************
 */
static os_err_t nrf5_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{  
    if(channel >= NRF5_ADC_MAX_CHANNEL)
    {

        LOG_E(DBG_TAG,"adc channel %d cannot find!\n", channel);

        return OS_ERROR;
    }
        
    nrf_saadc_input_t input_ain;
    input_ain = (nrf_saadc_input_t)(channel + 1);
    
    nrf_saadc_channel_config_t channelConfig = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(input_ain);
    nrf_drv_saadc_channel_init(channel, &channelConfig);
    nrf_drv_saadc_buffer_convert(s_bufferPool, SAMPLES_IN_BUFFER);
    
    nrf_drv_saadc_sample();
    nrf_delay_ms(10);
    *buff = (os_int32_t)(s_bufferPool[0] * 3600 / 1024);
    if(*buff < dev->ref_low)
        *buff = dev->ref_low;
    return OS_EOK;

}

static void adcCallbackFunc(nrf_drv_saadc_evt_t const *pEvent)
{
    if(pEvent->type == NRF_DRV_SAADC_EVT_DONE)
    {
        nrf_drv_saadc_buffer_convert(pEvent->data.done.p_buffer, SAMPLES_IN_BUFFER); 
    }
}

void nrf_adc_init(struct nrf5_adc_info *info)
{
    nrf_drv_saadc_init(NULL, adcCallbackFunc);
}

static os_err_t nrf5_adc_open(struct os_adc_device *dev)
{
    struct nrf5_adc *adc = (struct nrf5_adc *)dev->parent.user_data;
    nrf_adc_init(adc->info);
    return OS_EOK;
}

static os_err_t nrf5_adc_close(struct os_adc_device *dev)
{
    nrfx_saadc_uninit();
    return OS_EOK;

}

static os_err_t nrf5_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    if (!enable)
    {
        return nrf5_adc_close(dev);
    }
    else
    {
        return nrf5_adc_open(dev);
    }
}

static os_err_t nrf5_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}


static const struct os_adc_ops nrf5_adc_ops = {
    .adc_enabled            = nrf5_adc_enabled,
    .adc_control            = nrf5_adc_control,
    .adc_read               = nrf5_adc_read,
};

static os_list_node_t nrf5_adc_list = OS_LIST_INIT(nrf5_adc_list);

static int nrf5_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct nrf5_adc *nrf5_adc = os_calloc(1, sizeof(struct nrf5_adc)); 
    OS_ASSERT(nrf5_adc);

    struct os_adc_device *dev_adc = &nrf5_adc->adc;
    struct nrf5_adc_info *dev_info = (struct nrf5_adc_info*)dev->info;
    nrf5_adc->info = dev_info;
    dev_adc->ops = &nrf5_adc_ops;

    dev_adc->max_value = 0;
    dev_adc->ref_low   = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;

    nrf_adc_init(dev_info);

    os_list_add_tail(&nrf5_adc_list, &nrf5_adc->list);
    os_hw_adc_register(dev_adc,
                                dev->name,
                                NULL);

    return OS_EOK;
}

OS_DRIVER_INFO nrf5_adc_driver = {
    .name   = "ADC_HandleTypeDef",
    .probe  = nrf5_adc_probe,
};

OS_DRIVER_DEFINE(nrf5_adc_driver, DEVICE,OS_INIT_SUBLEVEL_HIGH);

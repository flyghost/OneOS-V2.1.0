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
 * @file        drv_dac.c
 *
 * @brief       This file implements adc driver for gd32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <string.h>
#include <os_memory.h>
#include <misc/dac.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.dac"
#include <drv_log.h>

#include "drv_dac.h"

struct gd32_dac {
    os_dac_device_t    dac;
    os_uint32_t dac_periph;
};

static os_uint32_t gd32_dac_channel(os_uint32_t channel)
{
    os_uint32_t os_ch;

    switch (channel)
    {
    case 1:
        os_ch = 1;
        break;

    default:
        os_ch = 0xFFFF;
        break;
    }

    return os_ch;
}

static os_err_t gd32_dac_enabled(os_dac_device_t *dac, os_uint32_t channel, os_bool_t enabled)
{
    struct gd32_dac *os_dac;
    os_uint32_t       os_ch;
    
    OS_ASSERT(dac != OS_NULL);

    os_dac = (struct gd32_dac *)dac;
    os_ch  = gd32_dac_channel(channel);

    if (os_ch == 0xFFFF)
    {
        LOG_EXT_E("dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    if (enabled)
    {
        dac_enable(os_dac->dac_periph);
    }
    else
    {
        dac_disable(os_dac->dac_periph);
    }

    return OS_EOK;
}

static os_err_t gd32_dac_write(os_dac_device_t *dac, os_uint32_t channel, os_uint32_t value)
{
    struct gd32_dac *os_dac;
    os_uint32_t       os_ch;

    OS_ASSERT(dac != OS_NULL);

    os_dac = (struct gd32_dac *)dac;
    os_ch  = gd32_dac_channel(channel);

    if (os_ch == 0xFFFF)
    {
        LOG_EXT_E("dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    /* set dac val */
    dac_data_set(os_dac->dac_periph, DAC_ALIGN_12B_R, value);
    return OS_EOK;
}

static const struct os_dac_ops gd32_dac_ops = {
    .enabled = gd32_dac_enabled,
    .write   = gd32_dac_write,
};


void gd32_dac_config(struct gd32_dac_info * dac_info)
{
    uint32_t dac_periph = dac_info->dac_periph;
    /* enable GPIOA clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable DAC clock */
    rcu_periph_clock_enable(RCU_DAC);
    
    gpio_init(RCU_GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, dac_info->pin);
    
    dac_trigger_disable(dac_periph);
    dac_wave_mode_config(dac_periph, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(dac_periph);
    
    /* enable DAC0 and set data */
    dac_enable(dac_periph);
    dac_data_set(dac_periph, DAC_ALIGN_12B_L, 0);
}

static int gd32_dac_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct gd32_dac *dac;
    
    struct gd32_dac_info * dac_info;

    dac = os_calloc(1, sizeof(struct gd32_dac));

    OS_ASSERT(dac);

    dac_info = (struct gd32_dac_info *) dev->info;
    dac->dac_periph = (os_uint32_t)dac_info->dac_periph;
    
    dac->dac.max_value = (1UL << 12) - 1;    /* 12bit */
    dac->dac.ref_low   = 0;                 /* ref 0 - 3.3v */
    dac->dac.ref_hight = 3300;
    dac->dac.ops       = &gd32_dac_ops;

    gd32_dac_config(dac_info);
    
    if (os_dac_register(&dac->dac, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_EXT_E("dac device register failed.");
        return OS_ERROR;
    }
    LOG_EXT_D("dac device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO gd32_dac_driver = {
    .name   = "DAC_Type",
    .probe  = gd32_dac_probe,
};

OS_DRIVER_DEFINE(gd32_dac_driver, "1");


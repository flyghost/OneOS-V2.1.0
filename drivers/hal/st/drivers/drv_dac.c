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
 * @brief       This file implements adc driver for stm32.
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

#define DBG_TAG "drv.dac"
#include <dlog.h>

struct stm32_dac {
    os_dac_device_t    dac;
    DAC_HandleTypeDef *hdac;
};

static os_uint32_t stm32_dac_channel(os_uint32_t channel)
{
    os_uint32_t st_ch;

    switch (channel)
    {
#ifdef DAC_CHANNEL_1
    case 1:
        st_ch = DAC_CHANNEL_1;
        break;
#endif
#ifdef DAC_CHANNEL_2
    case 2:
        st_ch = DAC_CHANNEL_2;
        break;
#endif
    default:
        st_ch = 0xFFFF;
        break;
    }

    return st_ch;
}

static os_err_t stm32_dac_enabled(os_dac_device_t *dac, os_uint32_t channel, os_bool_t enabled)
{
    struct stm32_dac *st_dac;
    os_uint32_t       st_ch;
    
    OS_ASSERT(dac != OS_NULL);

    st_dac = (struct stm32_dac *)dac;
    st_ch  = stm32_dac_channel(channel);

    if (st_ch == 0xFFFF)
    {
        LOG_E(DBG_TAG,"dac channel %d cannot find!\r\n", channel);
        return OS_ERROR;
    }

    if (enabled)
    {
        HAL_DAC_Start(st_dac->hdac, st_ch);
    }
    else
    {
        HAL_DAC_Stop(st_dac->hdac, st_ch);
    }

    return OS_EOK;
}

static os_err_t stm32_dac_write(os_dac_device_t *dac, os_uint32_t channel, os_uint32_t value)
{
    struct stm32_dac *st_dac;
    os_uint32_t       st_ch;

    OS_ASSERT(dac != OS_NULL);

    st_dac = (struct stm32_dac *)dac;
    st_ch  = stm32_dac_channel(channel);

    if (st_ch == 0xFFFF)
    {
        LOG_E(DBG_TAG,"dac channel %d cannot find!\r\n", channel);
        return OS_ERROR;
    }

    /* set dac val */
    HAL_DAC_SetValue(st_dac->hdac, st_ch, DAC_ALIGN_12B_R, value);
    return OS_EOK;
}

static const struct os_dac_ops stm32_dac_ops = {
    .enabled = stm32_dac_enabled,
    .write   = stm32_dac_write,
};

static int stm32_dac_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_dac *dac;

    dac = os_calloc(1, sizeof(struct stm32_dac));

    OS_ASSERT(dac);

    dac->hdac = (DAC_HandleTypeDef *)dev->info;
    
    dac->dac.max_value = (1UL << 12) - 1;    /* 12bit */
    dac->dac.ref_low   = 0;                 /* ref 0 - 3.3v */
    dac->dac.ref_hight = 3300;
    dac->dac.ops       = &stm32_dac_ops;

    if (os_dac_register(&dac->dac, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DBG_TAG,"dac device register failed.");
        return OS_ERROR;
    }
    LOG_D(DBG_TAG,"dac device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO stm32_dac_driver = {
    .name   = "DAC_HandleTypeDef",
    .probe  = stm32_dac_probe,
};

OS_DRIVER_DEFINE(stm32_dac_driver,PREV,OS_INIT_SUBLEVEL_LOW);


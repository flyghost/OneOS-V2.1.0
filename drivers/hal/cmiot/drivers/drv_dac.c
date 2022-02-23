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
 * @brief       This file implements dac driver for cm32.
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
#include <drv_log.h>
#include <drv_dac.h>
#include <cm32m101a_dac.h>
#include <cm32m101a_gpio.h>
#include "board.h"

#define DBG_TAG "drv.dac"
#include <dlog.h>

struct cm32_dac {
    os_dac_device_t dac;
};

static os_uint32_t cm32_dac_channel(os_uint32_t channel)
{
    os_uint32_t cm_ch;

    switch (channel)
    {
    case 1:
        cm_ch = 1;
        break;
    default:
        cm_ch = ~0;
        break;
    }

    return cm_ch;
}

static os_err_t cm32_dac_enabled(os_dac_device_t *dac, os_uint32_t channel, os_bool_t enabled)
{

    os_uint32_t cm_ch;

    cm_ch = cm32_dac_channel(channel);

    if (cm_ch == ~0)
    {
        LOG_E(DBG_TAG, "dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    if (enabled)
    {
        DAC_Enable(ENABLE);
    }
    else
    {
        DAC_Enable(DISABLE);
    }

    return OS_EOK;
}

static os_err_t cm32_dac_write(os_dac_device_t *dac, os_uint32_t channel, os_uint32_t value)
{
    os_uint32_t cm_ch;

    cm_ch = cm32_dac_channel(channel);

    if (cm_ch == ~0)
    {
        LOG_E(DBG_TAG, "dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    DAC_SetChData(DAC_ALIGN_R_12BIT, (uint16_t)value);
    DAC_SoftTrgEnable(ENABLE);

    return OS_EOK;
}

static const struct os_dac_ops cm32_dac_ops = {
    .enabled = cm32_dac_enabled,
    .write   = cm32_dac_write,
};

static inline void __os_hw_dac_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    DAC_InitType DAC_InitStructure;

    /* GPIOA Periph clock enable */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);

    GPIO_InitStructure.Pin       = GPIO_PIN_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Input;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure);

    /* DAC Periph clock enable */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_DAC, ENABLE);
    /* DAC channel1 Configuration */
    DAC_InitStructure.Trigger          = DAC_TRG_SOFTWARE;
    DAC_InitStructure.WaveGen          = DAC_WAVEGEN_NONE;
    DAC_InitStructure.LfsrUnMaskTriAmp = DAC_UNMASK_LFSRBIT0;
    DAC_InitStructure.BufferOutput     = DAC_BUFFOUTPUT_ENABLE;
    DAC_Init(&DAC_InitStructure);
}

static int cm32_dac_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct cm32_dac *dac;

    dac = os_calloc(1, sizeof(struct cm32_dac));

    OS_ASSERT(dac);

    __os_hw_dac_init();

    dac->dac.max_value = (1UL << 12) - 1;    /* 12bit */
    dac->dac.ref_low   = 0;                  /* ref 0 - 3.3v */
    dac->dac.ref_hight = 3300;
    dac->dac.ops       = &cm32_dac_ops;

    if (os_dac_register(&dac->dac, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DBG_TAG, "dac device register failed.");
        return OS_ERROR;
    }
    LOG_D(DBG_TAG, "dac device register success.");

    return OS_EOK;
}

OS_DRIVER_INFO cm32_dac_driver = {
    .name   = "DAC_Type",
    .probe  = cm32_dac_probe,
};

OS_DRIVER_DEFINE(cm32_dac_driver, PREV, OS_INIT_SUBLEVEL_LOW);

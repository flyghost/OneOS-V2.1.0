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
 * @brief       This file implements dac driver for hc32.
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
#include <hc_dac.h>
#include <hc_gpio.h>
#include "board.h"
#include "drv_dac.h"

#define DBG_TAG "drv.dac"
#include <dlog.h>

struct hc32_dac {
    os_dac_device_t dac;
};

static os_uint32_t hc32_dac_channel(os_uint32_t channel)
{
    os_uint32_t st_ch;

    switch (channel)
    {
    case 1:
        st_ch = 1;
        break;
    default:
        st_ch = ~0;
        break;
    }

    return st_ch;
}

static os_err_t hc32_dac_enabled(os_dac_device_t *dac, os_uint32_t channel, os_bool_t enabled)
{
    os_uint32_t hc_ch;

    hc_ch = hc32_dac_channel(channel);

    if (hc_ch == ~0)
    {
        LOG_E(DBG_TAG, "dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    if (enabled)
    {
        Dac_Cmd(TRUE);
        Dac_DmaCmd(TRUE);
    }
    else
    {
        Dac_Cmd(FALSE);
        Dac_DmaCmd(FALSE);
    }

    return OS_EOK;
}

static os_err_t hc32_dac_write(os_dac_device_t *dac, os_uint32_t channel, os_uint32_t value)
{
    os_uint32_t hc_ch;

    hc_ch = hc32_dac_channel(channel);

    if (hc_ch == ~0)
    {
        LOG_E(DBG_TAG, "dac channel %d cannot find!\n", channel);
        return OS_ERROR;
    }

    Dac_SetChannelData(DacRightAlign, DacBit12, (uint16_t)value);
    Dac_SoftwareTriggerCmd();
    return OS_EOK;
}

static const struct os_dac_ops hc32_dac_ops = {
    .enabled = hc32_dac_enabled,
    .write   = hc32_dac_write,
};

static inline void __os_hw_dac_init(struct hc32_dac_info *dac_info)
{
    stc_dac_cfg_t  dac_initstruct;

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    Gpio_SetAnalogMode(dac_info->dac_port, dac_info->dac_pin);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralDac, TRUE);

    dac_initstruct.boff_t = DacBoffDisable;
    dac_initstruct.ten_t  = DacTenEnable;
    dac_initstruct.sref_t = DacVoltageAvcc;
    dac_initstruct.wave_t = DacWaveDisable;
    dac_initstruct.mamp_t = DacMenp4095;
    dac_initstruct.tsel_t = DacSwTriger;
    dac_initstruct.align  = DacRightAlign;
    dac_initstruct.dhr12  = 0;

    Dac_Init(&dac_initstruct);
}

static int hc32_dac_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct hc32_dac *dac;
    struct hc32_dac_info *dac_info;

    dac = os_calloc(1, sizeof(struct hc32_dac));

    OS_ASSERT(dac);

    dac_info = (struct hc32_dac_info *) dev->info;

    __os_hw_dac_init(dac_info);

    dac->dac.max_value = (1UL << 12) - 1;    /* 12bit */
    dac->dac.ref_low   = 0;                  /* ref 0 - 3.3v */
    dac->dac.ref_hight = 3300;
    dac->dac.ops       = &hc32_dac_ops;

    if (os_dac_register(&dac->dac, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DBG_TAG, "dac device register failed.");
        return OS_ERROR;
    }
    LOG_D(DBG_TAG, "dac device register success.");

    return OS_EOK;
}

OS_DRIVER_INFO hc32_dac_driver = {
    .name   = "DAC_Type",
    .probe  = hc32_dac_probe,
};

OS_DRIVER_DEFINE(hc32_dac_driver, PREV, OS_INIT_SUBLEVEL_LOW);

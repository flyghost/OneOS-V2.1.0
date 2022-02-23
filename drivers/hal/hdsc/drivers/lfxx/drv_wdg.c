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
 * @file        drv_wdg.c
 *
 * @brief       This file implements watchdog driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>

#define DBG_TAG "drv.wdg"
#include <dlog.h>
#include <bus/bus.h>
#include "hc_wdt.h"
#include "drv_wdg.h"

static os_watchdog_t hc32_watchdog;
os_uint8_t timeout_set_flag = 0;

static os_err_t hc32_wdt_init(os_watchdog_t *wdt)
{
    Sysctrl_SetPeripheralGate(SysctrlPeripheralWdt, TRUE);

    return OS_EOK;
}

static os_err_t hc32_wdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    os_uint32_t timeout = 0;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        if (timeout_set_flag == 1)
        {
            Wdt_Feed();
        }
        else
        {
            LOG_E(DBG_TAG, "Wdt keepalive failed.");
            return OS_ENOSYS;
        }
        return OS_EOK;

    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        timeout = (*(float *)arg) * 1000;
        if (timeout == 102)
        {
            Wdt_WriteWdtLoad(WdtT102ms);
        }
        else if (timeout == 205)
        {
            Wdt_WriteWdtLoad(WdtT205ms);
        }
        else if (timeout == 500)
        {
            Wdt_WriteWdtLoad(WdtT500ms);
        }
        else if (timeout == 820)
        {
            Wdt_WriteWdtLoad(WdtT820ms);
        }
        else if (timeout == 1640)
        {
            Wdt_WriteWdtLoad(WdtT1s64);
        }
        else if (timeout == 3280)
        {
            Wdt_WriteWdtLoad(WdtT3s28);
        }
        else if (timeout == 6550)
        {
            Wdt_WriteWdtLoad(WdtT6s55);
        }
        else if (timeout == 13100)
        {
            Wdt_WriteWdtLoad(WdtT13s1);
        }
        else if (timeout == 26200)
        {
            Wdt_WriteWdtLoad(WdtT26s2);
        }
        else if (timeout == 52400)
        {
            Wdt_WriteWdtLoad(WdtT52s4);
        }
        else
        {
            timeout_set_flag = 0;
            LOG_E(DBG_TAG, "The timeout value is not supported.");
            return OS_ENOSYS;
        }
        timeout_set_flag = 1;
        return OS_EOK;

    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;

    case OS_DEVICE_CTRL_WDT_START:
        if (timeout_set_flag == 1)
        {
            Wdt_Start();
            LOG_I(DBG_TAG, "Wdt start.");
        }
        else
        {
            LOG_E(DBG_TAG, "Wdt start failed.");
            return OS_ENOSYS;
        }
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops =
{
    .init     = &hc32_wdt_init,
    .control  = &hc32_wdt_control,
};

static int hc32_wdg_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t ret = OS_EOK;

    hc32_watchdog.ops = &ops;

    ret = os_hw_watchdog_register(&hc32_watchdog, dev->name, OS_NULL);

    if (ret != OS_EOK)
    {
        LOG_E(DBG_TAG, "Os device register failed %d\n", ret);
    }

    return ret;
}

OS_DRIVER_INFO hc32_wdt_driver = {
    .name   = "WDG_HandleTypeDef",
    .probe  = hc32_wdg_probe,
};

OS_DRIVER_DEFINE(hc32_wdt_driver, PREV, OS_INIT_SUBLEVEL_LOW);

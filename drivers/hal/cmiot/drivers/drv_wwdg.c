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
 * @file        drv_wwdg.c
 *
 * @brief       This file implements wwdg driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include "drv_wwdg.h"

#define DBG_TAG "drv.wwdg"
#include <dlog.h>

static os_watchdog_t cm32_watchdog;
os_uint32_t counter_value = 127;

static os_err_t cm32_wwdt_init(os_watchdog_t *wdt)
{
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_WWDG, ENABLE);
    WWDG_SetPrescalerDiv(WWDG_PRESCALER_DIV8);

    return OS_EOK;
}

static os_err_t cm32_wwdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    os_uint32_t window_value = 0;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        WWDG_SetCnt(counter_value);
        return OS_EOK;

    /* set window value */
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        window_value = *(os_uint32_t *)arg;
        if (window_value >= counter_value)
        {
            break;
        }
        else
        {
            WWDG_SetWValue(window_value);
            return OS_EOK;
        }

    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;

    case OS_DEVICE_CTRL_WDT_START:
        WWDG_Enable(counter_value);

        LOG_I(DBG_TAG, "wdt start.");
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops =
{
    .init     = &cm32_wwdt_init,
    .control  = &cm32_wwdt_control,
};

static int cm32_wwdg_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t ret = OS_EOK;

    cm32_watchdog.ops = &ops;

    ret = os_hw_watchdog_register(&cm32_watchdog, dev->name, OS_NULL);

    if (ret != OS_EOK)
    {
        LOG_E(DBG_TAG, "Os device register failed %d\n", ret);
    }

    return ret;
}

OS_DRIVER_INFO cm32_wwdt_driver = {
    .name   = "WWDG_HandleTypeDef",
    .probe  = cm32_wwdg_probe,
};

OS_DRIVER_DEFINE(cm32_wwdt_driver, PREV, OS_INIT_SUBLEVEL_LOW);

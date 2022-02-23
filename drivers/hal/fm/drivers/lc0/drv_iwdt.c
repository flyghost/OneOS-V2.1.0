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
 * @file        drv_iwgt.c
 *
 * @brief       This file implements watchdog driver for fm33
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>

#include "drv_iwdt.h"

static void fm33_iwdg_start(struct fm33_iwdg *iwdg)
{
    FL_IWDT_Init(iwdg->info->instance, &iwdg->info->init);
}

static void fm33_iwdg_refresh(struct fm33_iwdg *iwdg)
{
    FL_IWDT_ReloadCounter(iwdg->info->instance);
}

static os_err_t fm33_iwdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

static os_err_t fm33_iwdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    struct fm33_iwdg *iwdg = (struct fm33_iwdg *)wdt;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        fm33_iwdg_refresh(iwdg);
        return OS_EOK;
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        break;
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;
    case OS_DEVICE_CTRL_WDT_START:
        fm33_iwdg_start(iwdg);
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops = 
{
    .init     = &fm33_iwdt_init,
    .control  = &fm33_iwdt_control,
};

static int fm33_iwdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct fm33_iwdg *iwdg;

    iwdg = os_calloc(1, sizeof(struct fm33_iwdg));

    OS_ASSERT(iwdg);

    iwdg->info = (struct fm33_iwdg_info *)dev->info;
    iwdg->wdg.ops = &ops;

    if (os_hw_watchdog_register(&iwdg->wdg, dev->name, iwdg) != OS_EOK)
    {
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO fm33_iwdt_driver = {
    .name   = "IWDG_Type",
    .probe  = fm33_iwdt_probe,
};

OS_DRIVER_DEFINE(fm33_iwdt_driver, PREV, OS_INIT_SUBLEVEL_LOW);


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
 * @file        drv_wdt.c
 *
 * @brief       This file provides operation functions declaration for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include "os_memory.h"
#include "nrfx_wdt.h"

nrfx_wdt_channel_id m_channel_id;

void wdt_event_handler(void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}

static void nrf5_wdg_refresh(os_watchdog_t *wdg)
{
    os_uint32_t level;

    level = os_irq_lock();
    nrfx_wdt_channel_feed(m_channel_id);
    os_irq_unlock(level);
}

static os_err_t nrf5_wdt_init(os_watchdog_t *wdg)
{
    nrfx_wdt_config_t config = NRFX_WDT_DEAFULT_CONFIG;
    config.reload_value = 10000;//ms
    nrfx_wdt_init(&config, wdt_event_handler);
    nrfx_wdt_channel_alloc(&m_channel_id);

    return OS_EOK;
}

static void nrf5_wdg_start(os_watchdog_t *wdg)
{
    os_uint32_t level;

    level = os_irq_lock();
    nrfx_wdt_channel_feed(m_channel_id);
    nrfx_wdt_enable();
    os_irq_unlock(level);
}

static os_err_t nrf5_wdt_control(os_watchdog_t *wdg, int cmd, void *arg)
{
    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        nrf5_wdg_refresh(wdg);
        return OS_EOK;
        
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_START:
        nrf5_wdg_start(wdg);
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops = 
{
    .init     = &nrf5_wdt_init,
    .control  = &nrf5_wdt_control,
};

static int nrf5_wdg_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_watchdog_t *wdg;

    wdg = os_calloc(1, sizeof(os_watchdog_t));

    OS_ASSERT(wdg);

    wdg->ops = &ops;

    if (os_hw_watchdog_register(wdg, dev->name, OS_NULL) != OS_EOK)
    {
        os_kprintf("wdg device register failed.\r\n");
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO nrf5_wdt_driver = {
    .name   = "WDG_HandleTypeDef",
    .probe  = nrf5_wdg_probe,
};

OS_DRIVER_DEFINE(nrf5_wdt_driver,PREV,OS_INIT_SUBLEVEL_LOW);

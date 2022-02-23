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
 * @file        drv_iwdg.c
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
#include <drv_iwdg.h>

#define DBG_TAG "drv.iwdt"
#include <dlog.h>
#include <bus/bus.h>

static void fm33_iwdg_start(struct fm33_iwdg *iwdg)
{
    FL_IWDT_Init(IWDT, &(iwdg->info->parm));
}

static void fm33_iwdg_refresh(struct fm33_iwdg *iwdg)
{
    FL_IWDT_ReloadCounter(IWDT);
}

static os_err_t fm33_iwdg_set_timeout(struct fm33_iwdg *iwdg, os_uint32_t timeout)
{
    FL_IWDT_InitTypeDef  *config = &(iwdg->info->parm);
    
    switch(timeout)
    {
        case PERIOD_125MS:
            config->overflowPeriod = FL_IWDT_PERIOD_125MS;
            break;
        case PERIOD_250MS:
            config->overflowPeriod = FL_IWDT_PERIOD_250MS;
            break;
        case PERIOD_500MS:
            config->overflowPeriod = FL_IWDT_PERIOD_500MS;
            break;
        case PERIOD_1000MS:
            config->overflowPeriod = FL_IWDT_PERIOD_1000MS;
            break;
        case PERIOD_2000MS:
            config->overflowPeriod = FL_IWDT_PERIOD_2000MS;
            break;
        case PERIOD_4000MS:
            config->overflowPeriod = FL_IWDT_PERIOD_4000MS;
            break;
        case PERIOD_8000MS:
            config->overflowPeriod = FL_IWDT_PERIOD_8000MS;
            break;
        case PERIOD_16000MS:
            config->overflowPeriod = FL_IWDT_PERIOD_16000MS;
            break;
        default:
            LOG_W(DBG_TAG, "%d,supported value:125/250/500/1000/2000/4000/8000/16000.",timeout);
            return OS_ERROR;
    }

    if(FL_IWDT_Init(IWDT, config) != FL_PASS)
    {
        return OS_ERROR;
    }
    else
    {
        iwdg->timeout = timeout;
        return OS_EOK;
    }
}

static os_err_t fm33_iwdg_get_timeout(struct fm33_iwdg *iwdg, os_uint32_t *timeout)
{
    *timeout = iwdg->timeout;
    return OS_EOK;
}

static os_err_t fm33_iwdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

static os_err_t fm33_iwdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    struct fm33_iwdg *iwdg = (struct fm33_iwdg *)wdt;
    os_uint32_t timeout = *(os_uint32_t *)arg;
    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        fm33_iwdg_refresh(iwdg);
        return OS_EOK;
        
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        if(fm33_iwdg_set_timeout(iwdg, timeout) != OS_EOK)
        {
            return OS_ENOSYS;
        }
        else
        {
            LOG_I(DBG_TAG,"iwdt set timeout %dms.", timeout);
            return OS_EOK;
        }
        
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        fm33_iwdg_get_timeout(iwdg, (os_uint32_t *)arg);
        return OS_EOK;
        
    case OS_DEVICE_CTRL_WDT_START:
        fm33_iwdg_start(iwdg);
        LOG_I(DBG_TAG,"iwdt start.");
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
    iwdg->timeout = PERIOD_2000MS;

    if (os_hw_watchdog_register(&iwdg->wdg, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DBG_TAG,"iwdt device register failed.");
        return OS_ERROR;
    }
    LOG_D(DBG_TAG,"iwdt device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO fm33_iwdt_driver = {
    .name   = "WDG_type",
    .probe  = fm33_iwdt_probe,
};

OS_DRIVER_DEFINE(fm33_iwdt_driver,PREV,OS_INIT_SUBLEVEL_LOW);


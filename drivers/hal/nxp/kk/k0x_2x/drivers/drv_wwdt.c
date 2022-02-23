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
 * @file        drv_wwdt.c
 *
 * @brief       This file implements wwdt driver for lpc
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_cfg.h>
#include <drv_wwdt.h>
#include "peripherals.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.wwdt"
#include <drv_log.h>

typedef struct lpc_wwdt
{
    os_watchdog_t wdt;
    struct lpc_wwdt_info *wwdt_info;
}lpc_wwdt_t;

lpc_wwdt_t *lpc_wwdt = OS_NULL;

void WWDT_IRQHANDLER(void)
{
    uint32_t wdtStatus;
    /*wwdt irq can be checked to do some thing after restart cpu, if restart is cased by wwdt! now this function not uesed!*/
    if (lpc_wwdt != OS_NULL)
    {
        wdtStatus = WWDT_GetStatusFlags(lpc_wwdt->wwdt_info->wwdt_base);

        /*this timeoutflag can be set,but in IRQ function cannot check it! it's sure in demo and test!*/
        if (wdtStatus & kWWDT_TimeoutFlag)
        {
            WWDT_Disable(lpc_wwdt->wwdt_info->wwdt_base);
            WWDT_ClearStatusFlags(lpc_wwdt->wwdt_info->wwdt_base, kWWDT_TimeoutFlag);
            WWDT_Enable(lpc_wwdt->wwdt_info->wwdt_base);
        }

        if (wdtStatus & kWWDT_WarningFlag)
        {
            WWDT_ClearStatusFlags(lpc_wwdt->wwdt_info->wwdt_base, kWWDT_WarningFlag);
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

static os_err_t lpc_wwdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

static os_err_t lpc_wwdt_close(os_watchdog_t *wdt)
{
    os_uint32_t level;

    struct lpc_wwdt *lpc_wwdt = (struct lpc_wwdt *)wdt;

    level = os_irq_lock();
    WWDT_Disable(lpc_wwdt->wwdt_info->wwdt_base);
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t lpc_wwdt_open(os_watchdog_t *wdt, os_uint16_t oflag)
{
    os_uint32_t level;
    
    struct lpc_wwdt *lpc_wwdt = (struct lpc_wwdt *)wdt;

    level = os_irq_lock();
#ifdef WWDT_IRQN
    EnableIRQ(WWDT_IRQN);
#endif
    WWDT_Enable(lpc_wwdt->wwdt_info->wwdt_base);
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t lpc_wwdt_refresh(os_watchdog_t *wdt)
{
    os_uint32_t level;
    
    struct lpc_wwdt *lpc_wwdt = (struct lpc_wwdt *)wdt;

    level = os_irq_lock();
    WWDT_Refresh(lpc_wwdt->wwdt_info->wwdt_base);
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t lpc_wwdt_control(os_watchdog_t *wdt, int cmd, void *args)
{
    OS_ASSERT(wdt != NULL);
    
    struct lpc_wwdt *lpc_wwdt = (struct lpc_wwdt *)wdt;

    switch(cmd)
    {
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
    {
        
    }
    break;
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
    {
        *(uint16_t *)args = lpc_wwdt->wwdt_info->wwdt_config->timeoutValue;
    }
    break;
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
    {
        lpc_wwdt_refresh(wdt);
    }
    break;
    case OS_DEVICE_CTRL_WDT_START:
    {
        lpc_wwdt_open(wdt, *(os_uint32_t *)args);
    }
    break;
    case OS_DEVICE_CTRL_WDT_STOP:
    {
        lpc_wwdt_close(wdt);
    }
    break;
    default:
        return OS_EINVAL;
    }

    return OS_EOK;
}

static struct os_watchdog_ops lpc_wwdt_ops =
{
    .init = lpc_wwdt_init,
    .control = lpc_wwdt_control,
};

static int lpc_wwdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    lpc_wwdt_t *lpc_wwdt = os_calloc(1, sizeof(lpc_wwdt_t));

    OS_ASSERT(lpc_wwdt);

    lpc_wwdt->wwdt_info = (struct lpc_wwdt_info *)dev->info;
    lpc_wwdt->wdt.ops = &lpc_wwdt_ops;
    
    if (lpc_wwdt->wwdt_info->wwdt_config->enableWatchdogReset == OS_FALSE)
    {
        LOG_E(DRV_EXT_TAG, "wwdt device only support reset mode");
        return OS_ERROR;
    }
        
    if (os_hw_watchdog_register(&lpc_wwdt->wdt, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "wwdt device register failed.");
        return OS_ERROR;
    }
    return OS_EOK;
}

OS_DRIVER_INFO lpc_wwdt_driver = {
    .name   = "WWDT_Type",
    .probe  = lpc_wwdt_probe,
};

OS_DRIVER_DEFINE(lpc_wwdt_driver, DEVICE, OS_INIT_SUBLEVEL_LOW);



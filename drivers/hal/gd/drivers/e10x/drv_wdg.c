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
 * @brief       This file implements watchdog driver for gd32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>

//#define DRV_EXT_TAG "drv.wwdt"
//#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>
#include <bus/bus.h>
#include <gd32f30x_fwdgt.h>

static void gd32_wdg_start(os_watchdog_t *wdg)
{
    os_uint32_t level;

    level = os_hw_interrupt_disable();
    /* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);
    /* wait till IRC40K is ready */
    while (SUCCESS != rcu_osci_stab_wait(RCU_IRC40K));
    fwdgt_counter_reload();
    fwdgt_enable();
    os_hw_interrupt_enable(level);

}

static void gd32_wdg_refresh(os_watchdog_t *wdg)
{
    os_uint32_t level;

    level = os_hw_interrupt_disable();
    fwdgt_counter_reload();
    os_hw_interrupt_enable(level);
}

static os_err_t gd32_wwdt_init(os_watchdog_t *wdg)
{
    /* confiure FWDGT counter clock: 40KHz(IRC40K) / 256 = 0.15625 KHz */
    fwdgt_config(FWDGT_RLD_RLD, FWDGT_PSC_DIV256);
    fwdgt_enable();
    return OS_EOK;
}

static os_err_t gd32_wwdt_control(os_watchdog_t *wdg, int cmd, void *arg)
{
    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        gd32_wdg_refresh(wdg);
        return OS_EOK;
        
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_START:
        gd32_wdg_start(wdg);
        LOG_EXT_I("wdt start.");
        while (FWDGT_STAT & FWDGT_STAT_RUD);
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops = 
{
    .init     = &gd32_wwdt_init,
    .control  = &gd32_wwdt_control,
};

static int gd32_wdg_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_watchdog_t *wdg;

    wdg = os_calloc(1, sizeof(os_watchdog_t));

    OS_ASSERT(wdg);

    wdg->ops = &ops;

    if (os_hw_watchdog_register(wdg, dev->name, OS_DEVICE_FLAG_DEACTIVATE, OS_NULL) != OS_EOK)
    {
        LOG_EXT_E("wdg device register failed.");
        return OS_ERROR;
    }
    LOG_EXT_E("wwdt device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO gd32_wwdt_driver = {
    .name   = "WDG_HandleTypeDef",
    .probe  = gd32_wdg_probe,
};

OS_DRIVER_DEFINE(gd32_wwdt_driver, "1");


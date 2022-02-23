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
 * @brief       This file implements watchdog driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>

#define DBG_TAG "drv.iwdt"
#include <dlog.h>
#include <bus/bus.h>

struct stm32_iwdg {
    os_watchdog_t wdg;
    
    IWDG_HandleTypeDef *hiwdg;
};

OS_WEAK void MX_IWDG_Init(void){}
OS_WEAK void MX_IWDG1_Init(void){}
OS_WEAK void MX_IWDG2_Init(void){}

static void stm32_iwdg_start(struct stm32_iwdg *iwdg)
{
    MX_IWDG_Init();
    MX_IWDG1_Init();
    MX_IWDG2_Init();
}

static void stm32_iwdg_refresh(struct stm32_iwdg *iwdg)
{
    if (HAL_IWDG_Refresh(iwdg->hiwdg) != HAL_OK)
    {
        LOG_E(DBG_TAG,"watch dog keepalive fail.");
    }
}

static os_err_t stm32_iwdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

static os_err_t stm32_iwdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    struct stm32_iwdg *iwdg = (struct stm32_iwdg *)wdt;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        stm32_iwdg_refresh(iwdg);
        return OS_EOK;
        
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_START:
        stm32_iwdg_start(iwdg);
        LOG_I(DBG_TAG,"iwdt start.");
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops = 
{
    .init     = &stm32_iwdt_init,
    .control  = &stm32_iwdt_control,
};

static int stm32_iwdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_iwdg *iwdg;

    iwdg = os_calloc(1, sizeof(struct stm32_iwdg));

    OS_ASSERT(iwdg);

    iwdg->hiwdg = (IWDG_HandleTypeDef *)dev->info;
    iwdg->wdg.ops = &ops;

    if (os_hw_watchdog_register(&iwdg->wdg, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DBG_TAG,"iwdt device register failed.");
        return OS_ERROR;
    }
    LOG_D(DBG_TAG,"iwdt device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO stm32_iwdt_driver = {
    .name   = "IWDG_HandleTypeDef",
    .probe  = stm32_iwdt_probe,
};

OS_DRIVER_DEFINE(stm32_iwdt_driver,PREV,OS_INIT_SUBLEVEL_LOW);


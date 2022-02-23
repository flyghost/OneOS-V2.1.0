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

#define DBG_TAG "drv.wwdt"
#include <dlog.h>
#include <bus/bus.h>

struct stm32_wwdg {
    os_watchdog_t wdg;
    
    WWDG_HandleTypeDef *hwwdg;
};

OS_WEAK void MX_WWDG_Init(void){}
OS_WEAK void MX_WWDG1_Init(void){}
OS_WEAK void MX_WWDG2_Init(void){}

static void stm32_wwdg_start(struct stm32_wwdg *wwdg)
{
    MX_WWDG_Init();
    MX_WWDG1_Init();
    MX_WWDG2_Init();
}

static void stm32_wwdg_refresh(struct stm32_wwdg *wwdg)
{
    if (HAL_WWDG_Refresh(wwdg->hwwdg) != HAL_OK)
    {
        LOG_E(DBG_TAG,"watch dog keepalive fail.");
    }
}

static os_err_t stm32_wwdt_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

static os_err_t stm32_wwdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    struct stm32_wwdg *wwdg = (struct stm32_wwdg *)wdt;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        stm32_wwdg_refresh(wwdg);
        return OS_EOK;
        
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;
        
    case OS_DEVICE_CTRL_WDT_START:
        stm32_wwdg_start(wwdg);
        LOG_I(DBG_TAG,"wdt start.");
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops = 
{
    .init     = &stm32_wwdt_init,
    .control  = &stm32_wwdt_control,
};

static int stm32_wwdt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_wwdg *wwdg;

    wwdg = os_calloc(1, sizeof(struct stm32_wwdg));

    OS_ASSERT(wwdg);

    wwdg->hwwdg = (WWDG_HandleTypeDef *)dev->info;
    wwdg->wdg.ops = &ops;

    if (os_hw_watchdog_register(&wwdg->wdg, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DBG_TAG,"wwdt device register failed.");
        return OS_ERROR;
    }
    LOG_D(DBG_TAG,"wwdt device register success.");
    return OS_EOK;
}

OS_DRIVER_INFO stm32_wwdt_driver = {
    .name   = "WWDG_HandleTypeDef",
    .probe  = stm32_wwdt_probe,
};

OS_DRIVER_DEFINE(stm32_wwdt_driver,PREV,OS_INIT_SUBLEVEL_LOW);


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
 * @brief       This file implements wdt driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>

#include "fsl_common.h"
#include "fsl_wdog.h"
#include "drv_wdg.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.wdg"
#include <drv_log.h>

struct os_imxrt_wdg
{
    os_watchdog_t           wdg;
    struct nxp_wdog_info   *info;
};

static os_err_t imxrt_wdog_stop(os_watchdog_t *wdg)
{
    struct os_imxrt_wdg *imxrt_wdg = (struct os_imxrt_wdg *)wdg;

    WDOG_Disable(imxrt_wdg->info->base);

    return OS_EOK;
}

static os_err_t imxrt_wdog_start(os_watchdog_t *wdg, os_uint16_t oflag)
{
    struct os_imxrt_wdg *imxrt_wdg = (struct os_imxrt_wdg *)wdg;
    
    WDOG_Enable(imxrt_wdg->info->base);

    return OS_EOK;
}

static os_err_t imxrt_wdog_refresh(os_watchdog_t *wdg)
{
    struct os_imxrt_wdg *imxrt_wdg = (struct os_imxrt_wdg *)wdg;

    WDOG_Refresh(imxrt_wdg->info->base);

    return OS_EOK;
}

static os_err_t imxrt_wdog_control(os_watchdog_t *wdg, int cmd, void *args)
{
    struct os_imxrt_wdg *imxrt_wdg = (struct os_imxrt_wdg *)wdg;
    
    switch(cmd)
    {
    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        
    break;
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        OS_ASSERT(*(uint16_t *)args != 0);
        if ((*(uint16_t *)args) > 128)
        {
            LOG_E(DRV_EXT_TAG,"wwdt timeout just support 128 seconds max!");
            return OS_ERROR;
        }
        
        WDOG_SetTimeoutValue(imxrt_wdg->info->base, (*(uint16_t *)args) * 2 - 1);
        imxrt_wdog_stop(wdg);
    break;
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        imxrt_wdog_refresh(wdg);
    break;
    case OS_DEVICE_CTRL_WDT_START:
        imxrt_wdog_start(wdg, *(os_uint32_t *)args);
    break;
    case OS_DEVICE_CTRL_WDT_STOP:
        imxrt_wdog_stop(wdg);
    break;
    default:
        return OS_EINVAL;
    }

    return OS_EOK;
}

static os_err_t imxrt_wdog_init(os_watchdog_t *wdt)
{
    return OS_EOK;
}

const static struct os_watchdog_ops wdg_ops = 
{
    .init     = imxrt_wdog_init,
    .control  = imxrt_wdog_control,
};

static int imxrt_wdg_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct os_imxrt_wdg *imxrt_wdg = OS_NULL;

    imxrt_wdg = os_calloc(1, sizeof(struct os_imxrt_wdg));

    OS_ASSERT(imxrt_wdg);

    imxrt_wdg->info = (struct nxp_wdog_info *)dev->info;
    imxrt_wdg->wdg.ops = &wdg_ops;

    if (os_hw_watchdog_register(&imxrt_wdg->wdg, dev->name, OS_NULL) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG,"wwdt device register failed!");
        return OS_ERROR;
    }

    return OS_EOK;
}

OS_DRIVER_INFO imxrt_wdg_driver = {
    .name   = "WDOG_Type",
    .probe  = imxrt_wdg_probe,
};

OS_DRIVER_DEFINE(imxrt_wdg_driver,PREV,OS_INIT_SUBLEVEL_LOW);


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
 * @file        wwd_bus.c
 *
 * @brief       This file implements wwd bus driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <os_types.h>
#include <os_errno.h>
#include <os_assert.h>
#include <dlog.h>
#include <device.h>
#include <sdio.h>
#include <wwd_dev.h>
#include <wlan_dev.h>

#include "wwd_wifi.h"
#include "wwd_poll.h"
#include <string.h>

static os_int32_t os_wwd_sdio_probe(struct os_mmcsd_card *card)
{
    wwd_dev->card = card;
    return 0;
}

static os_int32_t os_wwd_sdio_remove(struct os_mmcsd_card *card)
{
    return 0;
}

static struct os_sdio_device_id wwd_sdio_id = 
{
    .func_code      = 0,
    .manufacturer   = 0x02D0,
    .product        = 0xA962,
};

struct os_sdio_driver wwd_sdio_driver =
{
    .name   = "wwd",
    .probe  = os_wwd_sdio_probe,
    .remove = os_wwd_sdio_remove,
    .id     = &wwd_sdio_id,
};

os_err_t os_wwd_wait_bus(void)
{
    while(wwd_dev->card == OS_NULL)
    {
        os_task_msleep(50);
    }
    
    return OS_EOK;
}

os_err_t os_wwd_bus_init(void)
{
    if (wwd_dev->status != WWD_STATE_UNUSED)
    {
        return OS_EOK;
    }
    
    wwd_dev->card = OS_NULL;
    wwd_dev->driver = wwd_sdio_driver;
    wwd_dev->status = WWD_STATE_INIT;
    
    sdio_register_driver(&wwd_dev->driver);
    
    return OS_EOK;
}

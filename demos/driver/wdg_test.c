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
 * @file        wdg_test.c
 *
 * @brief       The test file for wdg.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_errno.h>
#include <watchdog/watchdog.h>
#include <string.h>
#include <stdlib.h>
#include <shell.h>

static int wdg_test(int argc, char *argv[])
{
    os_err_t     ret       = OS_EOK;
    os_uint32_t  timeout   = 10;
    os_uint32_t  period_ms = 100;
    char        *device_name;
    int          i;
    os_device_t *wdg_dev;

    if (argc != 3)
    {
        os_kprintf("usage: wdg_test <dev> <period>\r\n");
        os_kprintf("       wdg_test iwdg1 240 \r\n");
        return -1;
    }
    
    device_name = argv[1];
    period_ms   = strtol(argv[2], NULL, 0);

    wdg_dev = os_device_find(device_name);
    if (!wdg_dev)
    {
        os_kprintf("find %s failed!\r\n", device_name);
        return OS_ERROR;
    }

    ret = os_device_open(wdg_dev);
    if (ret != OS_EOK)
    {
        os_kprintf("initialize %s failed!\r\n", device_name);
        return OS_ERROR;
    }

    os_kprintf("watch dog keep alive for :10s\r\n");

    ret = os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (ret != OS_EOK)
    {
        os_kprintf("%s not support set timeout!\r\n", device_name);
    }

    ret = os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_START, OS_NULL);
    if (ret != OS_EOK)
    {
        os_kprintf("start %s failed!\r\n", device_name);
        return OS_ERROR;
    }
    
    for (i = 0; i < 10000 / period_ms; i++)
    {
        os_task_msleep(period_ms);
        os_kprintf("watch dog keep alive for :%d ms\r\n", i * period_ms);
        os_device_control(wdg_dev, OS_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
    }
    
    for (i = 0; ; i++)
    {
        os_kprintf("watch dog stop feed for :%d ms\r\n", i * 100);
        os_task_msleep(100);
    }
}

SH_CMD_EXPORT(wdg_test, wdg_test, "test watchdog!");

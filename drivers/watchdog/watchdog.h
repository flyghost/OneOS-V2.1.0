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
 * @file        watchdog.h
 *
 * @brief       This file provides macro/struct definition and watchdog function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include <os_assert.h>
#include <os_errno.h>
#include <device.h>

#define OS_DEVICE_CTRL_WDT_GET_TIMEOUT  (1) /* get timeout(in seconds) */
#define OS_DEVICE_CTRL_WDT_SET_TIMEOUT  (2) /* set timeout(in seconds) */
#define OS_DEVICE_CTRL_WDT_GET_TIMELEFT (3) /* get the left time before reboot(in seconds) */
#define OS_DEVICE_CTRL_WDT_KEEPALIVE    (4) /* refresh watchdog */
#define OS_DEVICE_CTRL_WDT_START        (5) /* start watchdog */
#define OS_DEVICE_CTRL_WDT_STOP         (6) /* stop watchdog */

struct os_watchdog_ops;

/**
 ***********************************************************************************************************************
 * @struct      os_watchdog_device
 *
 * @brief       structure of watchdog device
 ***********************************************************************************************************************
 */
struct os_watchdog_device
{
    struct os_device              parent; /* inherit from os_device  */
    const struct os_watchdog_ops *ops;    /* Operation function set */
};
typedef struct os_watchdog_device os_watchdog_t;

struct os_watchdog_ops
{
    os_err_t (*init)(os_watchdog_t *wdt);                        /* init watchdog */
    os_err_t (*control)(os_watchdog_t *wdt, int cmd, void *arg); /* control watchdog */
};

os_err_t os_hw_watchdog_register(os_watchdog_t *wdt, const char *name, void *data);

#endif /* __WATCHDOG_H__ */

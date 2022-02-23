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
 * @file        bus.h
 *
 * @brief       This file provides functions for registering can device.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _DRIVERS_DEVICES_BUS_H_
#define _DRIVERS_DEVICES_BUS_H_

#include <os_task.h>
#include <os_errno.h>
#include <os_assert.h>
#include <device.h>
#include <drv_cfg.h>

struct os_driver_info;
struct os_device_info;

typedef struct os_driver_info
{
    char *name;
    int (*probe)(const struct os_driver_info *drv, const struct os_device_info *dev);
} os_driver_info_t;

typedef struct os_device_info
{
    char *name;
    char *driver;
    const void *info;
} os_device_info_t;

#define OS_DRIVER_INFO static OS_USED OS_SECTION("driver_table") const os_driver_info_t
#define OS_DEVICE_INFO static OS_USED OS_SECTION("device_table") const os_device_info_t

#define OS_DRIVER_DEFINE(_driver_, sequence, subsequence)            \
    static os_err_t __driver_##_driver_##_init(void)    \
    {                                                   \
        return driver_match_devices(&_driver_);         \
    }                                                   \
    OS_##sequence##_INIT(__driver_##_driver_##_init, subsequence)

int driver_match_devices(const os_driver_info_t *driver);
int device_match_drivers(const os_device_info_t *device);

#endif // _DRIVERS_DEVICES_BUS_H_


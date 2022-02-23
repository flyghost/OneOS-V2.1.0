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
 * @file        bus.c
 *
 * @brief       This file provides functions for registering can device.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <bus/bus.h>

int driver_match_devices(const os_driver_info_t *driver)
{
#if defined(__CC_ARM) || defined(__CLANG_ARM)                           /* ARM MDK Compiler */    
    extern const int device_table$$Base;
    extern const int device_table$$Limit;
    
    const os_device_info_t *device_table = (os_device_info_t *)&device_table$$Base;
    int device_num = (os_device_info_t *)&device_table$$Limit - device_table;
    
#elif defined(__ICCARM__) || defined(__ICCRX__) /* for IAR Compiler */
    const os_device_info_t *device_table = (os_device_info_t *)__section_begin("device_table");
    int device_num = (os_device_info_t *)__section_end("device_table") - device_table;
#elif defined(__GNUC__)                         /* for GCC Compiler */
    extern const int __device_table_start;
    extern const int __device_table_end;
    const os_device_info_t *device_table = (os_device_info_t *)&__device_table_start;
    int device_num = (os_device_info_t *)&__device_table_end - device_table;
#endif

    int dev_index;

    for (dev_index = 0; dev_index < device_num; dev_index++)
    {
        if (!strcmp(driver->name, device_table[dev_index].driver))
        {
            driver->probe(driver, &device_table[dev_index]);
        }
    }
    
    return 0;
}

int device_match_drivers(const os_device_info_t *device)
{
#if defined(__CC_ARM) || defined(__CLANG_ARM)                           /* ARM MDK Compiler */
    extern const int driver_table$$Base;
    extern const int driver_table$$Limit;
    
    const os_driver_info_t *driver_table = (os_driver_info_t *)&driver_table$$Base;
    int driver_num = (os_driver_info_t *)&driver_table$$Limit - driver_table;
    
#elif defined(__ICCARM__) || defined(__ICCRX__) /* for IAR Compiler */
    const os_driver_info_t *driver_table = (os_driver_info_t *)__section_begin("driver_table");
    int driver_num = (os_driver_info_t *)__section_end("driver_table") - driver_table;
#elif defined(__GNUC__)                         /* for GCC Compiler */
    extern const int __driver_table_start;
    extern const int __driver_table_end;
    const os_driver_info_t *driver_table = (os_driver_info_t *)&__driver_table_start;
    int driver_num = (os_driver_info_t *)&__driver_table_end - driver_table;
#endif

    int drv_index;

    for (drv_index = 0; drv_index < driver_num; drv_index++)
    {
        if (!strcmp(device->driver, driver_table[drv_index].name))
        {
            driver_table[drv_index].probe(&driver_table[drv_index], device);

            /* a device must only match one driver */
            break;
        }
    }
    
    return 0;
}


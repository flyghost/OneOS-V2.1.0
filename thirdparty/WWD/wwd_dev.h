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
 * @file        wwd_dev.h
 *
 * @brief       This file implements wwd interface.
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WWD_DEV_H__
#define __WWD_DEV_H__

#include <sdio.h>
#include "wlan_dev.h"

#define COUNTRY             WICED_COUNTRY_CHINA
#define AP_SEC              WICED_SECURITY_WPA2_MIXED_PSK
#define AP_CHANNEL          1

typedef enum
{
  WWD_STATE_RUN             = 0,
  WWD_STATE_INIT            = 1,
  WWD_STATE_UNUSED          = 2
}wwd_status_t;

struct os_wwd_device
{
    struct os_wlan_device       wlan_sta_dev;
    struct os_wlan_device       wlan_ap_dev;
    struct os_mmcsd_card       *card;
    struct os_sdio_driver       driver;
    wwd_status_t                status;
};

struct os_wwd_scan_process_info
{
    os_sem_t                    sem;
    struct os_wlan_scan_result *scan_result;
};

extern struct os_wwd_device *wwd_dev;

extern os_err_t os_wwd_bus_init(void);
extern os_err_t os_wwd_wait_bus(void);

struct os_wwd_device *os_wlan_wwd_init(void);


#endif





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
 * @file        drv_ap6181.c
 *
 * @brief       This file implements ap6181 driver.
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

#include "oneos_config.h"
#include "drv_cfg.h"

#include "wlan_dev.h"
#include "wlan_lwip.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wwd_wifi"
#include <drv_log.h>

static int ap6181_early_init(void)
{
    /* WIFI_REG_ON */
    os_pin_mode(BSP_AP6181_REG_ON_PIN, PIN_MODE_OUTPUT);
    os_pin_write(BSP_AP6181_REG_ON_PIN, PIN_LOW);
    os_hw_us_delay(1000);
    os_pin_write(BSP_AP6181_REG_ON_PIN, PIN_HIGH);
    os_hw_us_delay(1000);
    return 0;
}
OS_PREV_INIT(ap6181_early_init, OS_INIT_SUBLEVEL_HIGH);

static int ap6181_init(void)
{
    struct os_device *dev = OS_NULL;
      
#ifdef BSP_USING_AP6181_STA
    dev = os_device_find(OS_WLAN_DEVICE_STA_NAME);
    if (dev == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "wifi_dev %s cannot find!", OS_WLAN_DEVICE_STA_NAME);
        return OS_EOK;
    }
    
    os_pin_mode(BSP_AP6181_IRQ_PIN, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(BSP_AP6181_IRQ_PIN, PIN_IRQ_MODE_RISING_FALLING, os_wlan_irq_handler, dev);
    os_pin_irq_enable(BSP_AP6181_IRQ_PIN, PIN_IRQ_ENABLE);

#ifdef BSP_USING_AP6181_LWIP
    if (os_wlan_lwip_register(dev) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "os_wlan_lwip_register fialed!");
        return OS_EOK;
    }
#endif

    os_wlan_sta_start(dev, OS_WLAN_COUNTRY_CHINA, OS_WLAN_SECURITY_WPA2_MIXED_PSK);
#endif

#ifdef BSP_USING_AP6181_AP
    dev = os_device_find(OS_WLAN_DEVICE_AP_NAME);
    if (dev == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "wifi_dev %s cannot find!", OS_WLAN_DEVICE_AP_NAME);
        return OS_EOK;
    }
    
    os_pin_mode(BSP_AP6181_IRQ_PIN, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(BSP_AP6181_IRQ_PIN, PIN_IRQ_MODE_RISING_FALLING, os_wlan_irq_handler, dev);
    os_pin_irq_enable(BSP_AP6181_IRQ_PIN, PIN_IRQ_ENABLE);
    
#ifdef BSP_USING_AP6181_LWIP
    if (os_wlan_lwip_register(dev) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "os_wlan_lwip_register fialed!");
        return OS_EOK;
    }
#endif

    os_wlan_security_t security;

    switch (BSP_USING_AP6181_AP_SECURITY)
    {
      case 0:
        security = OS_WLAN_SECURITY_OPEN;
        break;
      case 1:
        security = OS_WLAN_SECURITY_WPA2_AES_PSK;
        break;
      default:
        security = OS_WLAN_SECURITY_WPA2_AES_PSK;
        break;
    }
    
    os_wlan_ap_start(dev, BSP_USING_AP6181_AP_SSID, BSP_USING_AP6181_AP_PASSWORD, OS_WLAN_COUNTRY_CHINA, security, BSP_USING_AP6181_AP_CHANNEL);
#endif
    
    return OS_EOK;
}

OS_APP_INIT(ap6181_init, OS_INIT_SUBLEVEL_LOW);

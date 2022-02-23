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
 * @file        esp32_wifi.h
 *
 * @brief       esp32 module link kit wifi api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ESP32_WIFI_H__
#define __ESP32_WIFI_H__


#include "mo_wifi.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ESP32_USING_WIFI_OPS

os_err_t       esp32_wifi_set_mode(mo_object_t *module, mo_wifi_mode_t mode);
mo_wifi_mode_t esp32_wifi_get_mode(mo_object_t *module);
mo_wifi_stat_t esp32_wifi_get_stat(mo_object_t *module);
os_err_t       esp32_wifi_scan_info(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result);
os_err_t       esp32_wifi_connect_ap(mo_object_t *module, const char *ssid, const char *password);
os_err_t       esp32_wifi_disconnect_ap(mo_object_t *module);

#endif /* ESP32_USING_WIFI_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESP32_WIFI_H__ */

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
 * @file        app_cfg.h
 *
 * @brief       Define the micro used to configure stack.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdint.h>
#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#ifndef MYNEWT_VAL_BLE_PUBLIC_DEV_ADDR
#define MYNEWT_VAL_BLE_PUBLIC_DEV_ADDR ((uint8_t[6]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
#endif

#ifndef MYNEWT_VAL_TIMER_5
#define MYNEWT_VAL_TIMER_5 (1)
#endif

#define MYNEWT_VAL_LOG_LEVEL 1

#ifndef MYNEWT_VAL_NEWT_FEATURE_LOGCFG
#define MYNEWT_VAL_NEWT_FEATURE_LOGCFG (1)
#endif

#define MYNEWT_VAL_OS_CPUTIME_TIMER_NUM (5)


#ifndef bssnz_t
/* Just in case bsp.h does not define it, in this case console history will
 * not be preserved across software resets
 */
#define bssnz_t
#endif


#define BLE_MESH_ACCESS_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_ACCESS_LOG_INFO(...) MODLOG_INFO(10, __VA_ARGS__)
#define BLE_MESH_ACCESS_LOG_WARN(...) MODLOG_WARN(10, __VA_ARGS__)
#define BLE_MESH_ACCESS_LOG_ERROR(...) MODLOG_ERROR(10, __VA_ARGS__)
#define BLE_MESH_ACCESS_LOG_CRITICAL(...) MODLOG_CRITICAL(10, __VA_ARGS__)
#define BLE_MESH_ACCESS_LOG_DISABLED(...) MODLOG_DISABLED(10, __VA_ARGS__)

#define BLE_MESH_ADV_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_ADV_LOG_INFO(...) MODLOG_INFO(11, __VA_ARGS__)
#define BLE_MESH_ADV_LOG_WARN(...) MODLOG_WARN(11, __VA_ARGS__)
#define BLE_MESH_ADV_LOG_ERROR(...) MODLOG_ERROR(11, __VA_ARGS__)
#define BLE_MESH_ADV_LOG_CRITICAL(...) MODLOG_CRITICAL(11, __VA_ARGS__)
#define BLE_MESH_ADV_LOG_DISABLED(...) MODLOG_DISABLED(11, __VA_ARGS__)

#define BLE_MESH_BEACON_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_BEACON_LOG_INFO(...) MODLOG_INFO(12, __VA_ARGS__)
#define BLE_MESH_BEACON_LOG_WARN(...) MODLOG_WARN(12, __VA_ARGS__)
#define BLE_MESH_BEACON_LOG_ERROR(...) MODLOG_ERROR(12, __VA_ARGS__)
#define BLE_MESH_BEACON_LOG_CRITICAL(...) MODLOG_CRITICAL(12, __VA_ARGS__)
#define BLE_MESH_BEACON_LOG_DISABLED(...) MODLOG_DISABLED(12, __VA_ARGS__)

#define BLE_MESH_CRYPTO_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_CRYPTO_LOG_INFO(...) MODLOG_INFO(13, __VA_ARGS__)
#define BLE_MESH_CRYPTO_LOG_WARN(...) MODLOG_WARN(13, __VA_ARGS__)
#define BLE_MESH_CRYPTO_LOG_ERROR(...) MODLOG_ERROR(13, __VA_ARGS__)
#define BLE_MESH_CRYPTO_LOG_CRITICAL(...) MODLOG_CRITICAL(13, __VA_ARGS__)
#define BLE_MESH_CRYPTO_LOG_DISABLED(...) MODLOG_DISABLED(13, __VA_ARGS__)

#define BLE_MESH_FRIEND_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_FRIEND_LOG_INFO(...) MODLOG_INFO(14, __VA_ARGS__)
#define BLE_MESH_FRIEND_LOG_WARN(...) MODLOG_WARN(14, __VA_ARGS__)
#define BLE_MESH_FRIEND_LOG_ERROR(...) MODLOG_ERROR(14, __VA_ARGS__)
#define BLE_MESH_FRIEND_LOG_CRITICAL(...) MODLOG_CRITICAL(14, __VA_ARGS__)
#define BLE_MESH_FRIEND_LOG_DISABLED(...) MODLOG_DISABLED(14, __VA_ARGS__)

#define BLE_MESH_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_LOG_INFO(...) MODLOG_INFO(9, __VA_ARGS__)
#define BLE_MESH_LOG_WARN(...) MODLOG_WARN(9, __VA_ARGS__)
#define BLE_MESH_LOG_ERROR(...) MODLOG_ERROR(9, __VA_ARGS__)
#define BLE_MESH_LOG_CRITICAL(...) MODLOG_CRITICAL(9, __VA_ARGS__)
#define BLE_MESH_LOG_DISABLED(...) MODLOG_DISABLED(9, __VA_ARGS__)

#define BLE_MESH_LOW_POWER_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_LOW_POWER_LOG_INFO(...) MODLOG_INFO(15, __VA_ARGS__)
#define BLE_MESH_LOW_POWER_LOG_WARN(...) MODLOG_WARN(15, __VA_ARGS__)
#define BLE_MESH_LOW_POWER_LOG_ERROR(...) MODLOG_ERROR(15, __VA_ARGS__)
#define BLE_MESH_LOW_POWER_LOG_CRITICAL(...) MODLOG_CRITICAL(15, __VA_ARGS__)
#define BLE_MESH_LOW_POWER_LOG_DISABLED(...) MODLOG_DISABLED(15, __VA_ARGS__)

#define BLE_MESH_MODEL_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_MODEL_LOG_INFO(...) MODLOG_INFO(16, __VA_ARGS__)
#define BLE_MESH_MODEL_LOG_WARN(...) MODLOG_WARN(16, __VA_ARGS__)
#define BLE_MESH_MODEL_LOG_ERROR(...) MODLOG_ERROR(16, __VA_ARGS__)
#define BLE_MESH_MODEL_LOG_CRITICAL(...) MODLOG_CRITICAL(16, __VA_ARGS__)
#define BLE_MESH_MODEL_LOG_DISABLED(...) MODLOG_DISABLED(16, __VA_ARGS__)

#define BLE_MESH_NET_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_NET_LOG_INFO(...) MODLOG_INFO(17, __VA_ARGS__)
#define BLE_MESH_NET_LOG_WARN(...) MODLOG_WARN(17, __VA_ARGS__)
#define BLE_MESH_NET_LOG_ERROR(...) MODLOG_ERROR(17, __VA_ARGS__)
#define BLE_MESH_NET_LOG_CRITICAL(...) MODLOG_CRITICAL(17, __VA_ARGS__)
#define BLE_MESH_NET_LOG_DISABLED(...) MODLOG_DISABLED(17, __VA_ARGS__)

#define BLE_MESH_PROV_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_PROV_LOG_INFO(...) MODLOG_INFO(18, __VA_ARGS__)
#define BLE_MESH_PROV_LOG_WARN(...) MODLOG_WARN(18, __VA_ARGS__)
#define BLE_MESH_PROV_LOG_ERROR(...) MODLOG_ERROR(18, __VA_ARGS__)
#define BLE_MESH_PROV_LOG_CRITICAL(...) MODLOG_CRITICAL(18, __VA_ARGS__)
#define BLE_MESH_PROV_LOG_DISABLED(...) MODLOG_DISABLED(18, __VA_ARGS__)

#define BLE_MESH_PROXY_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_PROXY_LOG_INFO(...) MODLOG_INFO(19, __VA_ARGS__)
#define BLE_MESH_PROXY_LOG_WARN(...) MODLOG_WARN(19, __VA_ARGS__)
#define BLE_MESH_PROXY_LOG_ERROR(...) MODLOG_ERROR(19, __VA_ARGS__)
#define BLE_MESH_PROXY_LOG_CRITICAL(...) MODLOG_CRITICAL(19, __VA_ARGS__)
#define BLE_MESH_PROXY_LOG_DISABLED(...) MODLOG_DISABLED(19, __VA_ARGS__)

#define BLE_MESH_SETTINGS_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_SETTINGS_LOG_INFO(...) MODLOG_INFO(20, __VA_ARGS__)
#define BLE_MESH_SETTINGS_LOG_WARN(...) MODLOG_WARN(20, __VA_ARGS__)
#define BLE_MESH_SETTINGS_LOG_ERROR(...) MODLOG_ERROR(20, __VA_ARGS__)
#define BLE_MESH_SETTINGS_LOG_CRITICAL(...) MODLOG_CRITICAL(20, __VA_ARGS__)
#define BLE_MESH_SETTINGS_LOG_DISABLED(...) MODLOG_DISABLED(20, __VA_ARGS__)

#define BLE_MESH_TRANS_LOG_DEBUG(...) IGNORE(__VA_ARGS__)
#define BLE_MESH_TRANS_LOG_INFO(...) MODLOG_INFO(21, __VA_ARGS__)
#define BLE_MESH_TRANS_LOG_WARN(...) MODLOG_WARN(21, __VA_ARGS__)
#define BLE_MESH_TRANS_LOG_ERROR(...) MODLOG_ERROR(21, __VA_ARGS__)
#define BLE_MESH_TRANS_LOG_CRITICAL(...) MODLOG_CRITICAL(21, __VA_ARGS__)
#define BLE_MESH_TRANS_LOG_DISABLED(...) MODLOG_DISABLED(21, __VA_ARGS__)


#ifndef MYNEWT_VAL_BLE_MESH_DEV_UUID
#define MYNEWT_VAL_BLE_MESH_DEV_UUID ((uint8_t[16]){0x11, 0x22, 0})
#endif

#endif

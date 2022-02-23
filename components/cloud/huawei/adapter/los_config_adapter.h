/*
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
 * @file        los_config_adapter.c
 *
 * @brief       huawei cloud sdk file "los_config.c" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __LOS_CONFIG_ADAPTER_H__
#define __LOS_CONFIG_ADAPTER_H__
#include <oneos_config.h>
#include <os_types.h>

#if (defined USING_HUAWEI_CLOUD_CONNECT) && (defined HUAWEI_CLOUD_PORTOCOL_LwM2M)

#define LWM2M_CLIENT_MODE
#define LWM2M_BOOTSTRAP_SERVER_MODE
#define LWM2M_LITTLE_ENDIAN

#elif (defined USING_HUAWEI_CLOUD_CONNECT) && (defined HUAWEI_CLOUD_PORTOCOL_MQTT)

#define LOSCFG_BASE_IPC_MUX YES

#if (defined HUAWEI_CA)
#define MQTT_DEMO_USE_PSK  0
#define MQTT_DEMO_USE_CERT 1
#elif (defined HUAWEI_PSK)
#define MQTT_DEMO_USE_PSK  1
#define MQTT_DEMO_USE_CERT 0
#endif

#endif

#if defined(HUAWEI_CLOUD_WITH_TLS) && !(defined WITH_DTLS)
#define WITH_DTLS
#endif

typedef os_uint8_t  UINT8;
typedef os_uint16_t UINT16;
typedef os_uint32_t UINT32;
typedef os_uint64_t UINT64;

typedef os_int8_t  INT8;
typedef os_int16_t INT16;
typedef os_int32_t INT32;
typedef os_int64_t INT64;

typedef os_uint8_t  uint8_t;
typedef os_uint16_t uint16_t;
// typedef os_uint32_t uint32_t;
typedef os_uint64_t uint64_t;

typedef os_int8_t  int8_t;
typedef os_int16_t int16_t;
// typedef os_int32_t int32_t;
typedef os_int64_t int64_t;

#endif /* __LOS_CONFIG_ADAPTER_H__ */

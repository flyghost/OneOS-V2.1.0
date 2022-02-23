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
 * @file        mo_type.h
 *
 * @brief       module link kit object type definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_TYPE_H__
#define __MO_TYPE_H__

#include <oneos_config.h>

#ifdef NET_USING_MOLINK

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum mo_ops_type
{
    MODULE_OPS_NULL = -1,
#ifdef MOLINK_USING_GENERAL_OPS
    MODULE_OPS_GENERAL,
#endif
#ifdef MOLINK_USING_NETSERV_OPS
    MODULE_OPS_NETSERV,
#endif
#ifdef MOLINK_USING_PING_OPS
    MODULE_OPS_PING,
#endif
#ifdef MOLINK_USING_IFCONFIG_OPS
    MODULE_OPS_IFCONFIG,
#endif
#ifdef MOLINK_USING_NETCONN_OPS
    MODULE_OPS_NETCONN,
#endif
#ifdef MOLINK_USING_MQTTC_OPS
    MODULE_OPS_MQTTC,
#endif
#ifdef MOLINK_USING_ONENET_NB_OPS
    MODULE_OPS_ONENET_NB,
#endif
#ifdef MOLINK_USING_WIFI_OPS
    MODULE_OPS_WIFI,
#endif
#ifdef MOLINK_USING_PPP_OPS
    MODULE_OPS_PPP,
#endif
#ifdef MOLINK_USING_CTM2M_OPS
    MODULE_OPS_CTM2M,
#endif

    MODULE_OPS_MAX,
} mo_ops_type_t;

typedef enum mo_type
{
    MODULE_TYPE_NULL = -1,

/********* Type of NB-IoT modules *********/
#ifdef MOLINK_USING_M5310A
    MODULE_TYPE_M5310A,
#endif
#ifdef MOLINK_USING_M5311
    MODULE_TYPE_M5311,
#endif
#ifdef MOLINK_USING_BC95
    MODULE_TYPE_BC95,
#endif
#ifdef MOLINK_USING_BC28
    MODULE_TYPE_BC28,
#endif
#ifdef MOLINK_USING_BC26
    MODULE_TYPE_BC26,
#endif
#ifdef MOLINK_USING_BC20
    MODULE_TYPE_BC20,
#endif
#ifdef MOLINK_USING_SIM7020
    MODULE_TYPE_SIM7020,
#endif
#ifdef MOLINK_USING_SIM7070X
    MODULE_TYPE_SIM7070X,
#endif
#ifdef MOLINK_USING_N21
    MODULE_TYPE_N21,
#endif
#ifdef MOLINK_USING_MB26
    MODULE_TYPE_MB26,
#endif
#ifdef MOLINK_USING_GM120
    MODULE_TYPE_GM120,
#endif
#ifdef MOLINK_USING_E7025
    MODULE_TYPE_E7025,
#endif
#ifdef MOLINK_USING_ME3616
    MODULE_TYPE_ME3616,
#endif

/********* Type of 4G cat1 modules ********/
#ifdef MOLINK_USING_EC200X_600S
    MODULE_TYPE_EC200X_600S,
#endif
#ifdef MOLINK_USING_ML302
    MODULE_TYPE_ML302,
#endif
#ifdef MOLINK_USING_GM190
    MODULE_TYPE_GM190,
#endif
#ifdef MOLINK_USING_A7600X
    MODULE_TYPE_A7600X,
#endif
#ifdef MOLINK_USING_A7670X
    MODULE_TYPE_A7670X,
#endif
#ifdef MOLINK_USING_L610
    MODULE_TYPE_L610,
#endif
#ifdef MOLINK_USING_AIR723UG
    MODULE_TYPE_AIR723UG,
#endif
#ifdef MOLINK_USING_CLM920RV3
    MODULE_TYPE_CLM920RV3,
#endif
#ifdef MOLINK_USING_N58
    MODULE_TYPE_N58,
#endif
#ifdef MOLINK_USING_AIR720UH
    MODULE_TYPE_AIR720UH,
#endif

/********* Type of 4G cat4 modules ********/
#ifdef MOLINK_USING_GM510
    MODULE_TYPE_GM510,
#endif
#ifdef MOLINK_USING_EC20
    MODULE_TYPE_EC20,
#endif
#ifdef MOLINK_USING_SIM7600CE
    MODULE_TYPE_SIM7600CE,
#endif
#ifdef MOLINK_USING_ME3630_W
    MODULE_TYPE_ME3630_W,
#endif

/********* Type of 5G modules *************/
#ifdef MOLINK_USING_RG500Q
    MODULE_TYPE_RG500Q,
#endif

/********* Type of wifi modules ***********/
#ifdef MOLINK_USING_ESP8266
    MODULE_TYPE_ESP8266,
#endif
#ifdef MOLINK_USING_ESP32
    MODULE_TYPE_ESP32,
#endif

    MODULE_TYPE_MAX,
} mo_type_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NET_USING_MOLINK */

#endif /* __MO_TYPE_H__ */

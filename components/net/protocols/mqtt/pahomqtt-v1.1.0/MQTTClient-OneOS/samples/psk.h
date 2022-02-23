/**
***********************************************************************************************************************
* Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
* @file        psk.h
*
* @brief       psk header file.
*
* @revision
* Date         Author          Notes
* 2021-04-28   XieLi           First Version
***********************************************************************************************************************
*/

#ifndef __PSK_H_
#define __PSK_H_

#include <oneos_config.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifdef MQTT_USING_TLS
extern const char g_certificate[];

#ifdef MQTT_USING_TLS_ONETLS
extern const uint8_t g_psk_identity[];
extern uint16_t g_psk_identity_len;
extern const uint8_t g_psk_key[];
extern uint16_t g_psk_key_len;
#endif
#else
extern const char *g_certificate;
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __PSK_H_ */

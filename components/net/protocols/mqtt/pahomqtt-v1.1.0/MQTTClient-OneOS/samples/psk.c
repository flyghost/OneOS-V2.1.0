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
 * @file        psk.c
 *
 * @brief       psk functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <oneos_config.h>

#ifdef MQTT_USING_TLS_ONETLS
// identity: test
const uint8_t g_psk_identity[] = {0x74, 0x65, 0x73, 0x74};
uint16_t g_psk_identity_len = 4;
// key: 0xaabbccdd
const uint8_t g_psk_key[] = {0xaa, 0xbb, 0xcc, 0xdd};
uint16_t g_psk_key_len = 4;
#endif

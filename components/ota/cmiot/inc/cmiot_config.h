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
 * @file        cmiot_config.h
 *
 * @brief       The config header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_CONFIG_H__
#define __CMIOT_CONFIG_H__

#include "cmiot_type.h"
#include <oneos_config.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CMIOT_ONEOS_APP    "app"
#define CMIOT_ONEOS_UPDATE "download"

#define CMIOT_HW_VERSION "HW01"
#define CMIOT_SW_VERSION "SW01"

#define CMIOT_SEGMENT_SIZE_16_INDEX  0
#define CMIOT_SEGMENT_SIZE_32_INDEX  1
#define CMIOT_SEGMENT_SIZE_64_INDEX  2
#define CMIOT_SEGMENT_SIZE_128_INDEX 3
#define CMIOT_SEGMENT_SIZE_256_INDEX 4
#define CMIOT_SEGMENT_SIZE_512_INDEX 5

/* CMIOT_DEFAULT_NETWORK_PROTOCOL can be set to 1 or 2 */
#define CMIOT_PROTOCOL_COAP 1
#define CMIOT_PROTOCOL_HTTP 2

/* Try count */
#define CMIOT_DEFAULT_TRY_TIME  200
#define CMIOT_DEFAULT_TRY_COUNT 6
#define CMIOT_DEFAULT_UTC_TIME  133

/* get data size by index */
#define CMIOT_DATA_SEQ_MAX_LEN (1 << (CMIOT_DEFAULT_SEGMENT_SIZE_INDEX + 4))
#define CMIOT_HTTP_HEAD_LEN    640
#define CMIOT_HTTP_MAX_LEN     (CMIOT_DATA_SEQ_MAX_LEN + CMIOT_HTTP_HEAD_LEN)
#define CMIOT_COAP_MAX         (CMIOT_DATA_SEQ_MAX_LEN + 32)
#define CMIOT_COAP_MIN         256

#if (CMIOT_COAP_MAX < CMIOT_COAP_MIN)
#define CMIOT_COAP_MAX_LEN CMIOT_COAP_MIN
#else
#define CMIOT_COAP_MAX_LEN CMIOT_COAP_MAX
#endif

#ifndef CMIOT_DEFAULT_NETWORK_PROTOCOL
#define CMIOT_DEFAULT_NETWORK_PROTOCOL CMIOT_PROTOCOL_COAP
#endif

#if (CMIOT_DEFAULT_NETWORK_PROTOCOL == CMIOT_PROTOCOL_HTTP)
#define CMIOT_RECIEVE_BUF_MAXLEN CMIOT_HTTP_MAX_LEN
#define CMIOT_SEND_BUF_MAXLEN    (300 + CMIOT_DATA_SEQ_MAX_LEN)
#else
#define CMIOT_RECIEVE_BUF_MAXLEN (CMIOT_COAP_MAX_LEN * 2 + 100)
#define CMIOT_SEND_BUF_MAXLEN    (CMIOT_COAP_MAX_LEN)
#endif

cmiot_char * cmiot_get_sdk_version(void);
cmiot_uint32 cmiot_get_down_start_time(void);
cmiot_uint32 cmiot_get_down_end_time(void);
cmiot_uint32 cmiot_get_data_max_len(void);

cmiot_uint8 cmiot_get_default_protocol(void);
cmiot_char *cmiot_get_manufacturer(void);
cmiot_char *cmiot_get_model_number(void);
cmiot_char *cmiot_get_product_id(void);
cmiot_char *cmiot_get_product_sec(void);
cmiot_char *cmiot_get_device_type(void);
cmiot_char *cmiot_get_platform(void);
cmiot_char *cmiot_get_apk_version(void);
cmiot_char *cmiot_get_firmware_version(void);
cmiot_uint8 cmiot_get_default_segment_size(void);

#ifdef __cplusplus
}
#endif

#endif

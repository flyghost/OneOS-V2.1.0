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
 * @file        onetls_alert.h
 *
 * @brief       onetls_alert header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_ALERT_H__
#define __ONETLS_ALERT_H__
#include "onetls.h"

// level
#define ONETLS_ALERT_LEVEL_WARNING (1)
#define ONETLS_ALERT_LEVEL_FATAL   (2)

// description
#define ONETLS_ALERT_DESCRIPTION_CLOSE_NOTIFY                     (0)
#define ONETLS_ALERT_DESCRIPTION_UNEXPECTED_MESSAGE               (10)
#define ONETLS_ALERT_DESCRIPTION_BAD_RECORD_MAC                   (20)
#define ONETLS_ALERT_DESCRIPTION_DECRYPTION_FAILED_RESERVED       (21)
#define ONETLS_ALERT_DESCRIPTION_RECORD_OVERFLOW                  (22)
#define ONETLS_ALERT_DESCRIPTION_DECOMPRESSION_FAILURE            (30)
#define ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE                (40)
#define ONETLS_ALERT_DESCRIPTION_NO_CERTIFICATE_RESERVED          (41)
#define ONETLS_ALERT_DESCRIPTION_BAD_CERTIFICATE                  (42)
#define ONETLS_ALERT_DESCRIPTION_UNSUPPORTED_CERTIFICATE          (43)
#define ONETLS_ALERT_DESCRIPTION_CERTIFICATE_REVOKED              (44)
#define ONETLS_ALERT_DESCRIPTION_CERTIFICATE_EXPIRED              (45)
#define ONETLS_ALERT_DESCRIPTION_CERTIFICATE_UNKNOWN              (46)
#define ONETLS_ALERT_DESCRIPTION_ILLEGAL_PARAMETER                (47)
#define ONETLS_ALERT_DESCRIPTION_UNKNOWN_CA                       (48)
#define ONETLS_ALERT_DESCRIPTION_ACCESS_DENIED                    (49)
#define ONETLS_ALERT_DESCRIPTION_DECODE_ERROR                     (50)
#define ONETLS_ALERT_DESCRIPTION_DECRYPT_ERROR                    (51)
#define ONETLS_ALERT_DESCRIPTION_EXPORT_RESTRICTION_RESERVED      (60)
#define ONETLS_ALERT_DESCRIPTION_PROTOCOL_VERSION                 (70)
#define ONETLS_ALERT_DESCRIPTION_INSUFFICIENT_SECURITY            (71)
#define ONETLS_ALERT_DESCRIPTION_INTERNAL_ERROR                   (80)
#define ONETLS_ALERT_DESCRIPTION_USER_CANCELED                    (90)
#define ONETLS_ALERT_DESCRIPTION_NO_RENEGOTIATION                 (100)
#define ONETLS_ALERT_DESCRIPTION_MISSING_EXTENSION                (109)
#define ONETLS_ALERT_DESCRIPTION_UNSUPPORTED_EXTENSION            (110)
#define ONETLS_ALERT_DESCRIPTION_CERTIFICATE_UNOBTAINABLE         (111)
#define ONETLS_ALERT_DESCRIPTION_UNRECOGNIZED_NAME                (112)
#define ONETLS_ALERT_DESCRIPTION_BAD_CERTIFICATE_STATUS_RESPONSE  (113)
#define ONETLS_ALERT_DESCRIPTION_BAD_CERTIFICATE_HASH_VALUE       (114)
#define ONETLS_ALERT_DESCRIPTION_UNKNOWN_PSK_IDENTITY             (115)
#define ONETLS_ALERT_DESCRIPTION_CERTIFICATE_REQUIRED             (116)
#define ONETLS_ALERT_DESCRIPTION_NO_APPLICATION_PROTOCOL          (120)

#define ONETLS_SHUTDOWN_MASK 0x8000
#define ONETLS_SHUTDOWN_CODE(code) (ONETLS_SHUTDOWN_MASK | code)
#define ONETLS_CLOSED ONETLS_SHUTDOWN_CODE(ONETLS_ALERT_DESCRIPTION_CLOSE_NOTIFY)

// 收发告警
uint32_t onetls_send_alert(onetls_ctx *ctx, uint8_t level, uint8_t desc);
uint32_t onetls_recv_alert(onetls_ctx *ctx, uint8_t level, uint8_t desc);

void onetls_notify_close(onetls_ctx *ctx);

#endif

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
 * @file        hmac_sha1.h
 *
 * @brief       This file supply a certificate for TLS network conncet of OneNET-MQTTS.
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#ifndef CA_CERTIFICATE_H
#define CA_CERTIFICATE_H

#include <oneos_config.h>

#ifdef ONENET_MQTTS_USING_TLS
extern const char g_onenet_mqtts_ca_certificate[];
#else
extern const char *g_onenet_mqtts_ca_certificate;
#endif

#endif

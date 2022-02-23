/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *use this file except in compliance with the License. You may obtain a copy of
 *the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file        cms_con_udp_core.h
 *
 * @brief       define the mqtt protocol interface of CMS connection component
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-09   OneOS Team      First version.
 ***********************************************************************************************************************/
#ifndef __CMS_CON_UDP_CORE_H__
#define __CMS_CON_UDP_CORE_H__
#include "cms_con_def.h"

#if defined(__cplusplus)
extern "C" {
#endif

void *cms_udp_init(uint16_t scode, uint32_t recvbuf_size);

void cms_udp_deinit(void *handle);

int cms_udp_send(void *handle, const uint8_t *buf, size_t len);

int cms_udp_recv(void *handle, uint8_t* buf, size_t max_length, int timeout_ms);

int cms_udp_get_state(void *handle);

#if defined(__cplusplus)
}
#endif

#endif

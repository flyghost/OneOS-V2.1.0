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
 * @file        mo_ping.h
 *
 * @brief       module link kit ping api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __MO_PING_H__
#define __MO_PING_H__

#include "mo_type.h"
#include "mo_object.h"

#include <mo_ipaddr.h>

#ifdef MOLINK_USING_PING_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ping_resp
{
    ip_addr_t   ip_addr;   /* response IP address */
    os_uint16_t data_len;  /* response data length */
    os_uint16_t ttl;       /* time to live */
    os_uint32_t time;      /* response time, unit ms */
    void       *user_data; /* user-specific data */
} ping_resp_t;

typedef struct mo_ping_ops
{
    os_err_t (*ping)(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout_ms, struct ping_resp *resp);
} mo_ping_ops_t;

os_err_t mo_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout_ms, struct ping_resp *resp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_PING_OPS */

#endif /* __MO_PING_H__ */

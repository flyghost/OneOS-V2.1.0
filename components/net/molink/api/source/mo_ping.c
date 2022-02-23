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
 * @file        mo_wifi.c
 *
 * @brief       module link kit ping api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_ping.h"

#define MO_LOG_TAG "molink.ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_PING_OPS

static mo_ping_ops_t *get_ping_ops(mo_object_t *self)
{
    mo_ping_ops_t *ops = (mo_ping_ops_t *)self->ops_table[MODULE_OPS_PING];

    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support ping operates", self->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to ping remote host
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       host            The remote host name
 * @param[in]       len             The ping bytes
 * @param[in]       timeout         The ping timeout, in milliseconds
 * @param[out]      resp            The buffer to store ping response infomation
 *
 * @return          On success, return OS_EOK; on error, return a error code.
 * @retval          OS_EOK          Ping successfully
 * @retval          OS_ETIMEOUT     Ping timeout
 * @retval          OS_ERROR        Ping error
 ***********************************************************************************************************************
 */
os_err_t mo_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout_ms, struct ping_resp *resp)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != host);
    OS_ASSERT(OS_NULL != resp);

    mo_ping_ops_t *ops = get_ping_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->ping)
    {
        ERROR("Module %s does not support ping operate", self->name);
        return OS_ERROR;
    }

    return ops->ping(self, host, len, timeout_ms, resp);
}

#endif /* MOLINK_USING_PING_OPS */

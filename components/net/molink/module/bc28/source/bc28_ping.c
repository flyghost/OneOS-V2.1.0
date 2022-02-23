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
 * @file        bc28_ping.c
 *
 * @brief       bc28 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc28_ping.h"
#include "bc28_netconn.h"
#include <string.h>

#ifdef BC28_USING_PING_OPS

extern os_err_t bc28_ping_handler(mo_object_t *module, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp);

os_err_t bc28_ping(mo_object_t *module, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    return bc28_ping_handler(module, host, len, timeout, resp);
}

#endif /* BC28_USING_PING_OPS */

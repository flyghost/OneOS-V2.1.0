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
 * @file        tool.h
 *
 * @brief       The modules network debug functions header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __TOOL_H__
#define __TOOL_H__

#include <os_task.h>
#include <os_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the maximum of all used hardware address lengths */
#ifndef MODULE_HWADDR_MAX_LEN
#define MODULE_HWADDR_MAX_LEN 8U
#endif

#ifdef NET_USING_MOLINK

#ifdef MOLINK_TOOLS_USING_IFCONFIG
void module_show_info(void);
#endif /* MOLINK_TOOLS_USING_IFCONFIG */

#ifdef MOLINK_TOOLS_USING_PING
void module_ping(char *target_name, os_uint32_t times, os_uint16_t size, os_uint32_t timeout);
#endif /* MOLINK_TOOLS_USING_PING */

#if defined(MOLINK_TOOLS_USING_SOCKETSTAT) && defined(MOLINK_USING_NETCONN_OPS)
void module_show_socket_stat(void);
#endif /* MOLINK_TOOLS_USING_SOCKETSTAT */

#endif /* NET_USING_MOLINK */

#ifdef __cplusplus
}
#endif

#endif /* __TOOL_H__ */

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
 * @file        e7025_netconn.h
 *
 * @brief       E7025 module link kit netconnect api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __E7025_NETCONN_H__
#define __E7025_NETCONN_H__

#include "mo_netconn.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef E7025_USING_NETCONN_OPS

os_err_t      e7025_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info);
mo_netconn_t *e7025_netconn_create(mo_object_t *module, mo_netconn_type_t type);
os_err_t      e7025_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn);
os_err_t      e7025_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port);
os_size_t     e7025_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size);
os_err_t      e7025_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr);

#endif /* E7025_USING_NETCONN_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __E7025_NETCONN_H__ */

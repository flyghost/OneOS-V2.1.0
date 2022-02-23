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
 * @file        mo_ifconfig.h
 *
 * @brief       module link kit ifconfig api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __MO_IFCONFIG_H__
#define __MO_IFCONFIG_H__

#include "mo_type.h"
#include "mo_object.h"
#include "mo_ipaddr.h"

#ifdef MOLINK_USING_IFCONFIG_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 ***********************************************************************************************************************
 * @struct      dns_server_t
 *
 * @brief       struct that holds molink module dns_server information
 ***********************************************************************************************************************
 */
typedef struct dns_server
{
    char primary_dns[IPADDR_MAX_STR_LEN + 1];
    char secondary_dns[IPADDR_MAX_STR_LEN + 1];
} dns_server_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_ifconfig_ops_t
 *
 * @brief       molink module ifconfig ops table
 ***********************************************************************************************************************
 */
typedef struct mo_ifconfig_ops
{
    os_err_t (*ifconfig)(mo_object_t *self);
    os_err_t (*get_ipaddr)(mo_object_t *self, char ip[]);
    os_err_t (*set_dnsserver)(mo_object_t *self, dns_server_t dns);
    os_err_t (*get_dnsserver)(mo_object_t *self, dns_server_t *dns);
} mo_ifconfig_ops_t;

os_err_t mo_ifconfig(mo_object_t *self);
os_err_t mo_get_ipaddr(mo_object_t *self, char ip[]);
os_err_t mo_set_dnsserver(mo_object_t *self, dns_server_t dns);
os_err_t mo_get_dnsserver(mo_object_t *self, dns_server_t *dns);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_IFCONFIG_OPS */

#endif /* __MO_IFCONFIG_H__ */

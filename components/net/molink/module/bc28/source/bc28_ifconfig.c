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
 * @file        bc28_ifconfig.c
 *
 * @brief       bc28 module link kit ifconfig api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <mo_ipaddr.h>
#include "bc28_general.h"
#include "bc28_netserv.h"
#include "bc28_ifconfig.h"

#include <stdlib.h>
#include <string.h>

#ifdef BC28_USING_IFCONFIG_OPS

#define MO_LOG_TAG "bc28.ifconfig"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define BC28_IFCONFIG_INVALID_DEF (-1)

os_err_t bc28_ifconfig(mo_object_t *module)
{
    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    if (bc28_get_ipaddr(module, ipaddr) != OS_EOK)
    {
        memset(ipaddr, 0, sizeof(ipaddr));
    }

    char imei[MO_IMEI_LEN + 1] = {0};
    if (bc28_get_imei(module, imei, sizeof(imei)) != OS_EOK)
    {
        memset(imei, 0, sizeof(imei));
    }

    char iccid[MO_ICCID_LEN + 1] = {0};
    if (bc28_get_iccid(module, iccid, sizeof(iccid)) != OS_EOK)
    {
        memset(iccid, 0, sizeof(iccid));
    }

    os_uint8_t rssi = 0;
    os_uint8_t ber  = 0;

    if (bc28_get_csq(module, &rssi, &ber) != OS_EOK)
    {
        rssi = 0;
        ber  = 0;
    }

    os_kprintf("\r\nLIST AT MODULE INFORMATION\r\n");
    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }

    os_kprintf("\r\n");
    os_kprintf("Module Name    : %s\r\n", module->name);
    os_kprintf("IMEI   Number  : %s\r\n", imei);
    os_kprintf("ICCID  Number  : %s\r\n", iccid);
    os_kprintf("Signal Quality : rssi(%d), ber(%d)\r\n", rssi, ber);
    os_kprintf("IPv4   Address : %s\r\n", strlen(ipaddr) ? ipaddr : "0.0.0.0");

    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\r\n");

    return OS_EOK;
}

os_err_t bc28_set_dnsserver(mo_object_t *module, dns_server_t dns)
{
    /* BC28 must set usable dns server befor QDNS(gethostbyname), 
       otherwise both func will not be reachable before reboot! */
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if (OS_NULL == parser)
    {
        ERROR("BC28 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    /* it appears that module will reset dns if secondary dns is "0" */
    if (0 == strlen(dns.primary_dns) || 0 == strcmp(dns.secondary_dns, "0"))
    {
        ERROR("BC28 %s: with invalid param.", __FUNCTION__);
        return OS_ERROR;
    }

    if (0 == strlen(dns.secondary_dns))
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=\"%s\"", dns.primary_dns);
    }
    else
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=\"%s\",\"%s\"", dns.primary_dns, dns.secondary_dns);
    }
    
    return result;
}

os_err_t bc28_get_dnsserver(mo_object_t *module, dns_server_t *dns)
{
    at_parser_t *parser         = &module->parser;
    os_err_t     result         = OS_EOK;

    char primary_dns[IPADDR_MAX_STR_LEN + 1]   = {0};
    char secondary_dns[IPADDR_MAX_STR_LEN + 1] = {0};

    if (OS_NULL == parser)
    {
        ERROR("BC28 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    if (OS_NULL == dns)
    {
        ERROR("BC28 %s: dns is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .timeout   = 16 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &at_resp, "AT+QIDNSCFG?");

    /* return eg. PrimaryDns: 218.4.4.4\r\nSecondaryDns: 208.67.222.222 at BC28JAR01A04_ONT */
    if (at_resp_get_data_by_kw(&at_resp, "PrimaryDns:", "PrimaryDns: %[^\r]", primary_dns) <= 0)
    {
        ERROR("BC28 %s: get primary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "SecondaryDns:", "SecondaryDns: %[^\r]", secondary_dns) <= 0)
    {
        ERROR("BC28 %s: get secondary dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }

    strcpy(dns->primary_dns, primary_dns);
    strcpy(dns->secondary_dns, secondary_dns);

    DEBUG("BC28 %s: primary_dns[%s],secondary_dns[%s]", __FUNCTION__, dns->primary_dns, dns->secondary_dns);

__exit:
    return result;
}

os_err_t bc28_get_ipaddr(mo_object_t *module, char ip[])
{
    at_parser_t *parser = &module->parser;
    os_int8_t    ucid   = BC28_IFCONFIG_INVALID_DEF;
    os_int8_t    len    = BC28_IFCONFIG_INVALID_DEF;

    char ipaddr[IPADDR_MAX_STR_LEN + 1]       = {0};
    char resp_buff[2 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR");
    if (result != OS_EOK)
    {
        ERROR("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%hhd,%[^\r]", &ucid, ipaddr) <= 0)
    {
        ERROR("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        ERROR("IP address len[%d] invalid.", len);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        strcpy(ip, ipaddr);
        DEBUG("IP address: %s", ip);
    }

__exit:
    
    return result;
}

#endif /* BC28_USING_IFCONFIG_OPS */

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
 * @file        clm920rv3_ifconfig.c
 *
 * @brief       clm920rv3 module link kit ifconfig api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <mo_ipaddr.h>
#include "clm920rv3.h"

#include <stdlib.h>
#include <string.h>

#ifdef CLM920RV3_USING_IFCONFIG_OPS

#define MO_LOG_TAG "clm920rv3.ifconfig"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

extern os_err_t clm920rv3_pdp_act(mo_object_t *module);

os_err_t clm920rv3_ifconfig(mo_object_t *module)
{
    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    if (clm920rv3_get_ipaddr(module, ipaddr) != OS_EOK)
    {
        memset(ipaddr, 0, sizeof(ipaddr));
    }

    char imei[MO_IMEI_LEN + 1] = {0};
    if (clm920rv3_get_imei(module, imei, sizeof(imei)) != OS_EOK)
    {
        memset(imei, 0, sizeof(imei));
    }

    char iccid[MO_ICCID_LEN + 1] = {0};
    if (clm920rv3_get_iccid(module, iccid, sizeof(iccid)) != OS_EOK)
    {
        memset(iccid, 0, sizeof(iccid));
    }

    os_uint8_t rssi = 0;
    os_uint8_t ber  = 0;

    if (clm920rv3_get_csq(module, &rssi, &ber) != OS_EOK)
    {
        rssi = 0;
        ber  = 0;
    }

    os_kprintf("\r\nLIST AT MODULE INFORMATIONS\r\n");
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

os_err_t clm920rv3_get_ipaddr(mo_object_t *module, char ip[])
{
    at_parser_t *parser = &module->parser;
    os_int8_t    len    = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    char resp_buff[2 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR=1");
    if (result != OS_EOK)
    {
        ERROR("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR: 1,100.174.76.132 */
    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%*[^\"]\"%[^\"]", ipaddr) <= 0)
    {
        ERROR("Get ip address: parse resp fail.");
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        ERROR("IP address size [%d] error.", len);
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

os_err_t clm920rv3_set_dnsserver(mo_object_t *module, dns_server_t dns)
{
    /* CLM920RV3 set dnsserver function just for test use, not officially support for now. */
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    char ip[IPADDR_MAX_STR_LEN + 1]       = {0};
    
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};
    
    if (OS_NULL == parser)
    {
        ERROR("CLM920RV3 %s: at parser is NULL.", __func__);
        return OS_ERROR;
    }

    if (OS_EOK != clm920rv3_get_ipaddr(module, ip))
    {
        ERROR("CLM920RV3 %s: module registrition hasn't been complete.", __func__);
        return OS_ERROR;
    }

    /**
     * CLM920RV3 set dns server function just for test use, not officially support for now. 
     * and it appears that CLM920RV3 not support "0" to reset the DNS server address.
     **/
    if (!strlen(dns.primary_dns)        || !strlen(dns.secondary_dns) || 
        !strcmp(dns.secondary_dns, "0") || !strcmp(dns.secondary_dns, "0"))
    {
        ERROR("CLM920RV3 %s: with invalid param.", __func__);
        return OS_ERROR;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT*NETDNS=1,\"%s\",\"%s\"", dns.primary_dns, dns.secondary_dns);
    
    return result;
}

os_err_t clm920rv3_get_dnsserver(mo_object_t *module, dns_server_t *dns)
{
    /* CLM920RV3 get dnsserver function just for test use, not officially support for now. */
    at_parser_t *parser         = &module->parser;
    os_err_t     result         = OS_EOK;

    char primary_dns[IPADDR_MAX_STR_LEN + 1]   = {0};
    char secondary_dns[IPADDR_MAX_STR_LEN + 1] = {0};

    if (OS_NULL == parser)
    {
        ERROR("CLM920RV3 %s: at parser is NULL.", __func__);
        return OS_ERROR;
    }

    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &at_resp, "AT*NETDNS=1");

    if (at_resp_get_data_by_kw(&at_resp, "*NETDNS:", "*NETDNS: %[^,],%s", primary_dns, secondary_dns) <= 0)
    {
        ERROR("CLM920RV3 %s: get dns server addr failed.", __func__);
        result = OS_ERROR;
        goto __exit;
    }

    strcpy(dns->primary_dns, primary_dns);
    strcpy(dns->secondary_dns, secondary_dns);

    DEBUG("CLM920RV3 %s: primary_dns[%s],secondary_dns[%s]", __func__, dns->primary_dns, dns->secondary_dns);

__exit:
    return result;
}

#endif /* CLM920RV3_USING_IFCONFIG_OPS */

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
 * @file        ec20_ifconfig.c
 *
 * @brief       ec20 module link kit ifconfig api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <mo_ipaddr.h>
#include "ec20.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "ec20_ifconfig"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef EC20_USING_IFCONFIG_OPS

extern os_err_t ec20_pdp_act(mo_object_t *module);

os_err_t ec20_ifconfig(mo_object_t *self)
{
    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    if (ec20_get_ipaddr(self, ipaddr) != OS_EOK)
    {
        memset(ipaddr, 0, sizeof(ipaddr));
    }

    char imei[MO_IMEI_LEN + 1] = {0};
    if (ec20_get_imei(self, imei, sizeof(imei)) != OS_EOK)
    {
        memset(imei, 0, sizeof(imei));
    }

    char iccid[MO_ICCID_LEN + 1] = {0};
    if (ec20_get_iccid(self, iccid, sizeof(iccid)) != OS_EOK)
    {
        memset(iccid, 0, sizeof(iccid));
    }

    os_uint8_t rssi = 0;
    os_uint8_t ber  = 0;

    if (ec20_get_csq(self, &rssi, &ber) != OS_EOK)
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
    os_kprintf("Module Name    : %s\r\n", self->name);
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

os_err_t ec20_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    len    = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR=1");
    if (result != OS_EOK)
    {
        ERROR("Get ip address fail: AT+CGPADDR cmd exec fail.");
        goto __exit;
    }

    /* Response for ex: +CGPADDR:0,100.113.120.235 */
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

os_err_t ec20_set_dnsserver(mo_object_t *self, dns_server_t dns)
{
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;
    
    mo_ec20_t *ec20 = os_container_of(self, mo_ec20_t, parent);

    char ip[IPADDR_MAX_STR_LEN + 1] = {0};
    if (OS_EOK != ec20_get_ipaddr(self, ip))
    {
        ERROR("EC20 %s: module registrition hasn't been complete.", __func__);
        return OS_ERROR;
    }
    
    if (OS_FALSE == ec20->pdp_act)
    {
        result = ec20_pdp_act(self);
        if (OS_EOK != result)
        {
            ERROR("EC20 %s: set pdp context act failed.", __func__);
            return result;
        }
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    if (OS_NULL == parser)
    {
        ERROR("EC20 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    /* it appears that module will reset dns if secondary dns is "0" */
    if (0 == strlen(dns.primary_dns) || 0 == strcmp(dns.secondary_dns, "0"))
    {
        ERROR("EC20 %s: with invalid param.", __FUNCTION__);
        return OS_ERROR;
    }

    if (0 == strlen(dns.secondary_dns))
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=1,\"%s\"", dns.primary_dns);
    }
    else
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSCFG=1,\"%s\",\"%s\"", dns.primary_dns, dns.secondary_dns);
    }
    
    return result;
}

os_err_t ec20_get_dnsserver(mo_object_t *self, dns_server_t *dns)
{
    at_parser_t *parser         = &self->parser;
    os_err_t     result         = OS_EOK;

    char primary_dns[IPADDR_MAX_STR_LEN + 1] = {0};

    if (OS_NULL == parser)
    {
        ERROR("EC20 %s: at parser is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    if (OS_NULL == dns)
    {
        ERROR("EC20 %s: dns is NULL.", __FUNCTION__);
        return OS_ERROR;
    }

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 2,
                         .timeout   = OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &at_resp, "AT+QIDNSCFG=1");

    if (at_resp_get_data_by_kw(&at_resp, "+QIDNSCFG:", "+QIDNSCFG: 1,%[^,],\"%[^\"]", primary_dns, dns->secondary_dns) <= 0)
    {
        ERROR("EC20 %s: get dns failed.", __FUNCTION__);
        result = OS_ERROR;
        goto __exit;
    }

    sscanf(primary_dns, "\"%[^\"]", dns->primary_dns);

    DEBUG("EC20 %s: primary_dns[%s],secondary_dns[%s]", __FUNCTION__, dns->primary_dns, dns->secondary_dns);

__exit:
    return result;
}

#endif /* EC20_USING_IFCONFIG_OPS */

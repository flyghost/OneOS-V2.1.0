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
 * @file        esp8266_ifconfig.c
 *
 * @brief       esp8266 module link kit ifconfig api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <mo_ipaddr.h>
#include "esp8266_wifi.h"
#include "esp8266_ifconfig.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "esp8266.ifconfig"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef ESP8266_USING_IFCONFIG_OPS

os_err_t esp8266_ifconfig(mo_object_t *self)
{
    const char *mode_to_str[] = {
        [MO_WIFI_MODE_NULL]   = "Null",
        [MO_WIFI_MODE_STA]    = "Station",
        [MO_WIFI_MODE_AP]     = "SoftAP",
        [MO_WIFI_MODE_AP_STA] = "SoftAP+Station",
    };

    const char *stat_to_str[] = {
        [MO_WIFI_STAT_NULL]         = "Null",
        [MO_WIFI_STAT_INIT]         = "Initial",
        [MO_WIFI_STAT_CONNECTED]    = "Connected",
        [MO_WIFI_STAT_DISCONNECTED] = "Disconnected",
    };

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    os_err_t ret = OS_EOK;

    if (esp8266_get_ipaddr(self, ipaddr) != OS_EOK)
    {
        memset(ipaddr, 0, sizeof(ipaddr));
        ret = OS_ERROR;
    }

    mo_wifi_mode_t mode = esp8266_wifi_get_mode(self);
    mo_wifi_stat_t stat = esp8266_wifi_get_stat(self);

    os_kprintf("\r\nLIST AT MODULE INFORMATION\r\n");
    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }

    os_kprintf("\r\n");
    os_kprintf("Module   Name    : %s\r\n", self->name);
    os_kprintf("WiFi     Mode    : %s\r\n", mode_to_str[mode]);
    if (stat >= MO_WIFI_STAT_GOT_IP)
    {
        stat = MO_WIFI_STAT_CONNECTED;
    }
    os_kprintf("WiFi     Status  : %s\r\n", stat_to_str[stat]);

    if (mode == MO_WIFI_MODE_STA || mode == MO_WIFI_MODE_AP_STA)
    {
        os_kprintf("STA IPv4 Address : %s\r\n", strlen(ipaddr) ? ipaddr : "0.0.0.0");
    }

    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\r\n");

    return ret;
}

os_err_t esp8266_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;
    os_int8_t    len    = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1]   = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIFSR");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    /* Response for ex: +CIFSR:STAIP,"100.113.120.235" */
    if (at_resp_get_data_by_kw(&resp, "+CIFSR:STAIP", "+CIFSR:STAIP,\"%[^\"]", ipaddr) <= 0)
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

os_err_t esp8266_set_dnsserver(mo_object_t *self, dns_server_t dns)
{
    if (strlen(dns.primary_dns) == 0)
    {
        ERROR("Module %s set dns server failed! There is no valid primary DNS server address!");
        return OS_ERROR;
    }

    if (strncmp(dns.primary_dns, dns.secondary_dns, IPADDR_MAX_STR_LEN) == 0)
    {
        ERROR("Module %s set dns server failed! Cannot set the same DNS server address!");
        return OS_ERROR;
    }

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    os_err_t result = OS_EOK;

    if (strlen(dns.secondary_dns) == 0)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CIPDNS_DEF=1,\"%s\"", dns.primary_dns);
    }
    else
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CIPDNS_DEF=1,\"%s\",\"%s\"", dns.primary_dns, dns.secondary_dns);
    }

    return result;
}

os_err_t esp8266_get_dnsserver(mo_object_t *self, dns_server_t *dns)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIPDNS_DEF?");
    if (result != OS_EOK)
    {
        return result;
    }

    if (at_resp_get_data_by_line(&resp, 1, "+CIPDNS_DEF:%s", dns->primary_dns) < 0)
    {
        ERROR("Failed to resolve the primary DNS server address");
        return OS_ERROR;
    }

    if (resp.line_counts >= 3)
    {
        /* Attempt to resolve the secondary DNS server address */
        if (at_resp_get_data_by_line(&resp, 2, "+CIPDNS_DEF:%s", dns->secondary_dns) < 0)
        {
            ERROR("Failed to resolve the secondary DNS server address");
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

#endif /* ESP8266_USING_IFCONFIG_OPS */

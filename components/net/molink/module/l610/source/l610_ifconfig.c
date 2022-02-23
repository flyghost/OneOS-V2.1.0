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
 * @file        l610_ifconfig.c
 *
 * @brief       l610 module link kit ifconfig api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <mo_ipaddr.h>
#include "l610_general.h"
#include "l610_netserv.h"
#include "l610_ifconfig.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "l610.ifconfig"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef L610_USING_IFCONFIG_OPS

os_err_t l610_ifconfig(mo_object_t *self)
{
    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    char imei[MO_IMEI_LEN + 1]          = {0};
    char iccid[MO_ICCID_LEN + 1]        = {0};
    char imsi[MO_IMSI_LEN + 1]          = {0};

    os_err_t   ret  = OS_EOK;
    os_uint8_t rssi = 0;
    os_uint8_t ber  = 0;

    if (l610_get_ipaddr(self, ipaddr) != OS_EOK)
    {
        memset(ipaddr, 0, sizeof(ipaddr));
        ret = OS_ERROR;
    }

    if (l610_get_imei(self, imei, sizeof(imei)) != OS_EOK)
    {
        memset(imei, 0, sizeof(imei));
        ret = OS_ERROR;
    }

    if (l610_get_iccid(self, iccid, sizeof(iccid)) != OS_EOK)
    {
        memset(iccid, 0, sizeof(iccid));
        ret = OS_ERROR;
    }

    if (l610_get_imsi(self, imsi, sizeof(imsi)) != OS_EOK)
    {
        memset(imsi, 0, sizeof(imsi));
        ret = OS_ERROR;
    }

    if (l610_get_csq(self, &rssi, &ber) != OS_EOK)
    {
        rssi = 0;
        ber  = 0;
        ret = OS_ERROR;
    }

    os_kprintf("\r\nLIST AT MODULE INFORMATION\r\n");
    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }

    os_kprintf("\r\n");
    os_kprintf("Module Name    : %s\r\n", self->name);
    os_kprintf("IMEI   Number  : %s\r\n", imei);
    os_kprintf("ICCID  Number  : %s\r\n", iccid);
    os_kprintf("IMSI   Number  : %s\r\n", imsi);
    os_kprintf("Signal Quality : rssi(%d), ber(%d)\r\n", rssi, ber);
    os_kprintf("IPv4   Address : %s\r\n", strlen(ipaddr) ? ipaddr : "0.0.0.0");

    for (int i = 0; i < 40; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\r\n");

    return ret;
}

os_err_t l610_get_ipaddr(mo_object_t *self, char ip[])
{
    at_parser_t *parser = &self->parser;

    char ipaddr[IPADDR_MAX_STR_LEN + 1]   = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout =  2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGPADDR");
    if (result != OS_EOK)
    {
        ERROR("Get ip address fail: AT+CGPADDR cmd exec fail.");
        return result;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGPADDR:", "+CGPADDR:%*d,\"%[^,^\"]%*s", ipaddr) <= 0)
    {
        ERROR("Get ip address: parse resp fail.");
        result = OS_ERROR;
        return result;
    }

    os_int8_t len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        ERROR("IP address size [%d] error.", len);
        result = OS_ERROR;
        return result;
    }
    else
    {
        strcpy(ip, ipaddr);
        DEBUG("IP address: %s", ip);
    }

    return result;
}

#endif /* L610_USING_IFCONFIG_OPS */

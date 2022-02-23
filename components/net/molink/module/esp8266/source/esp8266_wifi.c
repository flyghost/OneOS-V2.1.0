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
 * @file        esp8266_wifi.c
 *
 * @brief       esp8266 module link kit wifi api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp8266_wifi.h"
#include "esp8266.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "esp8266.wifi"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define ESP8266_SCAN_RESP_BUFF_LEN 4096

#ifdef ESP8266_USING_WIFI_OPS

os_err_t esp8266_wifi_set_ipdinofo(mo_object_t *module, mo_cipdinfo_mode_t ipd_mode)
{
    at_parser_t *parser = &module->parser;
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_int8_t mode_data = 0;
    os_err_t do_err;

    switch (ipd_mode)
    {
    case MO_WIFI_CIPINFO_HIDE_IP_PORT:
        mode_data = 0;
        break;
    case MO_WIFI_CIPINFO_SHOW_IP_PORT:
        mode_data = 1;
        break;
    default:
        WARN("AP and AP&STA modes are not supported and AT instructions will not be executed");
        return OS_ERROR;
    }

    do_err = at_parser_exec_cmd(parser, &resp, "AT+CIPDINFO=%hhd", mode_data);
    if (OS_EOK == do_err)
    {
        esp8266->wifi_ipdinfo = ipd_mode;
    }

    return do_err;
}

os_err_t esp8266_wifi_set_ipv6(mo_object_t *module, mo_wifi_cipv6_t ipv6)
{
    at_parser_t *parser = &module->parser;
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_int8_t mode_data = 0;
    os_err_t do_err;

    switch (ipv6)
    {
    case MO_WIFI_CIPV6_DISABLE:
        mode_data = 0;
        break;
    case MO_WIFI_CIPV6_ENABLE:
        mode_data = 1;
        break;
    default:
        WARN("AP and AP&STA modes are not supported and AT instructions will not be executed");
        return OS_ERROR;
    }

    do_err = at_parser_exec_cmd(parser, &resp, "AT+CIPV6=%hhd", mode_data);
    if (OS_EOK == do_err)
    {
        esp8266->wifi_ipv6 = ipv6;
    }

    return do_err;
}

os_err_t esp8266_wifi_set_mode(mo_object_t *module, mo_wifi_mode_t mode)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    os_int8_t mode_data = 0;

    switch (mode)
    {
    case MO_WIFI_MODE_STA:
        mode_data = 1;
        break;
    case MO_WIFI_MODE_AP_STA:
        mode_data = 3;
        break;
    case MO_WIFI_MODE_AP:
    default:
        WARN("AP and AP&STA modes are not supported and AT instructions will not be executed");
        return OS_ERROR;
    }

    return at_parser_exec_cmd(parser, &resp, "AT+CWMODE=%hhd", mode_data);
}

mo_wifi_mode_t esp8266_wifi_get_mode(mo_object_t *module)
{
    at_parser_t   *parser    = &module->parser;
    os_int8_t      mode_data = 0;
    mo_wifi_mode_t wifi_mode = MO_WIFI_MODE_NULL;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CWMODE?");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "+CWMODE:", "+CWMODE:%hhd", &mode_data) < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }


    switch (mode_data)
    {
    case 1:
        wifi_mode = MO_WIFI_MODE_STA;
        break;
    case 3:
        wifi_mode = MO_WIFI_MODE_AP_STA;
        break;
    default:
        break;
    }

__exit:

    if (result != OS_EOK)
    {
        return MO_WIFI_MODE_NULL;
    }

    return wifi_mode;
}

mo_wifi_stat_t esp8266_wifi_get_stat(mo_object_t *module)
{
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);
    at_parser_t  *parser  = &module->parser;

    if (esp8266->wifi_stat >= MO_WIFI_STAT_GOT_IP)
    {
        return esp8266->wifi_stat;
    }

    char *resp_buff = os_calloc(1, AT_RESP_BUFF_SIZE_384);

    at_resp_t resp = {.buff = resp_buff, .buff_size = AT_RESP_BUFF_SIZE_384, .timeout = 5 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+CIPSTATUS") != OS_EOK)
    {
        esp8266->wifi_stat = MO_WIFI_STAT_NULL;
        os_free(resp_buff);

        return esp8266->wifi_stat;
    }

    os_uint8_t wifi_stat = 0;

    if (at_resp_get_data_by_kw(&resp, "STATUS:", "STATUS:%hhu", &wifi_stat) <= 0)
    {
        ERROR("Failed to check the status of module %s", module->name);
        esp8266->wifi_stat = MO_WIFI_STAT_NULL;
        os_free(resp_buff);

        return esp8266->wifi_stat;
    }

    switch (wifi_stat)
    {
    case 5: /* Esp8266 station not connected to AP */
        esp8266->wifi_stat = MO_WIFI_STAT_DISCONNECTED;
        break;

    case 2: /* Esp8266 station has connected to AP and obtained IP address */
    case 3: /* Esp8266 station has established TCP or UDP transmission */
    case 4: /* Esp8266 station is disconnected from the network */
        esp8266->wifi_stat = MO_WIFI_STAT_CONNECTED;
        break;

    default:
        ERROR("Wrong status code");
        break;
    }

    os_free(resp_buff);

    return esp8266->wifi_stat;
}

os_err_t esp8266_wifi_get_sta_cip(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask, ip_addr_t *ip6_ll, ip_addr_t *ip6_gl)
{
    at_parser_t *parser = &module->parser;
    os_int8_t    len    = -1;
//    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    //char gateway[IPADDR_MAX_STR_LEN + 1] = {0};
    //char netmask[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIPSTA?");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    /* Response for ex: +CIPSTA:ip:"192.168.5.34" */
    if (at_resp_get_data_by_kw(&resp, "+CIPSTA:ip:", "+CIPSTA:ip:\"%[^\"]", ipaddr) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        if (OS_NULL != ip)
        {
            IP_SET_TYPE(ip, IPADDR_TYPE_V4);
            inet_aton(ipaddr, ip);
        }
// #ifdef MOLINK_USING_IPV4
// #ifdef MOLINK_USING_IPV6
//         ip->u_addr.ip4.addr = inet_addr(ipaddr);
// #else
//         ip->addr = inet_addr(ipaddr);
// #endif
// #endif
        DEBUG("IP address: %s", ipaddr);
    }

    /* Response for ex: +CIPSTA:gateway:"192.168.5.1" */
    if (at_resp_get_data_by_kw(&resp, "+CIPSTA:gateway:", "+CIPSTA:gateway:\"%[^\"]", ipaddr) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        if (OS_NULL != gw)
        {
            IP_SET_TYPE(gw, IPADDR_TYPE_V4);
            inet_aton(ipaddr, gw);
        }
// #ifdef MOLINK_USING_IPV4
// #ifdef MOLINK_USING_IPV6
//         gw->u_addr.ip4.addr = inet_addr(ipaddr);
// #else
//         gw->addr = inet_addr(ipaddr);
// #endif
// #endif
        DEBUG("Gateway: %s", ipaddr);
    }

    /* Response for ex: +CIPSTA:netmask:"255.255.255.0" */
    if (at_resp_get_data_by_kw(&resp, "+CIPSTA:netmask:", "+CIPSTA:netmask:\"%[^\"]", ipaddr) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        if (OS_NULL != mask)
        {
            IP_SET_TYPE(mask, IPADDR_TYPE_V4);
            inet_aton(ipaddr, mask);
        }
// #ifdef MOLINK_USING_IPV4
// #ifdef MOLINK_USING_IPV6
//         mask->u_addr.ip4.addr = inet_addr(ipaddr);
// #else
//         mask->addr = inet_addr(ipaddr);
// #endif
// #endif
        DEBUG("Netmask: %s", ipaddr);
    }

#ifdef MOLINK_USING_IPV6
    if (MO_WIFI_CIPV6_ENABLE == esp8266->wifi_ipv6)
    {

        /* Response for ex: +CIPSTA:ip6ll:"FE80::3E61:5FF:FED0:9523"" */
        if (at_resp_get_data_by_kw(&resp, "+CIPSTA:ip6ll:", "+CIPSTA:ip6ll:\"%[^\"]", ipaddr) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        len = strlen(ipaddr);
        if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
        {
            result = OS_ERROR;
            goto __exit;
        }
        else
        {
            if (OS_NULL != ip6_ll)
            {
                IP_SET_TYPE(ip6_ll, IPADDR_TYPE_V6);
                inet_aton(ipaddr, ip6_ll); 
            }
        }

        /* Response for ex: +CIPSTA:ip6gl:"2409:8762:EFD:1A:3E61:5FF:FED0:9523" */
        if (at_resp_get_data_by_kw(&resp, "+CIPSTA:ip6gl:", "+CIPSTA:ip6gl:\"%[^\"]", ipaddr) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        len = strlen(ipaddr);
        if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
        {
            result = OS_ERROR;
            goto __exit;
        }
        else
        {
            if (OS_NULL != ip6_gl)
            {
                IP_SET_TYPE(ip6_gl, IPADDR_TYPE_V6);
                inet_aton(ipaddr, ip6_gl); 
            }
        }
    }
#endif

__exit:

    return result;
}

os_err_t esp8266_wifi_set_ap_cip(mo_object_t *module, char *ip, char *gw, char *mask)
{
    at_parser_t *parser = &module->parser;
    char cmd[128] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF * 2] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    snprintf(cmd, sizeof(cmd), "AT+CIPAP=\"%s\"", ip);
    
    return at_parser_exec_cmd(parser, &resp, cmd);
}

os_err_t esp8266_wifi_get_ap_cip(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask)
{
    at_parser_t *parser = &module->parser;
    
    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    char gateway[IPADDR_MAX_STR_LEN + 1] = {0};
    char netmask[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};
    int len;
    
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIPAP?");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    /* Response for ex: +CIPAP:ip:"192.168.5.34" */
    if (at_resp_get_data_by_kw(&resp, "+CIPAP:ip:", "+CIPAP:ip:\"%[^\"]", ipaddr) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(ipaddr);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
#ifdef MOLINK_USING_IPV4
#ifdef MOLINK_USING_IPV6
        ip->u_addr.ip4.addr = inet_addr(ipaddr);
#else
        ip->addr = inet_addr(ipaddr);
#endif
#endif   
        DEBUG("IP address: %s", ipaddr);
    }

    /* Response for ex: +CIPSTA:gateway:"192.168.5.1" */
    if (at_resp_get_data_by_kw(&resp, "+CIPAP:gateway:", "+CIPAP:gateway:\"%[^\"]", gateway) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(gateway);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
#ifdef MOLINK_USING_IPV4
#ifdef MOLINK_USING_IPV6
        gw->u_addr.ip4.addr = inet_addr(gateway);
#else       
        gw->addr = inet_addr(gateway); 
#endif
#endif
        DEBUG("Gateway: %s", gateway);
    }

    /* Response for ex: +CIPSTA:netmask:"255.255.255.0" */
    if (at_resp_get_data_by_kw(&resp, "+CIPAP:netmask:", "+CIPAP:netmask:\"%[^\"]", netmask) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    len = strlen(netmask);
    if ((len < IPADDR_MIN_STR_LEN) || (len > IPADDR_MAX_STR_LEN))
    {
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
#ifdef MOLINK_USING_IPV4
#ifdef MOLINK_USING_IPV6
        mask->u_addr.ip4.addr = inet_addr(netmask);
#else
        mask->addr = inet_addr(netmask);
#endif
#endif
        DEBUG("Netmask: %s", netmask);
    }

__exit:

    return result;
}

/* Returns int value of hex string character |c| */
static uint8_t hex_to_uint(uint8_t c) {
  if ('0' <= c && c <= '9') {
    return (uint8_t)(c - '0');
  }
  if ('A' <= c && c <= 'F') {
    return (uint8_t)(c - 'A' + 10);
  }
  if ('a' <= c && c <= 'f') {
    return (uint8_t)(c - 'a' + 10);
  }
  return 0;
}

os_err_t esp8266_wifi_get_sta_mac(mo_object_t *module, char mac[])
{
    at_parser_t  *parser  = &module->parser;
    const char *mac_resp;
    char *resp_buff = calloc(1, 128);
    char mac_key[] = "+CIPSTAMAC:\"";
	const char *mac_start;
    int index;

    at_resp_t resp = {.buff = resp_buff, .buff_size = 128, .timeout = 5 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+CIPSTAMAC?") != OS_EOK)
    {
        free(resp_buff);

        return OS_ERROR;
    }

    mac_resp = at_resp_get_line_by_kw(&resp, "+CIPSTAMAC:");
    if (OS_NULL == mac_resp)
    {
        free(resp_buff);

        return OS_ERROR;     
    }

	mac_start = mac_resp + strlen(mac_key);
	for (index = 0; index < 6; index++)
	{
		mac[index] = hex_to_uint(*mac_start);
		mac[index] = mac[index] << 4;

		mac_start++;
		mac[index] += hex_to_uint(*mac_start);
		mac_start++;
		mac_start++;
	}

    free(resp_buff);

    return OS_EOK;
}

os_err_t esp8266_wifi_get_ap_mac(mo_object_t *module, char mac[])
{
    at_parser_t  *parser  = &module->parser;
    const char *mac_resp;
    char *resp_buff = calloc(1, 128);
    char mac_key[] = "+CIPAPMAC:\"";
	const char *mac_start;
    int index;

    at_resp_t resp = {.buff = resp_buff, .buff_size = 128, .timeout = 5 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+CIPAPMAC?") != OS_EOK)
    {
        free(resp_buff);
        return OS_ERROR;
    }

    mac_resp = at_resp_get_line_by_kw(&resp, "+CIPAPMAC:");
    if (OS_NULL == mac_resp)
    {
        free(resp_buff);
        return OS_ERROR;     
    }

	mac_start = mac_resp + strlen(mac_key);
	for (index = 0; index < 6; index++)
	{
		mac[index] = hex_to_uint(*mac_start);
		mac[index] = mac[index] << 4;

		mac_start++;
		mac[index] += hex_to_uint(*mac_start);
		mac_start++;
		mac_start++;
	}
    
    free(resp_buff);

    return OS_EOK;
}

os_err_t esp8266_wifi_scan_info(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result)
{
    at_parser_t *parser   = &module->parser;
    os_err_t     result   = OS_EOK;
    os_int32_t   ecn_mode = 0;

    const char *data_format1 = "+CWLAP:(%d,\"%[^\"]\",%d,\"%[^\"]\",%d,%*s)";
    const char *data_format2 = "+CWLAP:(%*d,\"\",%d,\"%[^\"]\",%d,%*s)";

    at_resp_t resp = {.buff      = os_calloc(1, ESP8266_SCAN_RESP_BUFF_LEN),
                      .buff_size = ESP8266_SCAN_RESP_BUFF_LEN,
                      .timeout   = 5 * OS_TICK_PER_SECOND};

    if (OS_NULL == resp.buff)
    {
        ERROR("os_calloc wifi scan info response memory failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    if (OS_NULL != ssid)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CWLAP=\"%s\"", ssid);
        if (result != OS_EOK)
        {
            goto __exit;
        }
    }
    else
    {
        /* scan all ap list */
        result = at_parser_exec_cmd(parser, &resp, "AT+CWLAP");
        if (result != OS_EOK)
        {
            goto __exit;
        }
    }

    scan_result->info_array = (mo_wifi_info_t *)os_calloc(resp.line_counts, sizeof(mo_wifi_info_t));
    if (OS_NULL == scan_result->info_array)
    {
        ERROR("os_calloc wifi scan info memory failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    scan_result->info_num = 0;

    for (int i = 0; i < resp.line_counts; i++)
    {
        mo_wifi_info_t *tmp = &scan_result->info_array[i];

        os_int32_t get_result = at_resp_get_data_by_line(&resp,
                                                         i + 1,
                                                         data_format1,
                                                         &ecn_mode,
                                                         tmp->ssid.val,
                                                         &tmp->rssi,
                                                         tmp->bssid.bssid_str,
                                                         &tmp->channel);

        if (1 == get_result)
        {
            /* ssid is null  */
            get_result = at_resp_get_data_by_line(&resp,
                                                  i + 1,
                                                  data_format2,
                                                  &tmp->rssi,
                                                  tmp->bssid.bssid_str,
                                                  &tmp->channel);
        }

        if (get_result > 0)
        {
            switch (ecn_mode)
            {
            case 0:
                tmp->ecn_mode = MO_WIFI_ECN_OPEN;
                break;

            case 1:
                tmp->ecn_mode = MO_WIFI_ECN_WEP;
                break;

            case 2:
                tmp->ecn_mode = MO_WIFI_ECN_WPA_PSK;
                break;

            case 3:
                tmp->ecn_mode = MO_WIFI_ECN_WPA2_PSK;
                break;

            case 4:
                tmp->ecn_mode = MO_WIFI_ECN_WPA_WPA2_PSK;
                break;

            default:
                tmp->ecn_mode = MO_WIFI_ECN_NULL;
                break;
            }

            sscanf(tmp->bssid.bssid_str,
                   "%2x:%2x:%2x:%2x:%2x:%2x",
                   (os_int32_t *)&tmp->bssid.bssid_array[0],
                   (os_int32_t *)&tmp->bssid.bssid_array[1],
                   (os_int32_t *)&tmp->bssid.bssid_array[2],
                   (os_int32_t *)&tmp->bssid.bssid_array[3],
                   (os_int32_t *)&tmp->bssid.bssid_array[4],
                   (os_int32_t *)&tmp->bssid.bssid_array[5]);

            tmp->ssid.len = strlen(tmp->ssid.val);
            scan_result->info_num++;
        }
    }

__exit:
    if (result != OS_EOK)
    {
        if (scan_result->info_array != OS_NULL)
        {
            os_free(scan_result->info_array);
        }
		os_task_msleep(1000);
    }

    if (resp.buff != OS_NULL)
    {
        os_free(resp.buff);
    }

    return result;
}

os_err_t esp8266_wifi_connect_ap(mo_object_t *module, const char *ssid, const char *password)
{
    at_parser_t *parser = &module->parser;
    char *err_value;
    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 40 * OS_TICK_PER_SECOND};
    os_err_t result;

    if (OS_NULL == password)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CWJAP=\"%s\",", ssid);
    }
    else
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    }
    if (result != OS_EOK)
    {
        err_value = strstr(resp_buff, "+CWJAP");
        if (OS_NULL != err_value)
        {
            sscanf(err_value, "+CWJAP:%d", &result);
            os_kprintf("do cmd[+CWJAP] resp: %s\n", err_value);
        }
    }

    return result;
}

os_err_t esp8266_wifi_disconnect_ap(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CWQAP");
}

os_err_t esp8266_wifi_start_ap(mo_object_t *module, const char *ssid, const char *password, os_uint8_t channel, os_uint8_t ecn)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 40 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CWSAP=\"%s\",\"%s\",%d,%d", ssid, password, channel, ecn);
    if (result != OS_EOK)
    {
        ERROR("Module %s start ap failed, [%s, %s, %d, %d].", module->name, ssid, password, channel, ecn);
    }

    return result;
}

os_err_t esp8266_wifi_stop_ap(mo_object_t *module)
{
    //at_parser_t *parser = &module->parser;
    
    return OS_EOK;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != data);

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    if (strstr(data, "WIFI CONNECTED"))
    {
        esp8266->wifi_stat = MO_WIFI_STAT_CONNECTED;
        DEBUG("ESP8266 WIFI connected.");
    }
    else if (strstr(data, "WIFI DISCONNECT"))
    {
        esp8266->wifi_stat = MO_WIFI_STAT_DISCONNECTED;
        DEBUG("ESP8266 WIFI disconnected.");
    }
}

static void urc_ip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    INFO("ESP8266 WIFI has get ip");
    esp8266->wifi_stat = MO_WIFI_STAT_GOT_IP;
}

static void urc_ipv6_func(struct at_parser *parser, const char *data, os_size_t size)
{
    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    INFO("ESP8266 WIFI has get ipv6 gl");

    esp8266->wifi_stat |= MO_WIFI_STAT_GOT_IPV6;
}

static void urc_sta_connected_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("%s", data);
}

static void urc_sta_disconnected_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("%s", data);
}

static void urc_dist_sta_ip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("%s", data);
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "WIFI CONNECTED",        .suffix = "\n",   .func = urc_connect_func},
    {.prefix = "WIFI DISCONNECT",       .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "WIFI GOT IPv6 GL",      .suffix = "\r\n", .func = urc_ipv6_func},
    {.prefix = "WIFI GOT IP",           .suffix = "\r\n", .func = urc_ip_func},
    {.prefix = "+STA_CONNECTED:",       .suffix = "\n",   .func = urc_sta_connected_func},
    {.prefix = "+STA_DISCONNECTED:",    .suffix = "\n",   .func = urc_sta_disconnected_func},
    {.prefix = "+DIST_STA_IP:",         .suffix = "\n",   .func = urc_dist_sta_ip_func},
};

os_err_t esp8266_wifi_init(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return OS_EOK;
}

#endif /* ESP8266_USING_WIFI_OPS */

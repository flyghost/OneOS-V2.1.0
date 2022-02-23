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
 * @file        esp32_wifi.c
 *
 * @brief       esp32 module link kit wifi api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp32_wifi.h"
#include "esp32.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "esp32.wifi"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define ESP32_SCAN_RESP_BUFF_LEN 4096

#ifdef ESP32_USING_WIFI_OPS

os_err_t esp32_wifi_set_mode(mo_object_t *module, mo_wifi_mode_t mode)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_int8_t mode_data = 0;

    switch (mode)
    {
    case MO_WIFI_MODE_STA:
        mode_data = 1;
        break;

    case MO_WIFI_MODE_AP:
    case MO_WIFI_MODE_AP_STA:
    default:
        WARN("AP and AP&STA modes are not supported and AT instructions will not be executed");
        return OS_ERROR;
    }

    return at_parser_exec_cmd(parser, &resp, "AT+CWMODE=%hhd", mode_data);
}

mo_wifi_mode_t esp32_wifi_get_mode(mo_object_t *module)
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

mo_wifi_stat_t esp32_wifi_get_stat(mo_object_t *module)
{
    mo_esp32_t  *esp32  = os_container_of(module, mo_esp32_t, parent);
    at_parser_t *parser = &module->parser;

    char *resp_buff = os_calloc(1, AT_RESP_BUFF_SIZE_384);

    at_resp_t resp = {.buff = resp_buff, .buff_size = AT_RESP_BUFF_SIZE_384, .timeout = 5 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+CIPSTATUS") != OS_EOK)
    {
        esp32->wifi_stat = MO_WIFI_STAT_NULL;
        os_free(resp_buff);

        return esp32->wifi_stat;
    }

    os_uint8_t wifi_stat = 0;

    if (at_resp_get_data_by_kw(&resp, "STATUS:", "STATUS:%hhu", &wifi_stat) <= 0)
    {
        ERROR("Failed to check the status of module %s", module->name);
        esp32->wifi_stat = MO_WIFI_STAT_NULL;
        os_free(resp_buff);

        return esp32->wifi_stat;
    }

    switch (wifi_stat)
    {
    case 5: /* Esp8266 station not connected to AP */
        esp32->wifi_stat = MO_WIFI_STAT_DISCONNECTED;
        break;

    case 2: /* Esp8266 station has connected to AP and obtained IP address */
    case 3: /* Esp8266 station has established TCP or UDP transmission */
    case 4: /* Esp8266 station is disconnected from the network */
        esp32->wifi_stat = MO_WIFI_STAT_CONNECTED;
        break;

    default:
        ERROR("Wrong status code");
        break;
    }

    os_free(resp_buff);

    return esp32->wifi_stat;
}

os_err_t esp32_wifi_scan_info(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result)
{
    at_parser_t *parser   = &module->parser;
    os_err_t     result   = OS_EOK;
	os_int32_t   ecn_mode = 0;

    const char *data_format1 = "+CWLAP:(%d,\"%[^\"]\",%d,\"%[^\"]\",%d,%*s)";
    const char *data_format2 = "+CWLAP:(%*d,\"\",%d,\"%[^\"]\",%d,%*s)";

    at_resp_t resp = {.buff      = os_calloc(1, ESP32_SCAN_RESP_BUFF_LEN),
                      .buff_size = ESP32_SCAN_RESP_BUFF_LEN,
                      .timeout   = 10 * OS_TICK_PER_SECOND};

    if (resp.buff == OS_NULL)
    {
        ERROR("Calloc wifi scan info response memory failed!");
        result = OS_ENOMEM;
        goto __exit;
    }

    if (ssid != OS_NULL)
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
    if (scan_result->info_array == OS_NULL)
    {
        ERROR("Calloc wifi scan info memory failed!");
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
    }

    if (resp.buff != OS_NULL)
    {
        os_free(resp.buff);
    }

    return result;
}

os_err_t esp32_wifi_connect_ap(mo_object_t *module, const char *ssid, const char *password)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    if (result != OS_EOK)
    {
        ERROR("Module %s connect ap failed, check ssid(%s) and password(%s).", module->name, ssid, password);
    }

    return result;
}

os_err_t esp32_wifi_disconnect_ap(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CWQAP");
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_esp32_t  *esp32  = os_container_of(module, mo_esp32_t, parent);

    if (strstr(data, "WIFI CONNECTED"))
    {
        esp32->wifi_stat = MO_WIFI_STAT_CONNECTED;
        INFO("ESP32 WIFI is connected.");
    }
    else if (strstr(data, "WIFI DISCONNECT"))
    {
        esp32->wifi_stat = MO_WIFI_STAT_DISCONNECTED;
        INFO("ESP32 WIFI is disconnect.");
    }

    return;
}

static void urc_ip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("ESP32 WIFI is get ip");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "WIFI CONNECTED",  .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "WIFI DISCONNECT", .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "WIFI GOT IP",     .suffix = "\r\n", .func = urc_ip_func},
};

os_err_t esp32_wifi_init(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return OS_EOK;
}

#endif /* ESP32_USING_WIFI_OPS */

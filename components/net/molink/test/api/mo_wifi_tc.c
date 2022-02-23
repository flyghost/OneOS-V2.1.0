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
 * @file        mo_wifi_tc.c
 *
 * @brief       module link kit wifi api test case.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <atest.h>
#include <shell.h>
#include <serial.h>
#include <mo_api.h>

#include <string.h>

os_err_t sh_exec(const char *cmd);

#define MO_LOG_TAG "molink.wifi.tc"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifndef WIFI_TC_CONNECT_SSID
#define WIFI_TC_CONNECT_SSID     "Test"
#endif

#ifndef WIFI_TC_CONNECT_PASSWORD
#define WIFI_TC_CONNECT_PASSWORD "12345678"
#endif

static mo_object_t *test_module = OS_NULL;

static void test_wifi_set_and_get_mode(void)
{
    os_err_t result  = mo_wifi_set_mode(test_module, MO_WIFI_MODE_STA);
    tp_assert_true(OS_EOK == result);

    mo_wifi_mode_t wifi_mode = mo_wifi_get_mode(test_module);
    tp_assert_true(MO_WIFI_MODE_STA == wifi_mode);
}

static void test_wifi_connect_and_disconnect_ap(void)
{
    os_err_t result = mo_wifi_connect_ap(test_module, "not_exist", "");
    tp_assert_true(OS_EOK != result);

    result = mo_wifi_connect_ap(test_module, WIFI_TC_CONNECT_SSID, WIFI_TC_CONNECT_PASSWORD);
    tp_assert_true(OS_EOK == result);

    os_task_msleep(5000);

    result = mo_wifi_disconnect_ap(test_module);
    tp_assert_true(OS_EOK == result);
}

static void test_wifi_get_stat(void)
{
    os_err_t result = mo_wifi_connect_ap(test_module, WIFI_TC_CONNECT_SSID, WIFI_TC_CONNECT_PASSWORD);
    tp_assert_true(OS_EOK == result);

    mo_wifi_stat_t wifi_stat = mo_wifi_get_stat(test_module);
    tp_assert_true(MO_WIFI_STAT_CONNECTED == wifi_stat);

    os_task_msleep(5000);

    result = mo_wifi_disconnect_ap(test_module);
    tp_assert_true(OS_EOK == result);

    wifi_stat = mo_wifi_get_stat(test_module);
    tp_assert_true(MO_WIFI_STAT_DISCONNECTED == wifi_stat);

    os_task_msleep(5000);

    result = mo_wifi_connect_ap(test_module, WIFI_TC_CONNECT_SSID, WIFI_TC_CONNECT_PASSWORD);
    tp_assert_true(OS_EOK == result);

    wifi_stat = mo_wifi_get_stat(test_module);
    tp_assert_true(MO_WIFI_STAT_CONNECTED == wifi_stat);
}

static void test_wifi_scan_info_all(void)
{
    sh_exec("memshow");

    mo_wifi_scan_result_t scan_result = {0};

    os_err_t result = mo_wifi_scan_info(test_module, OS_NULL, &scan_result);
    tp_assert_true(OS_EOK == result);

    char *security;

    for (int i = 0; i < scan_result.info_num; i++)
    {
        os_kprintf("%-32.32s", &scan_result.info_array[i].ssid.val[0]);
        os_kprintf("%2x:%2x:%2x:%2x:%2x:%2x  ",
                   scan_result.info_array[i].bssid.bssid_array[0],
                   scan_result.info_array[i].bssid.bssid_array[1],
                   scan_result.info_array[i].bssid.bssid_array[2],
                   scan_result.info_array[i].bssid.bssid_array[3],
                   scan_result.info_array[i].bssid.bssid_array[4],
                   scan_result.info_array[i].bssid.bssid_array[5]);
        switch (scan_result.info_array[i].ecn_mode)
        {
        case MO_WIFI_ECN_OPEN:
            security = "OPEN";
            break;
        case MO_WIFI_ECN_WPA_PSK:
            security = "WPA_PSK";
            break;
        case MO_WIFI_ECN_WPA2_PSK:
            security = "WPA2_PSK";
            break;
        case MO_WIFI_ECN_WPA_WPA2_PSK:
            security = "WPA_WPA2_PSK";
            break;
        default:
            security = "UNKNOWN";
            break;
        }
        os_kprintf("%-14.14s ", security);
        os_kprintf("%-4d ", scan_result.info_array[i].rssi);
        os_kprintf("%-4d ", scan_result.info_array[i].channel);
    }

    mo_wifi_scan_info_free(&scan_result);

    sh_exec("memshow");
}

static void test_wifi_scan_info_specify(void)
{
    sh_exec("memshow");

    mo_wifi_scan_result_t scan_result = {0};

    os_err_t result = mo_wifi_scan_info(test_module, WIFI_TC_CONNECT_SSID, &scan_result);
    tp_assert_true(OS_EOK == result);

    char *security;

    for (int i = 0; i < scan_result.info_num; i++)
    {
        os_kprintf("%-32.32s", &scan_result.info_array[i].ssid.val[0]);
        os_kprintf("%2x:%2x:%2x:%2x:%2x:%2x  ",
                   scan_result.info_array[i].bssid.bssid_array[0],
                   scan_result.info_array[i].bssid.bssid_array[1],
                   scan_result.info_array[i].bssid.bssid_array[2],
                   scan_result.info_array[i].bssid.bssid_array[3],
                   scan_result.info_array[i].bssid.bssid_array[4],
                   scan_result.info_array[i].bssid.bssid_array[5]);
        switch (scan_result.info_array[i].ecn_mode)
        {
        case MO_WIFI_ECN_OPEN:
            security = "OPEN";
            break;
        case MO_WIFI_ECN_WPA_PSK:
            security = "WPA_PSK";
            break;
        case MO_WIFI_ECN_WPA2_PSK:
            security = "WPA2_PSK";
            break;
        case MO_WIFI_ECN_WPA_WPA2_PSK:
            security = "WPA_WPA2_PSK";
            break;
        default:
            security = "UNKNOWN";
            break;
        }
        os_kprintf("%-14.14s ", security);
        os_kprintf("%-4d ", scan_result.info_array[i].rssi);
        os_kprintf("%-4d ", scan_result.info_array[i].channel);
    }

    mo_wifi_scan_info_free(&scan_result);

    sh_exec("memshow");
}

static void test_case(void)
{
    mo_wifi_ops_t *ops = (mo_wifi_ops_t *)test_module->ops_table[MODULE_OPS_WIFI];

    if (ops->set_mode != OS_NULL && ops->get_mode != OS_NULL)
    {
        ATEST_UNIT_RUN(test_wifi_set_and_get_mode);
    }
    
    if (ops->connect_ap != OS_NULL && ops->disconnect_ap != OS_NULL)
    {
        ATEST_UNIT_RUN(test_wifi_connect_and_disconnect_ap);
    }
    
    if (ops->get_stat != OS_NULL)
    {
        ATEST_UNIT_RUN(test_wifi_get_stat);
    }

    if (ops->scan_info != OS_NULL)
    {
        ATEST_UNIT_RUN(test_wifi_scan_info_all);
        ATEST_UNIT_RUN(test_wifi_scan_info_specify);
    }
}

static os_err_t test_init(void)
{
    test_module = mo_get_default();

    if (OS_NULL == test_module)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t test_cleanup(void)
{
    os_task_msleep(100);

    return OS_EOK;
}

ATEST_TC_EXPORT(components.net.molink.api.wifi.tc, test_case, test_init, test_cleanup, TC_PRIORITY_MIDDLE);

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
 * @file        mo_netconn_tc.c
 *
 * @brief       Molink modules netconn API test cases
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <atest.h>
#include <shell.h>
#include <os_types.h>
#include <device.h>
#include <serial.h>

#include <string.h>
#include <stdlib.h>

#include <mo_api.h>

#define MO_LOG_TAG "molink.netconn.tc"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define TEST_SERVER_IP_ADDR      "121.89.166.244"
#define TEST_SERVER_TCP_PORT     (6588)
#define TEST_SERVER_UDP_PORT     (6589)

#define TEST_MSG                 "Hello\rOneOS!0123456789abcdefghijk"

#define TEST_NETCONN_NUM          (5)

static mo_object_t *test_module = OS_NULL;

extern void module_show_socket_stat(void);

static void test_mo_netconn_create_and_destroy(void)
{
    mo_netconn_t *netconn[TEST_NETCONN_NUM] = {0};

    os_err_t          result      = OS_EOK;
    mo_netconn_type_t socket_tpye = NETCONN_TYPE_NULL;

    netconn[0] = mo_netconn_create(test_module, NETCONN_TYPE_TCP);
    tp_assert_not_null(netconn[0]);

    os_kprintf("netconn id: %d\r\n", netconn[0]->connect_id);

    result = mo_netconn_destroy(test_module, netconn[0]);
    tp_assert_true(OS_EOK == result);
    netconn[0] = OS_NULL;

    netconn[1] = mo_netconn_create(test_module, NETCONN_TYPE_UDP);
    tp_assert_not_null(netconn[1]);

    os_kprintf("netconn id: %d\r\n", netconn[1]->connect_id);

    result = mo_netconn_destroy(test_module, netconn[1]);
    tp_assert_true(OS_EOK == result);
    netconn[1] = OS_NULL;

    socket_tpye = NETCONN_TYPE_TCP;
    for (int j = 0; j < 2; j++)
    {
        for (int i = 0; i < TEST_NETCONN_NUM; i++)
        {
            netconn[i] = mo_netconn_create(test_module, socket_tpye);
            tp_assert_not_null(netconn[i]);

            os_kprintf("netconn id: %d\r\n", netconn[i]->connect_id);
            os_task_msleep(100);
        }

        for (int i = 0; i < TEST_NETCONN_NUM; i++)
        {
            result = mo_netconn_destroy(test_module, netconn[i]);
            tp_assert_true(OS_EOK == result);

            os_task_msleep(100);
        }

        /* Wait for all resources are released */
        os_task_msleep(3000);
        socket_tpye = NETCONN_TYPE_UDP;
    }
}

static void test_mo_netconn_connect(void)
{
    ip_addr_t server_addr = {0};
    os_err_t  result      = OS_EOK;

    inet_aton(TEST_SERVER_IP_ADDR, &server_addr);

    mo_netconn_t *netconn[TEST_NETCONN_NUM] = {0};

    for (int i = 0; i < TEST_NETCONN_NUM; i++)
    {
        netconn[i] = mo_netconn_create(test_module, NETCONN_TYPE_TCP);
        tp_assert_not_null(netconn[i]);
        os_task_msleep(100);

        result = mo_netconn_connect(test_module, netconn[i], server_addr, TEST_SERVER_TCP_PORT);
        tp_assert_true(OS_EOK == result);
        tp_assert_true(ip_addr_cmp(&server_addr, &netconn[i]->remote_ip));
        tp_assert_true(TEST_SERVER_TCP_PORT == netconn[i]->remote_port);

        os_task_msleep(100);
    }

    module_show_socket_stat();

    for (int i = 0; i < TEST_NETCONN_NUM; i++)
    {
        result = mo_netconn_destroy(test_module, netconn[i]);
        tp_assert_true(OS_EOK == result);

        os_task_msleep(100);
    }

    memset(netconn, 0, sizeof(netconn));

    for (int i = 0; i < TEST_NETCONN_NUM; i++)
    {
        netconn[i] = mo_netconn_create(test_module, NETCONN_TYPE_UDP);
        tp_assert_not_null(netconn[i]);
        os_task_msleep(100);

        result = mo_netconn_connect(test_module, netconn[i], server_addr, TEST_SERVER_UDP_PORT);
        tp_assert_true(OS_EOK == result);
        tp_assert_true(ip_addr_cmp(&server_addr, &netconn[i]->remote_ip));
        tp_assert_true(TEST_SERVER_UDP_PORT == netconn[i]->remote_port);

        os_task_msleep(100);
    }
    
    module_show_socket_stat();

    for (int i = 0; i < TEST_NETCONN_NUM; i++)
    {
        result = mo_netconn_destroy(test_module, netconn[i]);
        tp_assert_true(OS_EOK == result);

        os_task_msleep(100);
    }

    return;
}

#define DOMAIN_DNS_NAME "www.baidu.com"

static void test_mo_netconn_get_host_by_name(void)
{
    ip_addr_t dns_addr = {0};

    os_err_t result = mo_netconn_gethostbyname(test_module, DOMAIN_DNS_NAME, &dns_addr);
    tp_assert_true(OS_EOK == result);
    
    os_kprintf("Module %s domain[%s] dns is %s\r\n", test_module->name, DOMAIN_DNS_NAME, inet_ntoa(dns_addr));

    return;
}

static void test_mo_netconn_send_and_recv(void)
{
    ip_addr_t server_addr = {0};
    void     *data_ptr    = OS_NULL;
    os_size_t data_size   = 0;

    inet_aton(TEST_SERVER_IP_ADDR, &server_addr);

    mo_netconn_t *test_netconn = mo_netconn_create(test_module, NETCONN_TYPE_TCP);
    tp_assert_not_null(test_netconn);

    os_err_t result = mo_netconn_connect(test_module, test_netconn, server_addr, TEST_SERVER_TCP_PORT);
    tp_assert_true(OS_EOK == result);

    os_size_t sent_size = mo_netconn_send(test_module, test_netconn, TEST_MSG, strlen(TEST_MSG));
    tp_assert_true(sent_size > 0);

    result = mo_netconn_recvfrom(test_module, test_netconn, &data_ptr, &data_size, OS_NULL, OS_NULL, OS_WAIT_FOREVER);
    tp_assert_true(OS_EOK == result);
    tp_assert_true(data_size == strlen(TEST_MSG));
    os_kprintf("Module %s, netconn %d, receive data_len: %d, send data_len: %d\r\n", 
               test_module->name,
               test_netconn->connect_id,
               data_size,
               strlen(TEST_MSG));
    os_kprintf("Module %s, netconn %d, receive data: %s\r\n", test_module->name, test_netconn->connect_id, data_ptr);

    /* URC recv data packet malloc buffer, app free buffer */
    os_free(data_ptr);

    result = mo_netconn_destroy(test_module, test_netconn);
    tp_assert_true(OS_EOK == result);

    test_netconn = mo_netconn_create(test_module, NETCONN_TYPE_UDP);
    tp_assert_not_null(test_netconn);

    result = mo_netconn_connect(test_module, test_netconn, server_addr, TEST_SERVER_UDP_PORT);
    tp_assert_true(OS_EOK == result);

    sent_size = mo_netconn_send(test_module, test_netconn, TEST_MSG, strlen(TEST_MSG));
    tp_assert_true(sent_size > 0);

    result = mo_netconn_recvfrom(test_module, test_netconn, &data_ptr, &data_size, OS_NULL, OS_NULL, OS_WAIT_FOREVER);
    tp_assert_true(OS_EOK == result);
    tp_assert_true(data_size == strlen(TEST_MSG));
    os_kprintf("Module %s, netconn %d, receive data_len: %d, send data_len: %d\r\n", 
               test_module->name,
               test_netconn->connect_id,
               strlen(data_ptr),
               strlen(TEST_MSG));
    os_kprintf("Module %s, netconn %d, receive data: %s\r\n", test_module->name, test_netconn->connect_id, data_ptr);

    /* URC recv data packet malloc buffer, app free buffer */
    os_free(data_ptr);

    result = mo_netconn_destroy(test_module, test_netconn);
    tp_assert_true(OS_EOK == result);

    return;
}

static void test_case(void)
{
    mo_netconn_ops_t *ops = (mo_netconn_ops_t *)test_module->ops_table[MODULE_OPS_NETCONN];

    if (ops->gethostbyname != OS_NULL)
    {
        ATEST_UNIT_RUN(test_mo_netconn_get_host_by_name);
        os_task_msleep(2000);
    }

    ATEST_UNIT_RUN(test_mo_netconn_create_and_destroy);
    os_task_msleep(5000);
    ATEST_UNIT_RUN(test_mo_netconn_connect);
    os_task_msleep(10000);
    ATEST_UNIT_RUN(test_mo_netconn_send_and_recv);
    os_task_msleep(2000);
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

ATEST_TC_EXPORT(components.net.molink.api.netconn.tc, test_case, test_init, test_cleanup, TC_PRIORITY_MIDDLE);

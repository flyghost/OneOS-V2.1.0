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
 * @file        mo_socket_tc.c
 *
 * @brief       module link kit socket api test case.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <atest.h>
#include <os_types.h>
#include <mo_api.h>
#include <shell.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "molink.mosocket.tc"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#define TEST_IP_ADDR  "121.89.166.244"
#define TEST_TCP_PORT (6588)
#define TEST_UDP_PORT (6589)

#define TEST_MSG "Hello, \r\nOneOS!,12314567,\x61\x62\x63"

#define TEST_RECV_BUFF_LEN   (50)
#define TEST_TASK_NUM        (4)
#define TEST_SEND_DATA_TIMES (10)

static mo_object_t *test_module = OS_NULL;
static os_sem_t    *test_sem    = OS_NULL;

static void socket_task(void *parameter)
{
    struct hostent    *host = OS_NULL;
    struct sockaddr_in addr = {0};

    int sent_size = 0;
    int recv_size = 0;

    const struct timeval recv_time = {.tv_sec = 20, .tv_usec = 0};

    char recv_buff[TEST_RECV_BUFF_LEN] = {0};

    host = mo_gethostbyname(test_module, TEST_IP_ADDR);
    tp_assert_not_null(host);

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(TEST_TCP_PORT);
    addr.sin_addr   = *((struct in_addr *)host->h_addr);
    memset(&(addr.sin_zero), 0, sizeof(addr.sin_zero));

    int socket_id = mo_socket(test_module, AF_INET, SOCK_STREAM, 0);
    tp_assert_true(socket_id >= 0);

    INFO("Create socket id %d", socket_id);

    os_int32_t result = mo_connect(test_module, socket_id, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    tp_assert_true(0 == result);

    if (result < 0)
    {
        result = mo_closesocket(test_module, socket_id);
        tp_assert_true(0 == result);

        os_sem_post(test_sem);
        return;
    }

    result = mo_setsockopt(test_module, socket_id, SOL_SOCKET, SO_RCVTIMEO, &recv_time, sizeof(recv_time));
    tp_assert_true(0 == result);

    for (int i = 0; i < TEST_SEND_DATA_TIMES; i++)
    {
        sent_size = mo_send(test_module, socket_id, TEST_MSG, strlen(TEST_MSG), 0);
        tp_assert_true(sent_size > 0);

        os_task_msleep(1000);

        recv_size = mo_recv(test_module, socket_id, recv_buff, strlen(TEST_MSG), 0);
        tp_assert_true(recv_size > 0);

        if (recv_size == 0)
        {
            break;
        }

        recv_buff[recv_size] = '\0';

        INFO("Socket id %d recv %d bytes data %s", socket_id, recv_size, recv_buff);

        os_task_msleep(2000);
    }

    result = mo_closesocket(test_module, socket_id);
    tp_assert_true(0 == result);

    os_sem_post(test_sem);
}

static void sock_muti_task_test(void)
{
    char task_name[OS_NAME_MAX + 1] = {0};

    test_sem = os_sem_create("socket_test", 0, OS_SEM_MAX_VALUE);

    for (int i = 0; i < TEST_TASK_NUM; i++)
    {
        snprintf(task_name, OS_NAME_MAX, "socket_test%d", i);

        os_task_t *sock_task = os_task_create(task_name, socket_task, OS_NULL, 2048, 25);
        if (sock_task != OS_NULL)
        {
            os_task_startup(sock_task);
        }
    }

    for (int i = 0; i < TEST_TASK_NUM; i++)
    {
        os_err_t ret = os_sem_wait(test_sem, OS_WAIT_FOREVER);
        tp_assert_true(ret == OS_EOK);
    }
    
    os_sem_destroy(test_sem);

    return;
}

static void test_case(void)
{
    ATEST_UNIT_RUN(sock_muti_task_test);
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

ATEST_TC_EXPORT(components.net.molink.api.socket.tc, test_case, test_init, test_cleanup, TC_PRIORITY_MIDDLE);

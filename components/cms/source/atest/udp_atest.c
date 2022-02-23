/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        udp_atest.c
 *
 * @brief       This is udp test file based atest.
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "oneos_config.h"
#if defined(CMS_CONNECT_COAP) || defined(CMS_CONNECT_UDP)
#include "cms_con_udp.h"
#include <shell.h>
#include <atest.h>
#include <os_errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <os_task.h>
#include <os_clock.h>

static int        scode      = 303;
static void *     udp_handle = NULL;
static os_task_t *recv_task  = NULL;
#define UDP_RX_MAX_LENGTH 5

static float get_time_s(void)
{
    return ((float)os_tick_get() / OS_TICK_PER_SECOND);
}

static uint8_t recv_buff[UDP_RX_MAX_LENGTH];
static int     cms_udp_recv_once(uint32_t timeout)
{
    int rc = cms_udp_recv(udp_handle, recv_buff, sizeof(recv_buff), timeout);
    if (rc <= 0)
        return rc;
    printf("%s(time:%02f,len:%d)", __func__, get_time_s(), rc);
    for (int i = 0; i < rc; i++)
    {
        if (i % 0x10 == 0)
            printf("\r\n");
        printf("%02X ", recv_buff[i]);
    }
    printf("\r\n");
    return rc;
}

static void atest_cms_udp_recv(void)
{
    int rc = cms_udp_recv_once(10 * 1000);
    tp_assert_true(rc > 0);
}

static void atest_cms_udp_recv_process(void *param)
{
    while (cms_udp_get_state(udp_handle) == cms_con_state_connect)
    {
        os_task_msleep(100);
        cms_udp_recv_once(1);
    }
    printf("exit udp recv task!\r\n");
    recv_task = NULL;
}

static void atest_cms_udp_recv_task(void)
{
    if (recv_task != NULL)
    {
        os_task_destroy(recv_task);
        recv_task = NULL;
        return;
    }
    recv_task = os_task_create("udp_recv_test", atest_cms_udp_recv_process, NULL, 1024, 22);
    os_task_startup(recv_task);
}

static void atest_cms_udp_init(void)
{
    if (udp_handle != NULL)
        return;
    udp_handle = cms_udp_init(scode, UDP_RX_MAX_LENGTH);
    tp_assert_true(udp_handle != NULL);
}

static void atest_cms_udp_deinit(void)
{
    if (udp_handle == NULL)
        return;
    cms_udp_deinit(udp_handle);
    udp_handle = NULL;
}

static uint8_t buf[5] = {1, 2, 3, 4, 5};
static void    atest_cms_udp_send(void)
{
    // for (int i = 0; i < sizeof(buf); i++)
    // {
    //     buf[i] = i % 10 + '0';
    // }
    int rc = cms_udp_send(udp_handle, buf, sizeof(buf));
    tp_assert_true(rc == sizeof(buf));
}

static void cms_udp_all(void)
{
    ATEST_UNIT_RUN(atest_cms_udp_init);
    ATEST_UNIT_RUN(atest_cms_udp_send);
    ATEST_UNIT_RUN(atest_cms_udp_deinit);
}

ATEST_TC_EXPORT(cms.udp.all, cms_udp_all, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.udp.init, atest_cms_udp_init, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.udp.deinit, atest_cms_udp_deinit, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.udp.send, atest_cms_udp_send, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.udp.recv.once, atest_cms_udp_recv, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.udp.recv.task, atest_cms_udp_recv_task, NULL, NULL, TC_PRIORITY_LOW);
#endif

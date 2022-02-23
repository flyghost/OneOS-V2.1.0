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
 * @file        iperf.c
 *
 * @brief       The module network iperf tool implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <mo_api.h>
#include <shell.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "iperf"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_TOOLS_USING_IPERF

#define MOLINK_IPERF_BUFSZ (4096)

static os_bool_t gs_iperf_run = OS_FALSE;
static struct sockaddr_in gs_server_addr;

static void iperf_client_udp(void *param)
{
    mo_object_t *module = mo_get_default();
    if (OS_NULL == module)
    {
        ERROR("iperf start error!");
        return;
    }

    int socket = mo_socket(module, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket < 0)
    {
        return;
    }

    os_size_t send_size = MOLINK_IPERF_BUFSZ  > 1470 ? 1470 : MOLINK_IPERF_BUFSZ;

    os_uint32_t *send_buf = os_malloc(send_size);
    if (OS_NULL == send_buf)
    {
        ERROR("Alloc iperf buffer memory failed!");
        return;
    }

    os_size_t packet_count = 0;

    while (gs_iperf_run)
    {
        os_tick_t tick = os_tick_get();

        send_buf[0] = htonl(packet_count);
        send_buf[1] = htonl(tick / OS_TICK_PER_SECOND);
        send_buf[2] = htonl(tick % OS_TICK_PER_SECOND);

        int ret = mo_sendto(module,
                            socket, 
                            send_buf,
                            send_size,
                            0,
                            (struct sockaddr *)&gs_server_addr,
                            sizeof(struct sockaddr_in));
        if (ret < 0)
        {
            break;
        }

        packet_count++;
    }

    mo_closesocket(module, socket);

    os_free(send_buf);
}

static void iperf_client_tcp(void *param)
{
    mo_object_t *module = mo_get_default();
    if (OS_NULL == module)
    {
        ERROR("iperf start error!");
        return;
    }

    int socket = mo_socket(module, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket < 0)
    {
        return;
    }

    int ret = mo_connect(module, socket, (const struct sockaddr *)&gs_server_addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        mo_closesocket(module, socket);
        return;
    }

    os_uint8_t *send_buf = (os_uint8_t *)os_malloc(MOLINK_IPERF_BUFSZ);
    if (OS_NULL == send_buf)
    {
        ERROR("Alloc iperf buffer memory failed!");
        return;
    }

    for (int i = 0; i < MOLINK_IPERF_BUFSZ; i++)
    {
        send_buf[i] = i & 0xff;
    }

    os_size_t sent_len   = 0;
    os_tick_t tick_begin = os_tick_get();
    os_tick_t tick_prev     = tick_begin;

    while (gs_iperf_run)
    {
        os_tick_t tick_curr = os_tick_get();
        if (tick_curr - tick_prev >= 5 * OS_TICK_PER_SECOND)
        {
            float speed;

            speed = (float)(sent_len * OS_TICK_PER_SECOND / 125 / (tick_curr - tick_prev));
            speed /= 1000.0f;

            printf("%.4f Mbps!\r\n", speed);

            if (tick_curr - tick_begin > 60 * OS_TICK_PER_SECOND)
            {
                gs_iperf_run = OS_FALSE;
                break;
            }

            tick_prev = tick_curr;
            sent_len  = 0;
        }

        ret = mo_send(module, socket, send_buf, MOLINK_IPERF_BUFSZ, 0);
        if (ret < 0)
        {
            gs_iperf_run = OS_FALSE;
            break;
        }

        sent_len += ret;
    }

    mo_closesocket(module, socket);

    os_free(send_buf);
}

void iperf_usage(void)
{
    os_kprintf("Usage: iperf [-u] -c host -p port\r\n");
    os_kprintf("       iperf [-h|--stop]\r\n");
    os_kprintf("\r\n");
    os_kprintf("Client:\r\n");
    os_kprintf("  -u           testing UDP protocol\r\n");
    os_kprintf("  -c <host>    run in client mode, connecting to <host>\r\n");
    os_kprintf("  -p #         server port to connect to\r\n");
    os_kprintf("\r\n");
    os_kprintf("Miscellaneous:\r\n");
    os_kprintf("  -h           print this message and quit\r\n");
    os_kprintf("  --stop       stop iperf program\r\n");
    return;
}

int molink_iperf(int argc, char const *argv[])
{
    os_size_t index   = 1;
    os_bool_t ues_udp = OS_FALSE;

    if (1 == argc || strcmp(argv[index], "-h") == 0)
    {
        /* iperf or iperf -h */
        iperf_usage();
        return 0;
    }
    
    if (strcmp(argv[index], "-u") == 0)
    {
        /* iperf -u [] */
        index   = 2;
        ues_udp = OS_TRUE;
    }
    else if (strcmp(argv[index], "--stop") == 0)
    {
        gs_iperf_run = OS_FALSE;
        return 0;
    }
    
    if (strcmp(argv[index], "-c") == 0 && strcmp(argv[index + 2], "-p") == 0)
    {
        if (argc < 5 || argc > 6)
        {
            iperf_usage();
            return 0;
        }

        gs_iperf_run = OS_TRUE;

        gs_server_addr.sin_family      = PF_INET;
        gs_server_addr.sin_addr.s_addr = inet_addr(argv[index + 1]);
        gs_server_addr.sin_port        = htons(atoi(argv[index + 3]));

        os_task_t *iperf_task = OS_NULL;

        if (ues_udp)
        {
            iperf_task = os_task_create("iperf", iperf_client_udp, OS_NULL, 2048, 10);
        }
        else
        {
            iperf_task = os_task_create("iperf", iperf_client_tcp, OS_NULL, 2048, 10);
        }

        if (iperf_task != OS_NULL)
        {
            os_task_startup(iperf_task);
        }
    }
    else
    {
        iperf_usage();
    }

    return 0;
}

SH_CMD_EXPORT(iperf, molink_iperf, "molink iperf test");

#endif /* MOLINK_TOOLS_USING_IPERF */

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
 * @file        socket_compare.c
 *
 * @brief       This is mqtt test file based atest.
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "oneos_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netdb.h>

#define BUFF_LENGTH 128

static int     fd = -1;
static uint8_t send_buf[BUFF_LENGTH + 1];
static uint8_t recv_buf[BUFF_LENGTH] = {0};

#if defined(CMS_CONNECT_MQTT) || defined(CMS_CONNECT_TCP)
static void atest_connect(void)
{
    int              rc;
    struct addrinfo  addr_info;
    struct addrinfo *addr_list;
    struct addrinfo *addr;

    memset(&addr_info, 0, sizeof(struct addrinfo));
    addr_info.ai_socktype = SOCK_STREAM;
    addr_info.ai_protocol = IPPROTO_TCP;

    rc = getaddrinfo("121.89.166.244", "6588", &addr_info, &addr_list);
    if (rc != 0)
    {
        printf("dns resolution failed\r\n");
        return;
    }

    for (addr = addr_list; addr != NULL; addr = addr->ai_next)
    {
        fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (fd < 0)
        {
            continue;
        }
        if ((rc = connect(fd, addr->ai_addr, addr->ai_addrlen)) == 0)
        {
            break;
        }
        closesocket(fd);
        fd = -1;
    }
    freeaddrinfo(addr_list);

    if (fd < 0)
    {
        printf("connect failed\r\n");
    }
    return;
}

static void atest_disconnect(void)
{
    if (fd < 0)
        return;
    closesocket(fd);
    fd = -1;
}
static void atest_send(void)
{
    int            rc;
    struct timeval interval = {5000 / 1000, (5000 % 1000) * 1000};

    if (fd < 0)
    {
        printf("not connect\r\n");
        return;
    }
    for (int i = 0; i < BUFF_LENGTH; i++)
    {
        send_buf[i] = '0' + i % 0x10;
    }
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&interval, sizeof(struct timeval));
    rc = send(fd, send_buf, BUFF_LENGTH, 0);
    if (!rc)
    {
        atest_disconnect();
        printf("The other end is closed!\r\n");
    }
    if (rc < 0)
    {
        atest_disconnect();
        printf("socket error(%d)!\r\n", rc);
    }
    printf("send success(%d)!\r\n", rc);
    return;
}
static void atest_recv(void)
{
    int            rc;
    int            socket_fd;
    struct timeval interval;
    fd_set         read_fds;

    if (fd < 0)
    {
        printf("not connect\r\n");
        return;
    }
    socket_fd = fd;

    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);

    interval.tv_sec  = 5000 / 1000;
    interval.tv_usec = (5000 % 1000) * 1000;

    rc = select(socket_fd + 1, &read_fds, NULL, NULL, &interval);
    if (!rc)
    {
        printf("time out\r\n");
        return;
    }
    if (rc < 0)
    {
        atest_disconnect();
        printf("socket error(%d)!\r\n", rc);
        return;
    }

    rc = recv(socket_fd, recv_buf, BUFF_LENGTH, 0);
    if (rc <= 0)
    {
        if (rc)
            printf("recv error(%d)!\r\n", rc);
        else
            printf("The other end is closed!\r\n");
        return;
    }
    printf("recv(%d):%s\r\n", rc, recv_buf);
    return;
}

#else
static struct sockaddr to_addr;

static void atest_disconnect(void)
{
    if (fd < 0)
        return;
    closesocket(fd);
    fd = -1;
}

static void atest_connect(void)
{
    int              rc;
    struct addrinfo  addr_info;
    struct addrinfo *addr_list;
    struct addrinfo *addr;

    memset(&addr_info, 0, sizeof(struct addrinfo));
    addr_info.ai_socktype = SOCK_DGRAM;
    addr_info.ai_protocol = IPPROTO_UDP;

    rc = getaddrinfo("121.89.166.244", "6589", &addr_info, &addr_list);
    if (rc != 0)
    {
        printf("dns resolution failed\r\n");
        return;
    }

    for (addr = addr_list; addr != NULL; addr = addr->ai_next)
    {
        fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (fd < 0)
        {
            continue;
        }
        to_addr = *addr->ai_addr;
        break;
    }
    freeaddrinfo(addr_list);
    if (fd < 0)
    {
        printf("connect failed\r\n");
        return;
    }
}

static void atest_send(void)
{
    int            rc;
    struct timeval interval = {5000 / 1000, (5000 % 1000) * 1000};

    if (fd < 0)
    {
        printf("not connect\r\n");
        return;
    }
    for (int i = 0; i < BUFF_LENGTH; i++)
    {
        send_buf[i] = '0' + i % 0x10;
    }
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&interval, sizeof(struct timeval));
    rc = sendto(fd, send_buf, BUFF_LENGTH, 0, &to_addr, sizeof(struct sockaddr));
    printf("udp send %d\r\n", rc);
}

static void atest_recv(void)
{
    int            rc;
    int            socket_fd;
    struct timeval interval;
    fd_set         read_fds;

    if (fd < 0)
    {
        printf("not connect\r\n");
        return;
    }
    socket_fd = fd;

    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);

    interval.tv_sec  = 5000 / 1000;
    interval.tv_usec = (5000 % 1000) * 1000;

    rc = select(socket_fd + 1, &read_fds, NULL, NULL, &interval);
    if (rc == 0)
    {
        printf("time out\r\n");
        return;
    }
    if (rc < 0)
    {
        atest_disconnect();
        printf("socket error(%d)!\r\n", rc);
        return;
    }
    uint32_t fromlen = sizeof(to_addr);
    rc               = recvfrom(socket_fd, recv_buf, BUFF_LENGTH, 0, &to_addr, &fromlen);
    printf("recv(%d):%s\r\n", rc, recv_buf);
    return;
}

#endif

#include <shell.h>
#include <atest.h>

ATEST_TC_EXPORT(socket.connect, atest_connect, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(socket.disconnect, atest_disconnect, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(socket.send, atest_send, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(socket.recv, atest_recv, NULL, NULL, TC_PRIORITY_LOW);


/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * Copyright (c) 2006-2018 RT-Thread Development Team.
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
 * @file        hal_tcp.c
 *
 * @brief      a port file of tcp for iotkit
 *
 * @details     
 *
 * @revision
 * Date               Author             Notes
 * 2019-07-21         MurphyZhao         first edit
 * 2020-06-10         OneOS Team         format and change request resource
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <sys/time.h>
#include "netinet/tcp.h"
#include "netdb.h"
#include "infra_types.h"
#define DBG_EXT_TAG "ali.tcp"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"

extern void     HAL_Printf(const char *fmt, ...);
extern uint64_t HAL_UptimeMs(void);

static uint64_t time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now)
    {
        t_left = t_end - t_now;
    }
    else
    {
        t_left = 0;
    }

    return t_left;
}

uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    struct addrinfo  hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur          = NULL;
    int              fd           = 0;
    int              rc           = 0;
    char             service[6];

    memset(&hints, 0, sizeof(hints));

    LOG_D(DBG_EXT_TAG, "establish tcp connection with server(host=%s port=%d)", host, port);

    hints.ai_family   = AF_INET; /* only IPv4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0)
    {
        LOG_E(DBG_EXT_TAG, "getaddrinfo error");
        return -1;
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next)
    {
        if (cur->ai_family != AF_INET)
        {
            LOG_E(DBG_EXT_TAG, "socket type error");
            rc = -1;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0)
        {
            LOG_E(DBG_EXT_TAG, "create socket error");
            rc = -1;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            rc = fd;
            break;
        }

        closesocket(fd);
        LOG_E(DBG_EXT_TAG, "connect error");
        rc = -1;
    }

    if (-1 == rc)
    {
        LOG_E(DBG_EXT_TAG, "fail to establish tcp");
    }
    else
    {
        LOG_D(DBG_EXT_TAG, "success to establish tcp, fd=%d", rc);
    }
    freeaddrinfo(addrInfoList);

    return (uintptr_t)rc;
}

int32_t HAL_TCP_Destroy(uintptr_t fd)
{
    int rc;

    /* Shutdown both send and receive operations. */
    rc = shutdown((int)fd, 2);
    if (0 != rc)
    {
        LOG_E(DBG_EXT_TAG, "shutdown error");
        return -1;
    }

    rc = closesocket((int)fd);
    if (0 != rc)
    {
        LOG_E(DBG_EXT_TAG, "closesocket error");
        return -1;
    }

    return 0;
}

int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int      ret;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set   sets;

    t_end    = HAL_UptimeMs() + timeout_ms;
    len_sent = 0;
    ret      = 1; /* send one time if timeout_ms is value 0 */

    do
    {
        t_left = time_left(t_end, HAL_UptimeMs());

        if (0 != t_left)
        {
            struct timeval timeout;

            FD_ZERO(&sets);
            FD_SET(fd, &sets);

            timeout.tv_sec  = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            ret = select(fd + 1, NULL, &sets, NULL, &timeout);
            if (ret > 0)
            {
                if (0 == FD_ISSET(fd, &sets))
                {
                    LOG_E(DBG_EXT_TAG, "Should NOT arrive");
                    /* If timeout in next loop, it will not sent any data */
                    ret = 0;
                    continue;
                }
            }
            else if (0 == ret)
            {
                LOG_E(DBG_EXT_TAG, "select-write timeout %d", timeout_ms);
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    LOG_E(DBG_EXT_TAG, "EINTR be caught");
                    continue;
                }

                LOG_E(DBG_EXT_TAG, "select-write fail");
                break;
            }
        }

        if (ret > 0)
        {
            ret = send(fd, buf + len_sent, len - len_sent, 0);
            if (ret > 0)
            {
                len_sent += ret;
            }
            else if (0 == ret)
            {
                LOG_E(DBG_EXT_TAG, "No data be sent");
            }
            else
            {
                if (EINTR == errno)
                {
                    LOG_E(DBG_EXT_TAG, "EINTR be caught");
                    continue;
                }

                LOG_E(DBG_EXT_TAG, "send fail");
                break;
            }
        }
    } while ((len_sent < len) && (time_left(t_end, HAL_UptimeMs()) > 0));

    return len_sent;
}

int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int            ret, err_code;
    uint32_t       len_recv;
    uint64_t       t_end, t_left;
    fd_set         sets;
    struct timeval timeout;

    t_end    = HAL_UptimeMs() + timeout_ms;
    len_recv = 0;
    err_code = 0;

    do
    {
        t_left = time_left(t_end, HAL_UptimeMs());
        if (0 == t_left)
        {
            break;
        }
        FD_ZERO(&sets);
        FD_SET(fd, &sets);

        timeout.tv_sec  = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(fd + 1, &sets, NULL, NULL, &timeout);
        if (ret > 0)
        {
            ret = recv(fd, buf + len_recv, len - len_recv, 0);
            if (ret > 0)
            {
                len_recv += ret;
            }
            else if (0 == ret)
            {
                LOG_E(DBG_EXT_TAG, "connection is closed");
                err_code = -1;
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    LOG_E(DBG_EXT_TAG, "EINTR be caught");
                    continue;
                }
                LOG_E(DBG_EXT_TAG, "recv fail");
                err_code = -2;
                break;
            }
        }
        else if (0 == ret)
        {
            break;
        }
        else
        {
            LOG_E(DBG_EXT_TAG, "select-recv fail");
            err_code = -2;
            break;
        }
    } while ((len_recv < len));

    /* priority to return data bytes if any data be received from TCP connection. */
    /* It will get error code on next calling */
    return (0 != len_recv) ? len_recv : err_code;
}

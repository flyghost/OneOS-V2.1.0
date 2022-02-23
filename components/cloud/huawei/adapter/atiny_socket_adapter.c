/*
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
 * @file        atiny_socket_adapter.c
 *
 * @brief       huawei cloud sdk file "atiny_socket.c" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/select.h>
#include "sal/atiny_socket.h"
#include "osdepends/atiny_osdep.h"

#if defined(SOCKET_DEBUG)
#define SOCKET_LOG(fmt, ...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        (void)atiny_printf("[SOCKET][%s:%d] " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);                      \
    } while (0)
#else
#define SOCKET_LOG(fmt, ...) ((void)0)
#endif

typedef struct
{
    int fd;
} atiny_net_context;

typedef struct _atiny_net_info_
{
    struct _atiny_net_info_ *next;
    char                     protocol;
    int                      fd;
    struct sockaddr          addr;
} atiny_net_info;

static atiny_net_info *g_atiny_net_info = NULL;

void atiny_net_deinit(void)
{
    atiny_net_info *info;
    while (g_atiny_net_info != NULL)
    {
        info = g_atiny_net_info->next;
        atiny_free(g_atiny_net_info);
        g_atiny_net_info = info;
    }
}
static void *add_atiny_net_info_list(const atiny_net_info *info)
{
    atiny_net_info *insert_info = (atiny_net_info *)atiny_malloc(sizeof(atiny_net_info));

    if (info == NULL || insert_info == NULL)
        return NULL;

    memcpy(insert_info, info, sizeof(atiny_net_info));
    insert_info->next = NULL;

    if (g_atiny_net_info == NULL)
    {
        g_atiny_net_info = insert_info;
        return g_atiny_net_info;
    }

    atiny_net_info *next_info = g_atiny_net_info;
    while (next_info->next != NULL)
    {
        next_info = next_info->next;
    }
    next_info->next = insert_info;
    return g_atiny_net_info;
}

static void *query_atiny_net_info_list(int fd)
{
    atiny_net_info *next_info = g_atiny_net_info;

    while (next_info != NULL)
    {
        if (next_info->fd == fd)
            return next_info;
        next_info = next_info->next;
    }
    return NULL;
}

static void del_atiny_net_info_list(int fd)
{

    if (g_atiny_net_info == NULL)
    {
        return;
    }
    else if (g_atiny_net_info->fd == fd)
    {
        atiny_free(g_atiny_net_info);
        g_atiny_net_info = NULL;
        return;
    }

    atiny_net_info *pre_info  = g_atiny_net_info;
    atiny_net_info *next_info = pre_info->next;

    while (pre_info != NULL && next_info != NULL)
    {
        if (next_info->fd == fd)
            goto del;
        pre_info  = next_info;
        next_info = pre_info->next;
    }
    return;
del:
    pre_info->next = next_info->next;
    atiny_free(next_info);
    return;
}

int atiny_net_accept(void *bind_ctx, void *client_ctx, void *client_ip, size_t buf_size, size_t *ip_len)
{
    return 0;
}

void *atiny_net_bind(const char *host, const char *port, int proto)
{
    return NULL;
}
#if !defined(WITH_DTLS) && defined(SOCKET_DEBUG)
static int coap_option_print(const unsigned char *start, int *last_option, int max_len)
{
    const char *option_name[] = {
        "NULL",          "IF_MATCH", "NULL", "URI_HOST",  "ETAG",           "IF_NONE_MATCH", "OBSERVE", "URI_PORT",
        "LOCATION_PATH", "NULL",     "NULL", "URI_PATH",  "CONTENT_TYPE",   "NULL",          "MAX_AGE", "URI_QUERY",
        "NULL",          "ACCEPT",   "NULL", "TOKEN",     "LOCATION_QUERY", "NULL",          "NULL",    "BLOCK2",
        "NULL",          "NULL",     "NULL", "BLOCK1",    "SIZE",           "NULL",          "NULL",    "NULL",
        "NULL",          "NULL",     "NULL", "PROXY_URI", "NULL",           "NULL",          "NULL",    "NULL"};
    volatile int cur_option;
    cur_option = (*last_option) + (start[0] >> 4);
    if (cur_option >= 39 || !strcmp(option_name[cur_option], "NULL"))
    {
        atiny_printf("CoAP option error!\r\n");
        return -1;
    }
    atiny_printf("%s: ", option_name[cur_option]);

    int data_len, ret = 0;
    if ((start[0] & 0xF) < 13)
    {
        data_len = start[0] & 0xF;
        ++ret;
    }
    else if ((start[0] & 0xF) == 13)
    {
        data_len = start[1] + 13;
        ret += 2;
    }
    else if ((start[0] & 0xF) == 14)
    {
        data_len = (((int)start[1] << 8) | start[2]) + 269;
        ret += 3;
    }
    else
    {
        atiny_printf("option length error!\r\n");
        return -1;
    }
    data_len += ret;    // whole option length

    switch (cur_option)
    {
    case 3:     // COAP_OPTION_URI_HOST:
    case 11:    // COAP_OPTION_URI_PATH:
    case 15:    // COAP_OPTION_URI_QUERY:
    case 8:     // COAP_OPTION_LOCATION_PATH
    case 20:    // COAP_OPTION_LOCATION_QUERY
    case 35:    // COAP_OPTION_PROXY_URI
        for (; ret < data_len && ret < max_len; ret++)
        {
            atiny_printf("%c", start[ret]);
        }
        break;

    case 7:    // COAP_OPTION_URI_PORT:
    default:
        for (; ret < data_len && ret < max_len; ret++)
        {
            atiny_printf(" %02X", start[ret]);
        }
        break;
    }
    atiny_printf("\r\n");
    *last_option = cur_option;
    return ret;
}
#endif

static void send_or_rec_buf_print(const char *direction, const unsigned char *buf, int len)
{
#if !defined(SOCKET_DEBUG)
    return;
#elif defined(WITH_DTLS)
    atiny_printf("-------------------(%.02f)---------------------\r\n", (float)atiny_gettime_ms() / 1000);
    switch (*direction)
    {
    case 's':
        atiny_printf("send:\r\n");
        break;

    case 'r':
        atiny_printf("rec:\r\n");
        break;

    default:
        atiny_printf("input error!!!\r\n");
        return;
    }
    for (int i = 0; i < len; i++)
    {
        atiny_printf("%02X ", buf[i]);
    }
    atiny_printf("\r\n");
#else
    const char *type_name[]     = {"Confirmable", "Non-confirmable", "ACK", "Reset"};
    const char *code_name_int[] = {"Request", "", "Success", "", "Error(clinet)", "Error(server)"};
    const char *code_name_dec[] = {"empty", "get", "post", "put", "delete"};
    int         index           = 0;
    atiny_printf("ver:%d\r\ntype:%s\r\n", buf[0] >> 6, type_name[(buf[0] >> 4) & 0x3]);
    atiny_printf("code:%s", code_name_int[buf[1] >> 5]);
    if (!(buf[1] >> 5))
        atiny_printf("--%s", code_name_dec[buf[1] & 0x1F]);
    atiny_printf("\r\nmid:%02X%02X\r\ntoken:", buf[2], buf[3]);
    for (index = 4; index < 4 + (buf[0] & 0xF); index++)
    {
        atiny_printf("%02X", buf[index]);
    }
    atiny_printf("\r\n");

    int last_option = 0;
    while (index < len && buf[index] != 0xFF)
    {
        int ret = coap_option_print(&buf[index], &last_option, len - index);
        if (ret < 0)
            return;
        index += ret;
    }

    if (index >= len)
        return;

    index++;    // 0xFF
    atiny_printf("payload:");
    for (; index < len; index++)
    {
        atiny_printf(" %02x", buf[index]);
    }
    atiny_printf("\r\n");
    if (len >= 200)
        atiny_printf("!");
    return;
#endif
}
void *atiny_net_connect(const char *host, const char *port, int proto)
{
    atiny_net_context *ctx = NULL;
#if defined(SOCKET_DEBUG)
    int ret;
#endif
    struct addrinfo  hints;
    struct addrinfo *addr_list;
    struct addrinfo *cur;

    if (NULL == port || (proto != ATINY_PROTO_UDP && proto != ATINY_PROTO_TCP))
    {
        SOCKET_LOG("ilegal incoming parameters,(%p,%p,%d)", host, port, proto);
        return NULL;
    }
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = proto == ATINY_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == ATINY_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    SOCKET_LOG("try to do name resolution now...");
#if defined(SOCKET_DEBUG)
    if ((ret = getaddrinfo(host, port, &hints, &addr_list)) != 0)
    {
        SOCKET_LOG("getaddrinfo failed: 0x%x", ret);
#else
    if (getaddrinfo(host, port, &hints, &addr_list) != 0)
    {
#endif
        return NULL;
    }

    SOCKET_LOG("do name resolution succeed");

    ctx = atiny_malloc(sizeof(atiny_net_context));

    if (NULL == ctx)
    {
        SOCKET_LOG("malloc failed for socket context");
        freeaddrinfo(addr_list);
        return NULL;
    }

    ctx->fd = -1;

    /* Try the sockaddrs until a connection succeeds */
    for (cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        ctx->fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);

        if (ctx->fd < 0)
        {
            continue;
        }

        if (proto == ATINY_PROTO_UDP)
        {
            atiny_net_info info;
            info.fd       = ctx->fd;
            info.addr     = *(cur->ai_addr);
            info.protocol = proto;
            add_atiny_net_info_list(&info);
            break;
        }

        if (connect(ctx->fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            break;
        }

        closesocket(ctx->fd);
        ctx->fd = -1;
    }

    freeaddrinfo(addr_list);

    if (ctx->fd < 0)
    {
        SOCKET_LOG("unkown host or port");
        atiny_free(ctx);
        return NULL;
    }

    if (proto == ATINY_PROTO_UDP)
    {
        SOCKET_LOG("UDP create socket and bind to server finished");
    }
    else /* proto == ATINY_PROTO_TCP */
    {
        SOCKET_LOG("TCP connect to server succeed");
    }
    return ctx;
}

int atiny_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int ret = -1;
    int fd  = ((atiny_net_context *)ctx)->fd;

    if (fd < 0)
    {
        SOCKET_LOG("ilegal socket(%d)", fd);
        return ATINY_NET_ERR;
    }

    atiny_net_info *info = query_atiny_net_info_list(fd);
    if (info == NULL)
    {
        ret = recv(fd, buf, len, 0);
    }
    else
    {
        uint32_t fromlen = sizeof(info->addr);
        ret              = recvfrom(fd, buf, len, 0, &info->addr, &fromlen);
    }
    if (ret <= 0)
    {
        SOCKET_LOG("socket was closed by peer");
        return -1;
    }
    send_or_rec_buf_print("rec", buf, ret);
    return ret;
}

int atiny_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int ret = -1;
    int fd  = ((atiny_net_context *)ctx)->fd;

    if (fd < 0)
    {
        SOCKET_LOG("ilegal socket(%d)", fd);
        return ATINY_NET_ERR;
    }
    send_or_rec_buf_print("send", buf, len);

    atiny_net_info *info = query_atiny_net_info_list(fd);
    if (info == NULL)
    {
        ret = send(fd, buf, len, 0);
    }
    else
    {
        ret = sendto(fd, buf, len, 0, &info->addr, sizeof(info->addr));
    }
    if (ret < 0)
    {
        SOCKET_LOG("error accured when send: 0x%x", errno);
        return -1;
    }

    return ret;
}

int atiny_net_recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    int            ret = -1;
    struct timeval tv;
    fd_set         read_fds;
    int            fd = ((atiny_net_context *)ctx)->fd;

    if (fd < 0)
    {
        SOCKET_LOG("ilegal socket(%d)", fd);
        return ATINY_NET_ERR;
    }
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
    if (ret == 0)
    {
        return ATINY_NET_TIMEOUT;
    }
    if (ret < 0)
    {
        SOCKET_LOG("select error ret=%d,err 0x%x", ret, errno);
        return ATINY_NET_ERR;
    }
    return atiny_net_recv(ctx, buf, len);
}

int atiny_net_send_timeout(void *ctx, const unsigned char *buf, size_t len, uint32_t timeout)
{
    int            fd;
    struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};

    fd = ((atiny_net_context *)ctx)->fd;
    if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
    {
        interval.tv_sec  = 0;
        interval.tv_usec = 100;
    }

    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&interval, sizeof(struct timeval));
    int ret = send(fd, buf, len, 0);
    send_or_rec_buf_print("send", buf, ret);
    return ret;
}

void atiny_net_close(void *ctx)
{
    if (ctx == NULL)
    {
        SOCKET_LOG("ilegal socket");
        return;
    }
    int fd = ((atiny_net_context *)ctx)->fd;

    if (fd < 0)
    {
        SOCKET_LOG("ilegal socket(%d)", fd);
        return;
    }

    atiny_net_info *info = query_atiny_net_info_list(fd);
    if (info != NULL)
    {
        del_atiny_net_info_list(fd);
    }
    closesocket(fd);
    atiny_free(ctx);
}

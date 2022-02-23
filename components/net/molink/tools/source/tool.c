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
 * @file        tool.c
 *
 * @brief       The module network debug functions implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <os_task.h>
#include <tool.h>

#ifdef OS_USING_SHELL
#include <shell.h>

#define MO_LOG_TAG "tool"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef NET_USING_MOLINK
#include <mo_api.h>

#ifdef MOLINK_TOOLS_USING_IFCONFIG

/**
 ***********************************************************************************************************************
 * @brief           Show module information
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
void module_show_info(void)
{
    mo_object_t  *defmo_obj = mo_get_default();
    if (defmo_obj == OS_NULL)
    {
        os_kprintf("ifconfig: get default molink module failed, module is not create.\r\n");
        return;
    }

    if (mo_ifconfig(defmo_obj) != OS_EOK)
    {
        os_kprintf("ifconfig: get default molink module information failed.\r\n");
    }

    return;
}

static int module_cmd_ifconfig(int argc, char **argv)
{
    if (argc != 1)
    {
        os_kprintf("Input errror, please input: ifconfig\r\n");
    }
    else
    {
        module_show_info();
    }

    return 0;
}
SH_CMD_EXPORT(ifconfig, module_cmd_ifconfig, "List the information of module");
#endif /* MOLINK_TOOLS_USING_IFCONFIG */

#if defined(MOLINK_TOOLS_USING_PING) && defined(MOLINK_USING_IFCONFIG_OPS)
#define MODULE_PING_DATA_SIZE  (64)
#define MODULE_PING_TIMES      (4)
#define MODULE_PING_RECV_TIMEO (5 * 1000) /* ping recv timeout - in milliseconds */

/**
 ***********************************************************************************************************************
 * @brief           Module ping network host address and show the result information
 *
 * @param[in]       target_name      ping network host address
 * @param[in]       times            ping packets number
 * @param[in]       size             ping packets size
 * @param[in]       timeout          ping timeout configuration, in seconds
 *
 * @return          void
 ***********************************************************************************************************************
 */
void module_ping(char *target_name, os_uint32_t times, os_uint16_t size, os_uint32_t timeout)
{
    struct ping_resp resp;

    mo_object_t *defmo_obj = OS_NULL;
    os_err_t    ret        = OS_EOK;
    os_uint16_t index      = 0;
    os_uint32_t maxtime    = 0;
    os_uint32_t sumtime    = 0;
    os_uint32_t mintime    = 0xFFFFFFFF;
    os_uint16_t recv_cnt   = 0;
    os_bool_t   ping_flag  = OS_FALSE;
    char        ip[16]     = {0};

    defmo_obj = mo_get_default();
    if (defmo_obj == OS_NULL)
    {
        os_kprintf("Ping: get defmo obj failed, module is not create.\r\n");
        return;
    }

    if (times == 0)
    {
        times = MODULE_PING_TIMES;
    }

    if (size == 0)
    {
        size = MODULE_PING_DATA_SIZE;
    }

    if (timeout == 0)
    {
        timeout = MODULE_PING_RECV_TIMEO;
    }
    else
    {
        /* For most of modules timeout parameter are in milliseconds */
        timeout *= 1000;
    }

    /* Step 1: get module intf address, if it failed to get address that means module is not ready for ping */
    if (mo_get_ipaddr(defmo_obj, ip) != OS_EOK)
    {
        os_kprintf("Ping: get module intf address failed, module is not ready for ping\r\n");
        return;
    }

    /* Step 2: call the ping API of the module */
    for (index = 0; index < times; index++)
    {
        memset(&resp, 0x00, sizeof(struct ping_resp));
        ret = mo_ping(defmo_obj, target_name, size, timeout, &resp);

        /* Parse ping ret and show to user */
        if (ret == OS_ETIMEOUT)
        {
            os_kprintf("[%d] Ping: from %s icmp_seq=%d timeout\r\n",
                       index,
                       (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr),
                       index);
        }
        else if (ret == OS_EOK)
        {
            if (resp.ttl != 0)
            {
                os_kprintf("[%d] %d bytes from %s icmp_seq=%d ttl=%d time=%d ms\r\n",
                           index,
                           resp.data_len,
                           (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr),
                           index,
                           resp.ttl,
                           resp.time);
            }
            else
            {
                os_kprintf("[%d] %d bytes from %s icmp_seq=%d time=%d ms\r\n",
                           index,
                           resp.data_len,
                           (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr),
                           index,
                           resp.time);
            }

            recv_cnt++;
            sumtime += resp.time;

            if ((resp.time > 0) && (resp.time < mintime))
            {
                mintime = resp.time;
            }

            if ((resp.time > 0) && (resp.time > maxtime))
            {
                maxtime = resp.time;
            }

            if (ping_flag == OS_FALSE)
            {
                ping_flag = OS_TRUE;
            }
        }
        else
        {
            os_kprintf("[%d] Ping: %s %s failed\r\n",
                       index,
                       (ip_addr_isany(&(resp.ip_addr))) ? "host" : "address",
                       (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr));
        }

        /* Delay 1000ms to continue */
        os_task_msleep(1000);
    }

    os_kprintf("\r\nPing statistics for %s :\r\n",
              (ip_addr_isany(&(resp.ip_addr))) ? target_name : inet_ntoa(resp.ip_addr));
    if (ping_flag == OS_TRUE)
    {
        os_kprintf("Packets: Sent = %d, Received = %d, Lost = %d, Mintime = %u ms, Maxtime = %u ms, Avgtime = %u ms\r\n",
                    times,
                    recv_cnt,
                    (times - recv_cnt),
                    mintime,
                    maxtime,
                    sumtime / recv_cnt);
    }
    else
    {
        os_kprintf("Packets: Sent = %d, Received = %d, Lost = %d\r\n", times, recv_cnt, (times - recv_cnt));
    }

    return;
}

/* Note that:  uint32: 0~4294967295 */
static os_uint32_t strnum_to_uint(char *strnum)
{
    os_uint32_t  ret  = 0;
    char        *ptr  = OS_NULL;

    ptr = strnum;
    while(((*ptr) >= '0') && ((*ptr) <= '9'))
    {
        ret *=10;
        ret += *ptr - '0';
        ptr++;
    }

    return ret;
}

static int module_cmd_ping(int argc, char **argv)
{
    os_uint32_t times    = 0;
    os_uint16_t size     = 0;
    os_uint32_t timeout  = 0;
    os_err_t    ret      = OS_EOK;

    switch (argc)
    {
    case 1:
    {
        /* Input: ping */
        os_kprintf("Please input: ping <host address> <times[1,%d]> <pkg_size> <timeout(sec)>\r\n",
                   MO_PING_MAX_TIMES_CONFIG);
        ret = OS_ERROR;
        break;
    }

    case 2:
    {
        /* Input: ping <host address> */
        break;
    }

    case 3:
    {
        /* Input: ping <host address> <times> */
        times = strnum_to_uint(argv[2]);
        break;
    }

    case 4:
    {
        /* Input: ping <host address> <times> <pkg_siz> */
        times = strnum_to_uint(argv[2]);
        size  = strnum_to_uint(argv[3]);
        break;
    }

    case 5:
    {
        /* Input: ping <host address> <times> <pkg_siz> <timeout(sec)> */
        times    = strnum_to_uint(argv[2]);
        size     = strnum_to_uint(argv[3]);
        timeout  = strnum_to_uint(argv[4]);
        break;
    }

    default:
        os_kprintf("Input error, please input: ping <host address> <times[1,%d]> <pkg_size> <timeout(sec)>\r\n",
                   MO_PING_MAX_TIMES_CONFIG);
        ret = OS_ERROR;
        break;
    }

    /* The ping action cannot be terminated manually. Limit the number of pings */
    if (times > MO_PING_MAX_TIMES_CONFIG)
    {
        os_kprintf("Input error, ping times [%d] is out of range[1,%d]\r\n", times, MO_PING_MAX_TIMES_CONFIG);
        ret = OS_ERROR;
    }

    if (ret != OS_ERROR)
    {
        module_ping(argv[1], times, size, timeout);
    }

    return ret;
}
SH_CMD_EXPORT(ping, module_cmd_ping, "Ping module network host: ping <host address> <times> <pkg_size> <timeout(sec)>");
#endif /* defined(MOLINK_TOOLS_USING_PING) && defined(MOLINK_USING_NETSERV_OPS) */


#if defined(MOLINK_TOOLS_USING_SOCKETSTAT) && defined(MOLINK_USING_NETCONN_OPS)
static void socket_status_convert_to_char_info(os_uint8_t stat, char status[])
{
    switch(stat)
    {
    case NETCONN_STAT_NULL:
        strcpy(status, "Not create");
        break;

    case NETCONN_STAT_INIT:
        strcpy(status, "Not connect");
        break;

    case NETCONN_STAT_CONNECT:
        strcpy(status, "Connect OK");
        break;

    case NETCONN_STAT_CLOSE:
        strcpy(status, "Closed");
        break;

    default:
        os_kprintf("Convert fail: socket status[0x%02x] unknown\r\n", stat);
        strcpy(status, "Unknown");
        break;
    }

    return;
}

static void socket_type_convert_to_char_info(os_uint8_t type, char type_info[])
{
    switch(type)
    {
    case NETCONN_TYPE_TCP:
        strcpy(type_info, "TCP");
        break;

    case NETCONN_TYPE_UDP:
        strcpy(type_info, "UDP");
        break;

    default:
        strcpy(type_info, "Unknown");
        break;
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Show module socket status information
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
void module_show_socket_stat(void)
{
    mo_object_t      *defmo_obj       = OS_NULL;
    os_uint8_t        i               = 0;
    char              socket_type[8]  = {0};
    char              socket_stat[32] = {0};
    char              ipaddr[32]      = {0};
    mo_netconn_info_t netconn_info    = {0};

    /* Get module intf obj, if it failed to get address that means module sockets are not ready */
    defmo_obj = mo_get_default();
    if (OS_NULL == defmo_obj)
    {
        os_kprintf("Show socket status: get defmo obj failed, module is not create.\r\n");
        return;
    }

    /* Get module intf address, if it failed to get address that means module is not ready for ping */
    if (OS_EOK != mo_get_ipaddr(defmo_obj, ipaddr))
    {
        os_kprintf("Show socket status: get module intf address failed, module is not ready to provide socket service\r\n");
        return;
    }

    if (OS_EOK !=  mo_netconn_get_info(defmo_obj, &netconn_info))
    {
        os_kprintf("Module %s dosen't support mo_netconn_get_info\r\n", defmo_obj->name);
        return;
    }

    /* Show socket information */
    os_kprintf("\r\nThe max socket connections supported by module %s is %d\r\n",
               defmo_obj->name,
               netconn_info.netconn_nums);
    os_kprintf("\r\nThe connected socket status information list\r\n");
    for (i = 0; i < 50; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\r\n");

    os_kprintf("%-8s", "index");
    os_kprintf("%-10s", "socket");
    os_kprintf("%-10s", "type");
    os_kprintf("%-18s", "ip address");
    os_kprintf("%-8s", "port");
    os_kprintf("%-16s\r\n", "status");

    for (i = 0; i < netconn_info.netconn_nums; i++)
    {
        const mo_netconn_t *netconn = &netconn_info.netconn_array[i];

        socket_type_convert_to_char_info(netconn->type, socket_type);
        socket_status_convert_to_char_info(netconn->stat, socket_stat);

        os_kprintf("  %-6d", i + 1);

        if (netconn->connect_id > -1)
        {
            os_kprintf("%-10d", netconn->connect_id);
        }
        else
        {
            os_kprintf("          ");
        }

        os_kprintf("%-10s", socket_type);
        os_kprintf("%-18s", inet_ntoa(netconn->remote_ip));
        os_kprintf("%-8d", netconn->remote_port);
        os_kprintf("%-16s\r\n", socket_stat);
    }

    for (i = 0; i < 50; i++)
    {
        os_kprintf("--");
    }
    os_kprintf("\r\n");

    return;
}

static int module_cmd_socket_stat(int argc, char **argv)
{
    if (argc != 1)
    {
        os_kprintf("Input errror, please input: socketstat\r\n");
    }
    else
    {
        module_show_socket_stat();
    }

    return 0;
}
SH_CMD_EXPORT(socketstat, module_cmd_socket_stat, "List the information of module sockets status");
#endif /* defined(MOLINK_TOOLS_USING_SOCKETSTAT) && defined(MOLINK_USING_NETCONN_OPS) */

#endif /* NET_USING_MOLINK */

#endif /* OS_USING_SHELL */

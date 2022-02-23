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
 * @file        atiny_socket_adapter.h
 *
 * @brief       huawei cloud sdk file "atiny_socket.h" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __ATINY_SOCKET_ADAPTER_H__
#define __ATINY_SOCKET_ADAPTER_H__
#include <sys/socket.h>
#include <sys/select.h>

#define ONEOS_ADAPTER

#define ATINY_PROTO_TCP 0 /* < The TCP transport protocol */
#define ATINY_PROTO_UDP 1 /* < The UDP transport protocol */

#define ATINY_NET_OK               0
#define ATINY_NET_ERR              -1
#define ATINY_NET_TIMEOUT          -2
#define ATINY_NET_BIND_FAILED      -3
#define ATINY_NET_LISTEN_FAILED    -4
#define ATINY_NET_ACCEPT_FAILED    -5
#define ATINY_NET_BUF_SMALL_FAILED -6
#define ATINY_NET_SOCKET_FAILED    -7

//#endif
#endif /* __ATINY_SOCKET_ADAPTER_H__ */

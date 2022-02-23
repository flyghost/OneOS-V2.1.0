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
 * @file        onetls_socket.h
 *
 * @brief       onetls_socket header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_SOCKET_H__
#define __ONETLS_SOCKET_H__
#include "onetls.h"
#include "onetls_util.h"

#define ONETLS_TLS_SOCKET_POLL_DEFAULT_TIMEOUT  60000 // 60秒
// 探测接口
#define ONETLS_SOCK_RD (0)
#define ONETLS_SOCK_WR (1)

uint32_t onetls_sock_test(int fd, uint8_t op, uint32_t timeout);    // timeout是毫秒

uint32_t onetls_sock_recv(int fd, uint8_t *buf, uint32_t len, uint32_t *recv_len, uint32_t time_out);
uint32_t onetls_sock_send(int fd, uint8_t *buf, uint32_t len, uint32_t *send_len, uint32_t time_out);

uint32_t onetls_send_handshake_message(onetls_ctx *ctx, uint8_t flight);

uint32_t onetls_recv_record_type(onetls_ctx *ctx, uint8_t type);
uint32_t onetls_recv_record(onetls_ctx *ctx);
uint32_t onetls_send_record(onetls_ctx *ctx);
uint32_t onetls_send_record_not_encrypt(onetls_ctx *ctx);
#endif

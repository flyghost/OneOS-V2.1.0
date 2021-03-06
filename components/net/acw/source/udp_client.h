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
 * @file        acw.h
 *
 * @brief       acw declaration  
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-24   OneOS Team      first version
 ***********************************************************************************************************************
 */

#ifndef __UDP_CLIENT_H__
#define __UDP_CLIENT_H__

#include <os_types.h>
#include <os_errno.h>
#include <sys/socket.h>

extern int udp_client_init(ip_addr_t s_addr, os_uint16_t s_port);

#endif /* end of __UDP_CLIENT_H__ */

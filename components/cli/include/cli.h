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
 * @file        cli.h
 *
 * @brief       The client header file
 *
 * @revision
 * Date         Author          Notes
 * 2021-07-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CLI_H__
#define __CLI_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "oneos_config.h"

#include <drv_cfg.h>
#ifdef OS_USING_CLI

#if defined(CLI_PAYLOAD_BY_CMS) || defined(CLI_PAYLOAD_BY_LWIP) || defined(CLI_PAYLOAD_BY_MOLINK)
#define CLI_DATA_DEVICE_NAME   "clid"
#define CLI_CONTROL_DEVICE_NAM "clic"
#else
#warning "not defined cli payload network"
#endif

struct os_cli_device
{
    struct os_device dev;
    uint32_t         scode;
    uint16_t         buff_size;
    void *           handle;
    os_bool_t        is_open;
    os_bool_t        has_send_buf;
    rb_ring_buff_t * send_ring_buf;
};

#define NET_DEVICE_CONNECT    0x01
#define NET_DEVICE_DISCONNECT 0x02

typedef enum
{
    CLI_IDLE  = 0,
    CLI_OPEN  = 1,
    CLI_CLOSE = 2,
    CLI_RUN   = 3
} cli_status;

#endif
#ifdef __cplusplus
}
#endif

#endif

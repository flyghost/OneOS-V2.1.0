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

#ifndef __ACW_DEV_AP_H__
#define __ACW_DEV_AP_H__

#include <stdio.h>
#include <stdlib.h>
#include <os_list.h>

#include "acw_prot_common.h"

#define SLAVE_ALREADY_CONFIGED_TIMEOUT          (2 * 60 * 1000)
#define SLAVE_REQUESTED_TIMEOUT                 (5 * 60 * 1000)
#define SLAVE_APPROVED_TIMEOUT                  (2 * 60 * 1000)
#define SLAVE_REJECT_TIMEOUT                    (2 * 60 * 1000)
#define SLAVE_REUSE_TIMEOUT_DEF                 (30 * 60 * 1000)

#define OS_TICK_DIFF(a, b)      ((a > b) ? (a - b) : (0x100000000 - b + a))

#define UDP_SERVER_ADDR_SLAVE	"192.168.169.1"
#define UDP_SERVER_ADDR_MASTER	"192.168.168.1"
#define UDP_SERVER_PORT_TEST	9999
#define UDP_SERVER_BUF_MAX_LEN	512
#define MASTER_NO_MSG_DEF_RESP  "you:wait\n"

#define ACW_TABLE_ADD_INTER_DEF    4

typedef struct
{
    os_list_node_t dlist;
    ip_addr_t addr;
    os_int32_t port;
    int msg_len;
    char msg[1];
} acw_dev_ap_msg_t;

extern void acw_slave_loop(acw_run_ctrl_t *ctrl);
extern int acw_slave_search_master(acw_run_ctrl_t *ctrl);
extern void acw_slave_dev_ap_init(void);
extern void acw_slave_dev_ap_exit(void);
extern void acw_master_dev_ap_init(void);
extern void acw_master_dev_ap_exit(void);

extern acw_dev_ap_msg_t *acw_master_msg_alloc(int len);

static inline void acw_owner_id_to_ap_spec(char *owner, char *ap_spec, int spec_buf_len)
{
    long long phone_num;

    phone_num = strtoll(owner, NULL, 10);
    snprintf(ap_spec, spec_buf_len, "%llx", phone_num);

    return;
}

static inline void acw_ap_spec_to_owner(char *ap_spec, char *owner, int owner_buf_len)
{
    long long phone_num;

    phone_num = strtoll(ap_spec, NULL, 16);
    snprintf(owner, owner_buf_len, "%lld", phone_num);

    return;
}

#endif /* end of __ACW_DEV_AP_H__ */

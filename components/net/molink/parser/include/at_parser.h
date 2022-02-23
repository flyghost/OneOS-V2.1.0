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
 * @file        at_parser.h
 *
 * @brief       AT parser header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __AT_PARSER_H__
#define __AT_PARSER_H__

#include "at_resp.h"

#include <os_sem.h>
#include <os_event.h>
#include <os_mutex.h>
#include <os_task.h>
#include <os_types.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifndef END_MARK_LEN
#define END_MARK_LEN 2
#endif

#ifndef AT_PARSER_TASK_STACK_SIZE
#define AT_PARSER_TASK_STACK_SIZE 2048
#endif

#ifndef AT_PARSER_TASK_PRIORITY
#define AT_PARSER_TASK_PRIORITY (OS_TASK_PRIORITY_MAX / 3 - 1)
#endif

struct at_parser;

/**
 ***********************************************************************************************************************
 * @struct      at_parser_urc
 *
 * @brief       URC(Unsolicited Result Code) object
 ***********************************************************************************************************************
 */
typedef struct at_urc
{
    os_slist_node_t list; /* URC object list */

    const char *prefix; /* URC data prefix */
    const char *suffix; /* URC data prefix */
    /* URC data function */
    void (*func)(struct at_parser *parser, const char *data, os_size_t size);
} at_urc_t;

/**
 ***********************************************************************************************************************
 * @struct      at_parser
 *
 * @brief       The object of AT parser
 ***********************************************************************************************************************
 */
typedef struct at_parser
{
    os_slist_node_t list;       /* at parser object manage list  */

    char name[OS_NAME_MAX + 1]; /* The name of AT parser */

    os_device_t *device; /* The device used by AT parser */
    char     *at_cmd_buff; /* The send buffer of cmd */
    char     *recv_buff;     /* The receive buffer of AT parser */
    os_size_t recv_buff_len; /* The receive buffer length of AT parser */
    os_size_t curr_recv_len; /* The current receive data length of AT parser */

    os_event_t rx_notice_evt; /* The receive event of at parser */
    os_mutex_t exec_lock; /* The recursive lock that protects the execution of the AT command */

    os_sem_t resp_notice; /* the response notice semaphore */

    os_size_t end_mark_len;               /* The special end mark length */
    char      end_mark[END_MARK_LEN + 1]; /* The special end mark */

    at_resp_t *resp; /* AT command response object */

    os_slist_node_t urc_list; /* URC handle list , Used to process URC data */

    os_task_t *task; /* AT Parser task*/
} at_parser_t;

os_err_t at_parser_init(at_parser_t *parser, const char *name, os_device_t *device, os_size_t recv_buff_len);
os_err_t at_parser_startup(at_parser_t *parser);
os_err_t at_parser_deinit(at_parser_t *parser);

os_err_t  at_parser_exec_lock(at_parser_t *parser);
os_err_t  at_parser_exec_unlock(at_parser_t *parser);
os_err_t  at_parser_exec_cmd_valist(at_parser_t *parser, at_resp_t *resp, const char *cmd_expr, va_list args);
os_err_t  at_parser_exec_cmd(at_parser_t *parser, at_resp_t *resp, const char *cmd_expr, ...);

os_err_t  at_parser_connect(at_parser_t *parser, os_uint8_t retry_times);
os_size_t at_parser_send(at_parser_t *parser, const char *buf, os_size_t size);
os_size_t at_parser_recv(at_parser_t *parser, char *buf, os_size_t size, os_int32_t timeout);

os_err_t at_parser_set_end_mark(at_parser_t *parser, const char *end_mark, os_size_t end_mark_len);
void     at_parser_set_urc_table(at_parser_t *parser, at_urc_t urc_table[], os_size_t table_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AT_PARSER_H__ */

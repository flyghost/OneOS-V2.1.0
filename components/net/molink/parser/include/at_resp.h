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
 * @file        at_resp.h
 *
 * @brief       AT response header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __AT_RESP_H__
#define __AT_RESP_H__

#include <os_assert.h>
#include <os_errno.h>
#include <os_stddef.h>
#include <os_types.h>
#include <os_clock.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AT_RESP_OK    "OK"
#define AT_RESP_ERROR "ERROR"

#define AT_RESP_BUFF_SIZE_DEF 64                    /* AT response default buffer size */
#define AT_RESP_BUFF_SIZE_128 128                   /* AT response 128 buffer size */
#define AT_RESP_BUFF_SIZE_256 256                   /* AT response 256 buffer size */
#define AT_RESP_BUFF_SIZE_384 384                   /* AT response 384 buffer size */
#define AT_RESP_BUFF_SIZE_512 512                   /* AT response 512 buffer size */
#define AT_RESP_LINE_NUM_DEF  0                     /* The expected response data linenum */
#define AT_RESP_TIMEOUT_DEF   os_tick_from_ms(5000) /* The expected at response timeout, uint ms */

/**
 ***********************************************************************************************************************
 * @enum        resp_stat
 *
 * @brief       The state of AT parser response object
 ***********************************************************************************************************************
 */
typedef enum resp_stat
{
    RESP_STAT_NULL = 0,      /* Parser response end is NULL */
    RESP_STAT_OK,        /* Parser response end is OK */
    RESP_STAT_ERROR,     /* Parser response end is ERROR */
    RESP_STAT_TIMEOUT,   /* Parser response is timeout */
    RESP_STAT_BUFF_FULL, /* Parser response buffer is full */
} resp_stat_t;

/**
 ***********************************************************************************************************************
 * @struct      at_parser_resp
 *
 * @brief       AT parser response object
 ***********************************************************************************************************************
 */
typedef struct at_resp
{
    resp_stat_t stat;          /* the status of current response */

    os_size_t   line_num;      /* the number of setting response lines */
    os_size_t   line_counts;   /* the count of received response lines */
    os_int32_t  timeout;       /* the maximum response time */

    os_size_t   curr_buff_len; /* the length of current response buffer */
    os_size_t   buff_size;     /* the maximum response buffer size */
    char       *buff;          /* response buffer */
} at_resp_t;

const char *at_resp_get_line(at_resp_t *resp, os_size_t resp_line);
const char *at_resp_get_line_by_kw(at_resp_t *resp, const char *keyword);
os_int32_t  at_resp_get_data_by_line(at_resp_t *resp, os_size_t resp_line, const char *resp_expr, ...);
os_int32_t  at_resp_get_data_by_kw(at_resp_t *resp, const char *keyword, const char *resp_expr, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AT_RESP_H__ */

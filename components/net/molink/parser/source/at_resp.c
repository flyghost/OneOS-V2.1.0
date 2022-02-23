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
 * @file        at_resp.c
 *
 * @brief       Implement AT response object functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "at_resp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "at.resp"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

/**
 ***********************************************************************************************************************
 * @brief           Get one line form AT parser response buffer by line number
 *
 * @param[in]       resp            A pointer to AT Parser response instance
 * @param[in]       resp_line       The line number of response data, , start from '1'
 *
 * @return          Return string of one line data
 * @retval          != OS_NULL      Operate successfully
 * @retval          == OS_NULL      Operate failed
 ***********************************************************************************************************************
 */
const char *at_resp_get_line(at_resp_t *resp, os_size_t resp_line)
{
    OS_ASSERT(resp);

    char *resp_buf = resp->buff;

    if (resp_line > resp->line_counts || resp_line < 1)
    {
        ERROR("AT response get line failed! Input response line(%d) error!", resp_line);
        return OS_NULL;
    }

    for (int line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (resp_line == line_num)
        {
            char *resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Get one line form AT parser response buffer by keyword
 *
 * @param[in]       resp            A pointer to AT Parser response instance
 * @param[in]       keyword         The keyword of response data
 *
 * @return          Return string of one line data
 * @retval          != OS_NULL      Operate successfully
 * @retval          == OS_NULL      Operate failed
 ***********************************************************************************************************************
 */
const char *at_resp_get_line_by_kw(at_resp_t *resp, const char *keyword)
{
    OS_ASSERT(resp);
    OS_ASSERT(keyword);

    char *resp_buf = resp->buff;

    for (int line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (strstr(resp_buf, keyword))
        {
            char *resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Get data form AT parser response buffer by line
 *
 * @param[in]       resp            A pointer to AT response instance
 * @param[in]       resp_line       The line number of response data, , start from '1
 * @param[in]       resp_expr       The data parser expression
 * @param[in]       ...             The data parser arguments
 * 
 * @return          Return get data nums
 ***********************************************************************************************************************
 */
os_int32_t at_resp_get_data_by_line(at_resp_t *resp, os_size_t resp_line, const char *resp_expr, ...)
{
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(resp_expr != OS_NULL);

    const char *resp_line_buf = at_resp_get_line(resp, resp_line);

    if (OS_NULL == resp_line_buf)
    {
        return -1;
    }

    va_list args = {0};

    va_start(args, resp_expr);

    os_int32_t resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

/**
 ***********************************************************************************************************************
 * @brief           Get data form AT parser response buffer by line
 *
 * @param[in]       resp            A pointer to AT response instance
 * @param[in]       keyword         The keyword of response data
 * @param[in]       resp_expr       The data parser expression
 * @param[in]       ...             The data parser arguments
 * 
 * @return          Return get data nums
 ***********************************************************************************************************************
 */
os_int32_t at_resp_get_data_by_kw(at_resp_t *resp, const char *keyword, const char *resp_expr, ...)
{
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(keyword != OS_NULL);
    OS_ASSERT(resp_expr != OS_NULL);

    const char *resp_line_buf = at_resp_get_line_by_kw(resp, keyword);

    if (OS_NULL == resp_line_buf)
    {
        return -1;
    }

    va_list args = {0};

    va_start(args, resp_expr);

    os_int32_t resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

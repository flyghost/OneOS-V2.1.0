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
 * @file        a7600x_general.c
 *
 * @brief       a7600x module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "a7600x_general.h"
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "a7600x.general"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef A7600X_USING_GENERAL_OPS

os_err_t a7600x_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t a7600x_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGSN");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_line(&resp, 1, "%s", value) <= 0)
    {
        ERROR("Get %s module imei failed", self->name);
        return OS_ERROR;
    }

    value[MO_IMEI_LEN] = '\0';

    DEBUG("module %s imei: %s", value);

    return OS_EOK;
}

os_err_t a7600x_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIMI");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_line(&resp, 1, "%s", value) <= 0)
    {
        ERROR("Get %s module imsi failed", self->name);
        return OS_ERROR;
    }

    value[MO_IMSI_LEN] = '\0';

    DEBUG("module %s imsi: %s", value);

    return OS_EOK;
}

os_err_t a7600x_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CICCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+ICCID:", "+ICCID: %s", value) <= 0)
    {
        ERROR("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    DEBUG("module %s iccid: %s", value);

    return OS_EOK;
}

os_err_t a7600x_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN: %hhu", fun_lvl) <= 0)
    {
        ERROR("Get %s module level of functionality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t a7600x_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

#endif /* A7600X_USING_GENERAL_OPS */

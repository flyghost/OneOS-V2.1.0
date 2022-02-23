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
 * @file        m5310a_general.c
 *
 * @brief       m5310-a module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5310a_general.h"
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "m5310a.general"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef M5310A_USING_GENERAL_OPS

os_err_t m5310a_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t m5310a_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGSN=1");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGSN:", "+CGSN:%s", value) <= 0)
    {
        ERROR("Get %s module imei failed", self->name);
        return OS_ERROR;
    }

    value[MO_IMEI_LEN] = '\0';

    DEBUG("module %s imei:%s", value);

    return OS_EOK;
}

os_err_t m5310a_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * AT_RESP_TIMEOUT_DEF};

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

    DEBUG("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t m5310a_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NCCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+NCCID:", "+NCCID:%s", value) <= 0)
    {
        ERROR("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    DEBUG("module %s iccid: %s", value);

    return OS_EOK;
}

os_err_t m5310a_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN:%hhu", fun_lvl) <= 0)
    {
        ERROR("Get %s module level of functionality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5310a_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

os_err_t m5310a_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version)
{
   at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGMR");
    if (result != OS_EOK)
    {
        return result;
    }

    version->ver_info = os_calloc(resp.line_counts - 1, sizeof(char *));
    if (OS_NULL == version->ver_info)
    {
        return OS_ENOMEM;
    }

    version->line_counts = 0;

    for (int i = 2; i <= resp.line_counts - 1; i += 2)
    {
        const char *source_line = at_resp_get_line(&resp, i);
        os_size_t   line_length = strlen(source_line);

        char **dest_line = &version->ver_info[version->line_counts];

        *dest_line = os_calloc(1, line_length + 1);
        if (OS_NULL == *dest_line)
        {
            mo_get_firmware_version_free(version);

            return OS_ENOMEM;
        }

        strncpy(*dest_line, source_line, line_length);
        version->line_counts ++;
    }

    return OS_EOK;
}

#endif /* M5310A_USING_GENERAL_OPS */

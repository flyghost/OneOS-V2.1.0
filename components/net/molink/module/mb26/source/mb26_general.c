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
 * @file        mb26_general.c
 *
 * @brief       mb26 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mb26_general.h"
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "mb26.general"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#ifdef MB26_USING_GENERAL_OPS

os_err_t mb26_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t mb26_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGSN=1");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGSN:", "+CGSN: \"%[^\"]", value) <= 0)
    {
        ERROR("Get %s module imei failed", self->name);
        return OS_ERROR;
    }

    value[MO_IMEI_LEN] = '\0';

    DEBUG("module %s imei:%s", value);

    return OS_EOK;
}

os_err_t mb26_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIMI");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_line(&resp, 2, "%s", value) <= 0)
    {
        ERROR("Get %s module imsi failed", self->name);
        return OS_ERROR;
    }

    value[MO_IMSI_LEN] = '\0';

    DEBUG("module %s imsi:%s", value);

    return OS_EOK;
}

os_err_t mb26_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+ECICCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    /* For ex: +ECICCID: 89861119220009636664 */
    if (at_resp_get_data_by_kw(&resp, "+ECICCID:", "+ECICCID: %s", value) <= 0)
    {
        ERROR("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    DEBUG("module %s iccid: %s", value);

    return OS_EOK;
}

os_err_t mb26_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;
    os_uint32_t  val    = 0;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN:%u", &val) <= 0)
    {
        ERROR("Get %s module level of functionality failed", self->name);
        return OS_ERROR;
    }

    *fun_lvl = (os_uint8_t)val;

    return OS_EOK;
}

os_err_t mb26_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%u", fun_lvl);
}

os_err_t mb26_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version)
{
   at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGMR");
    if (result != OS_EOK)
    {
        return result;
    }

    DEBUG("Get %s module version resp line counts: %d", self->name, resp.line_counts);

    version->ver_info = os_calloc(resp.line_counts - 2, sizeof(char *));
    if (OS_NULL == version->ver_info)
    {
        return OS_ENOMEM;
    }

    version->line_counts = 0;

    for (int i = 3; i <= resp.line_counts - 2; i++)
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

#endif /* MB26_USING_GENERAL_OPS */

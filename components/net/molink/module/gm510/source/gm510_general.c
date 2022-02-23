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
 * @file       gm510_general.c
 *
 * @brief      gm510 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#include <stdlib.h>
#include <string.h>
#include "gm510_general.h"

#define MO_LOG_TAG "gm510_general"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef GM510_USING_GENERAL_OPS

os_err_t gm510_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t gm510_get_imei(mo_object_t * self,char * value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CGSN"))
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_line(&resp, 1, "%s", value) <= 0)
    {
        ERROR("Get %s module imei failed.");
        return OS_ERROR;
    }


    value[MO_IMEI_LEN] = '\0';

    INFO("%s module imei:%s", self->name, value);

    return OS_EOK;
}

os_err_t gm510_get_imsi(mo_object_t * self,char * value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &self->parser;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * AT_RESP_TIMEOUT_DEF};

    if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CIMI"))
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_line(&resp, 1, "%s", value) <= 0)
    {
        ERROR("Get %s module imsi failed.");
        return OS_ERROR;
    }

    value[MO_IMSI_LEN] = '\0';

    DEBUG("%s module imsi:%s",self->name,value);

    return OS_EOK;
}

os_err_t gm510_get_iccid(mo_object_t * self,char * value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+ZGETICCID"))
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+ZGETICCID:", "+ZGETICCID: %s", value) <= 0)
    {
        ERROR("Get %s module ccid failed.", self->parser);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    return OS_EOK;
}

os_err_t gm510_get_cfun(mo_object_t * self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CFUN?"))
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN: %hhu",fun_lvl) <= 0)
    {
        ERROR("Get %s module level of functionality failed.", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t gm510_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

os_err_t gm510_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+GMR");
    if (result != OS_EOK)
    {
        return result;
    }

    version->ver_info = os_calloc(1, sizeof(char *));
    if (OS_NULL == version->ver_info)
    {
        return OS_ENOMEM;
    }

    const char *source_line = at_resp_get_line(&resp, 1);
    os_size_t   line_length = strlen(source_line);

    char **dest_line = &version->ver_info[0];

    *dest_line = os_calloc(1, line_length + 1);
    if (OS_NULL == *dest_line)
    {
        mo_get_firmware_version_free(version);

        return OS_ENOMEM;
    }

    strncpy(*dest_line, source_line, line_length);
    version->line_counts = 1;

    return OS_EOK;
}


#endif   /* GM510_USING_GENERAL_OPS */


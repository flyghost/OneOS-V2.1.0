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
 * @file        l610_general.c
 *
 * @brief       l610 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "l610_general.h"
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "l610.general"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef L610_USING_GENERAL_OPS

os_err_t l610_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 1 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t l610_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 1 * OS_TICK_PER_SECOND};

    if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CGSN=1"))
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGSN:", "+CGSN:%s", value) <= 0)
    {
        ERROR("Get %s module imei failed.", self->name);
        return OS_ERROR;
    }

    value[MO_IMEI_LEN] = '\0';

    DEBUG("%s module imei:%s", self->name, value);

    return OS_EOK;
}

os_err_t l610_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 1 * OS_TICK_PER_SECOND};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CIMI?"))
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CIMI:", "+CIMI: %s", value) <= 0)
    {
        ERROR("Get %s module imsi failed.", self->name);
        return OS_ERROR;
    }

    value[MO_IMSI_LEN] = '\0';

    DEBUG("%s module imsi:%s", self->name, value);

    return OS_EOK;
}

os_err_t l610_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 1 * OS_TICK_PER_SECOND};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CCID?"))
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CCID:", "+CCID: %s", value) <= 0)
    {
        ERROR("Get %s module ccid failed.", self->parser);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    return OS_EOK;
}

static os_err_t l610_set_apn(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+COPS?") != OS_EOK)
    {
        ERROR("Module %s read operator selects failed", self->name);
        return OS_ERROR;
    }

    char oper[17] = {0};
    if (at_resp_get_data_by_kw(&resp, "+COPS:", "+COPS: %*d,%*d,\"%[^\"]\",%*d", oper) < 0)
    {
        WARN("Module %s parser operator selects failed", self->name);
        return OS_ERROR;
    }

    char apn[10] = {0};
    if (strcmp(oper, "CHINA MOBILE") == 0)
    {
        strncpy(apn, "CMNET", strlen("CMNET"));
    }
    else if (strcmp(oper, "CHN-UNICOM") == 0)
    {
        strncpy(apn, "3GNET", strlen("3GNET"));
    }
    else if (strcmp(oper, "CHN-CT") == 0)
    {
        strncpy(apn, "CTNET", strlen("CTNET"));
    }

    resp.line_num = 2;
    resp.timeout  = 150 * OS_TICK_PER_SECOND;

    if (at_parser_exec_cmd(parser, &resp, "AT+MIPCALL=1,\"%s\"", apn) != OS_EOK)
    {
        ERROR("Module %s open tcp/ip protocol stack failed", self->name);
        return OS_ERROR;
    }

    os_int32_t stat = 0;
    if ((at_resp_get_data_by_kw(&resp, "+MIPCALL:", "+MIPCALL: %d", &stat) <= 0) || (0 == stat))
    {
        WARN("Module %s open tcp/ip protocol stack, resp parse failed", self->name);
        return OS_ERROR;
    }

    INFO("Module %s netserv open OK", self->name);

    return OS_EOK;
}

os_err_t l610_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 1 * OS_TICK_PER_SECOND};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CFUN?"))
    {
        ERROR("Get %s module level of functionality failed, cmd exec failed", self->name);
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN: %hhu", fun_lvl) <= 0)
    {
        ERROR("Get %s module level of functionality failed.", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t l610_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
    if (result != OS_EOK)
    {
        ERROR("Set %s module level of functionality failed, cmd exec failed", self->name);
        return result;
    }

    if (1 == fun_lvl)
    {
        os_task_msleep(3000);

        result = l610_set_apn(self);
        if (result != OS_EOK)
        {
            WARN("Module %s set apn failed", self->name);
        }
    }

    return result;
}

os_err_t l610_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+GMR");
    if (result != OS_EOK)
    {
        return result;
    }

    version->ver_info = os_calloc(1, sizeof(char *));
    if (OS_NULL == version->ver_info)
    {
        ERROR("Get %s module firmware version failed, malloc memory failed.", self->name);
        return OS_ENOMEM;
    }

    const char *source_line = at_resp_get_line(&resp, 1);
    os_size_t   line_length = strlen(source_line);

    char **dest_line = &version->ver_info[0];

    *dest_line = os_calloc(1, line_length + 1);
    if (OS_NULL == *dest_line)
    {
        mo_get_firmware_version_free(version);
        ERROR("Get %s module firmware version failed, malloc memory failed", self->name);
        return OS_ENOMEM;
    }

    strncpy(*dest_line, source_line, line_length);
    version->line_counts = 1;

    return OS_EOK;
}

#endif /* L610_USING_GENERAL_OPS */


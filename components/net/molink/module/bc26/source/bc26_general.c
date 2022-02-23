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
 * @file        bc26_general.c
 *
 * @brief       bc26 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc26_general.h"
#include "bc26.h"
#include <stdio.h>
#include <string.h>

#define MO_LOG_TAG "bc26.general"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef BC26_USING_GENERAL_OPS

os_err_t bc26_at_test(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t bc26_get_imei(mo_object_t *module, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGSN=1");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGSN:", "+CGSN:%s", value) <= 0)
    {
        ERROR("Get %s module imei failed", module->name);
        return OS_ERROR;
    }

    value[MO_IMEI_LEN] = '\0';

    DEBUG("%s-%d: imei:%s", __func__, __LINE__, value);

    return OS_EOK;
}

os_err_t bc26_get_imsi(mo_object_t *module, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIMI");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_line(&resp, 1, "%s", value) <= 0)
    {
        ERROR("Get %s module imsi failed", module->name);
        return OS_ERROR;
    }

    value[MO_IMSI_LEN] = '\0';

    DEBUG("%s-%d: imsi:%s", __func__, __LINE__, value);

    return OS_EOK;
}

os_err_t bc26_get_iccid(mo_object_t *module, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+QCCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+QCCID:", "+QCCID:%s", value) <= 0)
    {
        ERROR("Get %s module iccid failed", module->name);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    DEBUG("%s-%d: iccid:%s", __func__, __LINE__, value);

    return OS_EOK;
}

os_err_t bc26_get_cfun(mo_object_t *module, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 85 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN:%hhu", fun_lvl) <= 0)
    {
        ERROR("Get %s module level of functionality failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc26_set_cfun(mo_object_t *module, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 85 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

static void bc26_ip_indicator_urc(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    char ip_indicator_buf[AT_RESP_BUFF_SIZE_DEF] = {0};

    sscanf(data, "+IP: %63s", ip_indicator_buf);
    
    INFO("%s IP indocator:[%s]", __func__, ip_indicator_buf);
    return;
}

static void bc26_cpin_indicator_urc(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    char cpin_indicator_buf[AT_RESP_BUFF_SIZE_DEF] = {0};

    sscanf(data, "+CPIN: %63s", cpin_indicator_buf);
    
    INFO("%s CPIN indocator:[%s]", __func__, cpin_indicator_buf);
    return;
}

static at_urc_t bc26_general_urc_table[] = {
    {.prefix = "+IP:",   .suffix = "\r\n", .func = bc26_ip_indicator_urc  },
    {.prefix = "+CPIN:", .suffix = "\r\n", .func = bc26_cpin_indicator_urc},
};

void bc26_general_init(mo_bc26_t *module)
{
    /* Set general urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, bc26_general_urc_table, 
                            sizeof(bc26_general_urc_table) / sizeof(bc26_general_urc_table[0]));
}

#endif /* BC26_USING_GENERAL_OPS */

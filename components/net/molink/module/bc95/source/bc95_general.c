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
 * @file        bc95_general.c
 *
 * @brief       bc95 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc95_general.h"

#define MO_LOG_TAG "bc95.general"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef BC95_USING_GENERAL_OPS

#define BC95_GENERAL_TIMEOUT_DFT  (1 * OS_TICK_PER_SECOND)

os_err_t bc95_at_test(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = BC95_GENERAL_TIMEOUT_DFT};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t bc95_get_imei(mo_object_t *module, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = BC95_GENERAL_TIMEOUT_DFT};

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

    DEBUG("module %s imei:%s", module->name, value);

    return OS_EOK;
}

os_err_t bc95_get_imsi(mo_object_t *module, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = BC95_GENERAL_TIMEOUT_DFT};

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

    DEBUG("module %s imsi:%s", module->name, value);

    return OS_EOK;
}

os_err_t bc95_get_iccid(mo_object_t *module, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = BC95_GENERAL_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NCCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+NCCID:", "+NCCID:%s", value) <= 0)
    {
        ERROR("Get %s module iccid failed", module->name);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    DEBUG("module %s iccid: %s", module->name, value);

    return OS_EOK;
}

os_err_t bc95_get_cfun(mo_object_t *module, os_uint8_t *fun_lvl)
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

os_err_t bc95_set_cfun(mo_object_t *module, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 85 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

#endif /* BC95_USING_GENERAL_OPS */

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
 * @file        clm920rv3_netserv.c
 *
 * @brief       clm920rv3 module link kit netserv api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "clm920rv3_netserv.h"
#include "clm920rv3.h"

#include <stdlib.h>
#include <string.h>
#ifdef MOLINK_USING_IP
#include <mo_ipaddr.h>
#endif /* MOLINK_USING_IP */

#ifdef CLM920RV3_USING_NETSERV_OPS

#define MO_LOG_TAG "clm920rv3.netserv"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

os_err_t clm920rv3_set_attach(mo_object_t *module, os_uint8_t attach_stat)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 140 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%hhu", attach_stat);
}

os_err_t clm920rv3_get_attach(mo_object_t *module, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 140 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT: %hhu", attach_stat) <= 0)
    {
        ERROR("Get %s module attach state failed", module->name);
        return OS_ERROR;
    }
    
    return OS_EOK;
}

os_err_t clm920rv3_set_reg(mo_object_t *module, os_uint8_t reg_n)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%hhu", reg_n);
}

os_err_t clm920rv3_get_reg(mo_object_t *module, eps_reg_info_t *info)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    memset(info, 0, sizeof(eps_reg_info_t));

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG: %hhu,%hhu,", &info->reg_n, &info->reg_stat) <= 0)
    {
        ERROR("Get %s module register state failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t clm920rv3_get_cgact(mo_object_t *module, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT: %hhu,%hhu", cid, act_stat) <= 0)
    {
        ERROR("Get %s module cgact state failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t clm920rv3_get_csq(mo_object_t *module, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ: %hhu,%hhu", rssi, ber) <= 0)
    {
        ERROR("Get %s module signal quality failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

#endif /* CLM920RV3_USING_NETSERV_OPS */

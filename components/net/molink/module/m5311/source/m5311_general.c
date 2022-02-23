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
 * @file        m5311_general.c
 *
 * @brief       m5311 module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "m5311_general.h"
#include "mo_time.h"

#define MO_LOG_TAG "m5311.general"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef M5311_USING_GENERAL_OPS

#define M5311_GENERAL_TIMEOUT_DFT   (2 * OS_TICK_PER_SECOND)
#define M5311_PSM_QUOTES_LEN        (2)
#define M5311_TIMEZONE_STR_LEN_DEF  (8)


os_err_t m5311_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_GENERAL_TIMEOUT_DFT};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t m5311_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout =  M5311_GENERAL_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+GSN");
    if(result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_line(&resp, 1, "%s", value))
    {
        ERROR("%s-%d:Get imei failed.", __func__, __LINE__);
        return OS_ERROR;
    }

    value[MO_IMEI_LEN] = '\0';

    DEBUG("%s-%d:imei:%s.", __func__, __LINE__, value);

    return OS_EOK;
}

os_err_t m5311_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMSI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_GENERAL_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIMI");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_line(&resp, 1, "%s", value))
    {
        ERROR("%s-%d:Get imsi failed.", __func__, __LINE__);
        return OS_ERROR;
    }

    value[MO_IMSI_LEN] = '\0';

    DEBUG("%s-%d:imsi:%s.", __func__, __LINE__, value);

    return OS_EOK;
}

os_err_t m5311_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_GENERAL_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+ICCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_kw(&resp, "+ICCID: ", "+ICCID: %s", value))
    {
        ERROR("%s-%d:Get iccid failed.", __func__, __LINE__);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    DEBUG("%s-%d:iccid:%s.", __func__, __LINE__, value);

    return OS_EOK;
}

os_err_t m5311_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_GENERAL_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CFUN?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_kw(&resp, "+CFUN:", "+CFUN: %hhu", fun_lvl))
    {
        ERROR("%s-%d:Get module level of functionality failed.", __func__, __LINE__);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 90 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

os_err_t m5311_gm_time(mo_object_t *self, struct tm *l_tm)
{
    at_parser_t *parser   = &self->parser;
    os_err_t     result   = OS_EOK;
    _mo_tm_t     mo_tm    = {0};

    char timezone_str[M5311_TIMEZONE_STR_LEN_DEF] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF]         = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 90 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &resp, "AT+CCLK?");
    if (OS_EOK != result)
    {
        ERROR("%s-%d:Request local time failed", __func__, __LINE__);
        goto __exit;
    }

    /* at least 6 parm required: yy/MM/dd,hh:mm:ss */
    if (6 >= at_resp_get_data_by_kw(&resp, "+CCLK:", "+CCLK: %d/%d/%d,%d:%d:%d%s", 
                                    &mo_tm.tm_year, 
                                    &mo_tm.tm_mon,
                                    &mo_tm.tm_mday,
                                    &mo_tm.tm_hour,
                                    &mo_tm.tm_min,
                                    &mo_tm.tm_sec,
                                     timezone_str))
    {
        ERROR("%s-%d:get local time failed", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    /* adapt to mo_tm_t tm_year range. */
    mo_tm.tm_year += 2000;

    /* timezone handle process */
    if (0 == strlen(timezone_str))
    {
        DEBUG("%s-%d:no timezone info passed by module.", __func__, __LINE__);
    }
    else if (0 >= sscanf(timezone_str, "%d", &mo_tm.tm_q_off)) 
    {
        WARN("%s-%d:parse timezone info failed.", __func__, __LINE__);
    }

    time_struct_convert(&mo_tm, l_tm);

__exit:

    if (OS_EOK != result) memset(l_tm, 0, sizeof(struct tm));

    return result;
}

#endif /* M5311_USING_GENERAL_OPS */

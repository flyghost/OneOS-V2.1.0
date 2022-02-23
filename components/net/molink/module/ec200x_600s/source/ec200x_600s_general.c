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
 * @file        ec200x_600s.c
 *
 * @brief       ec200x_600s module link kit general api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec200x_600s_general.h"
#include <stdlib.h>
#include <string.h>
#include <board.h>
#include <drv_gpio.h>

#define MO_LOG_TAG "ec200x_600s.general"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef EC200X_600S_USING_GENERAL_OPS

os_err_t ec200x_600s_at_test(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT");
}

os_err_t ec200x_600s_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_IMEI_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+GSN");
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

    DEBUG("module %s imei: %s", self->name, value);

    return OS_EOK;
}

os_err_t ec200x_600s_get_imsi(mo_object_t *self, char *value, os_size_t len)
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

    DEBUG("module %s imsi: %s", self->name, value);

    return OS_EOK;
}

os_err_t ec200x_600s_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(len > MO_ICCID_LEN);

    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+QCCID");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+QCCID:", "+QCCID: %s", value) <= 0)
    {
        ERROR("Get %s module iccid failed", self->name);
        return OS_ERROR;
    }

    value[MO_ICCID_LEN] = '\0';

    DEBUG("module %s iccid: %s", self->name, value);

    return OS_EOK;
}

os_err_t ec200x_600s_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

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

os_err_t ec200x_600s_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CFUN=%hhu", fun_lvl);
}

os_err_t ec200x_600s_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

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



#ifndef EC200X_600S_DTR_PIN_NUM
#define EC200X_600S_DTR_PIN_NUM (-1)
#endif

#ifndef EC200X_600S_WAKE_IN_PIN_NUM
#define EC200X_600S_WAKE_IN_PIN_NUM (-1)
#endif

os_err_t ec200x_600s_sleep_mode_set(mo_object_t *self, os_uint8_t fun_lvl)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    os_err_t result;
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * OS_TICK_PER_SECOND};
    
    os_pin_mode(EC200X_600S_DTR_PIN_NUM, PIN_MODE_OUTPUT);
    os_pin_mode(EC200X_600S_WAKE_IN_PIN_NUM, PIN_MODE_OUTPUT);
      
    if (fun_lvl == 1)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+QSCLK=1");
        if (result != OS_EOK)
        {
            return result;
        }
        os_pin_write(EC200X_600S_DTR_PIN_NUM,PIN_HIGH);
        os_pin_write(EC200X_600S_WAKE_IN_PIN_NUM,PIN_HIGH);
        
        os_task_msleep(6000); 
    }
    else if (fun_lvl == 0)
    {        
        os_pin_write(EC200X_600S_DTR_PIN_NUM,PIN_LOW);
        os_pin_write(EC200X_600S_WAKE_IN_PIN_NUM,PIN_LOW);
        
        os_task_msleep(200);
        
        os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+QSCLK=0");
        if (result != OS_EOK)
        {
            return result;
        }    
    }  
    else
    {
        ERROR("Set sleep mode arg error,the arg is 0 or 1!");
        return OS_ERROR;
    }
    
    return OS_EOK;
}



#endif /* EC200X_600S_USING_GENERAL_OPS */

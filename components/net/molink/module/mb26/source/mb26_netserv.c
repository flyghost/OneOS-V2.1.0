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
 * @file        mb26_netserv.c
 *
 * @brief       mb26 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mb26_netserv.h"
#include "mb26.h"

#define MO_LOG_TAG "mb26.netserv"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#ifdef MB26_USING_NETSERV_OPS

#define MB26_PSM_QUOTES_LEN    (2)

os_err_t mb26_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%u", attach_stat);
}

os_err_t mb26_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;
    os_uint32_t  val    = 0;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT: %u", &val) <= 0)
    {
        ERROR("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    *attach_stat = (os_uint8_t)val;

    return OS_EOK;
}

os_err_t mb26_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%u", reg_n);
}

os_err_t mb26_get_reg(mo_object_t *self, eps_reg_info_t *info)
{
    at_parser_t *parser = &self->parser;
    os_uint32_t  reg_n  = 0;
    os_uint32_t  state  = 0;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG: %u,%u", &reg_n, &state) <= 0)
    {
        ERROR("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    info->reg_n    = (os_uint8_t)reg_n;
    info->reg_stat = (os_uint8_t)state;

    return OS_EOK;
}

os_err_t mb26_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%u,%u", act_stat, cid);
}

os_err_t mb26_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;
    os_uint32_t  val    = 0;
    os_uint32_t  state  = 0;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT: %u,%u", &val, &state) <= 0)
    {
        ERROR("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    *cid      = (os_uint8_t)val;
    *act_stat = (os_uint8_t)state;

    return OS_EOK;
}

os_err_t mb26_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser   = &self->parser;
    os_uint32_t  tmp_rssi = 0;
    os_uint32_t  tmp_ber  = 0;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ:%u,%u", &tmp_rssi, &tmp_ber) <= 0)
    {
        ERROR("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    *rssi = (os_uint8_t)tmp_rssi;
    *ber  = (os_uint8_t)tmp_ber;

    return OS_EOK;
}

os_err_t mb26_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
    at_parser_t *parser       = &self->parser;
    os_uint32_t  buff_len     = 1024;
    const char  *tmp_src_resp = OS_NULL;
    char        *dest_ptr     = OS_NULL;

    memset(radio_info, 0, sizeof(radio_info_t));

    char *resp_buff = os_calloc(1, buff_len);

    if (OS_NULL == resp_buff)
    {
        ERROR("Module %s get radio info failed, no enough memory", self->name);
        return OS_ENOMEM;
    }
    memset(resp_buff, 0, buff_len);

    at_resp_t resp = {.buff = resp_buff, .buff_size = buff_len, .timeout = 5 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+ECSTATUS");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    /* Get DlEarfcn info form resp buff */
    if (at_resp_get_data_by_kw(&resp, "+ECSTATUS: PHY", "+ECSTATUS: PHY, DlEarfcn:%d,", &radio_info->earfcn) <= 0)
    {
        ERROR("Get %s module Earfcn failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    tmp_src_resp = at_resp_get_line_by_kw(&resp, "+ECSTATUS: PHY");

    /* Get PCI info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "PCI:");
    if (dest_ptr == OS_NULL)
    {
        ERROR("Get %s module PCI failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "PCI:%d,", &radio_info->pci);

    /* Get RSRP info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "RSRP:");
    if (dest_ptr == OS_NULL)
    {
        ERROR("Get %s module RSRP failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "RSRP:%d,",&radio_info->rsrp);

    /* Get RSRQ info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "RSRQ:");
    if (dest_ptr == OS_NULL)
    {
        ERROR("Get %s module RSRQ failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "RSRQ:%d,",&radio_info->rsrq);

    /* Get SNR info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "SNR:");
    if (dest_ptr == OS_NULL)
    {
        ERROR("Get %s module SNR failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "SNR:%d,",&radio_info->snr);

    /* Get ECL info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "CeLevel:");
    if (dest_ptr == OS_NULL)
    {
        ERROR("Get %s module ECL failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "CeLevel:%d,",&radio_info->ecl);

    tmp_src_resp = at_resp_get_line_by_kw(&resp, "+ECSTATUS: RRC");
    /* Get CellId info form tmp_src_resp buff */
    dest_ptr = strstr(tmp_src_resp, "CellId:");
    if (dest_ptr == OS_NULL)
    {
        ERROR("Get %s module CellId failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }
    sscanf(dest_ptr, "CellId:%s",radio_info->cell_id);

__exit:

    if(resp_buff != OS_NULL)
    {
        os_free(resp_buff);
    }

    return result;
}

os_err_t mb26_set_psm(mo_object_t *self, mo_psm_info_t info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    /* MB26 needs double quotes */
    os_int8_t periodic_rau[PSM_TIMER_MAX_STR_LEN + MB26_PSM_QUOTES_LEN]     = {0};
    os_int8_t gprs_ready_timer[PSM_TIMER_MAX_STR_LEN + MB26_PSM_QUOTES_LEN] = {0};
    os_int8_t periodic_tau[PSM_TIMER_MAX_STR_LEN + MB26_PSM_QUOTES_LEN]     = {0};
    os_int8_t active_time[PSM_TIMER_MAX_STR_LEN + MB26_PSM_QUOTES_LEN]      = {0};

    /* only insert double quotes when the value is valid */
    if (0 != strlen(info.periodic_rau))
    {
        snprintf((char *)periodic_rau, sizeof(periodic_rau), "\"%s\"", info.periodic_rau);
    }

    if (0 != strlen(info.gprs_ready_timer))
    {
        snprintf((char *)gprs_ready_timer, sizeof(gprs_ready_timer), "\"%s\"", info.gprs_ready_timer);
    }

    if (0 != strlen(info.periodic_tau))
    {
        snprintf((char *)periodic_tau, sizeof(periodic_tau), "\"%s\"", info.periodic_tau);
    }

    if (0 != strlen(info.active_time))
    {
        snprintf((char *)active_time, sizeof(active_time), "\"%s\"", info.active_time);
    }

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser,
                              &resp,
                              "AT+CPSMS=%d,%s,%s,%s,%s",
                              info.psm_mode,
                              periodic_rau,
                              gprs_ready_timer,
                              periodic_tau,
                              active_time);
}

os_err_t mb26_get_psm(mo_object_t *self, mo_psm_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF * 2] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CPSMS?"))
    {
        ERROR("Get psm info fail: AT+CPSMS? cmd exec fail.");
        return OS_ERROR;
    }

    memset(info, 0, sizeof(mo_psm_info_t));

    if (0 >= at_resp_get_data_by_kw(&resp, "+CPSMS", "+CPSMS: %d,,,\"%[^\"]\",\"%[^\"]", &info->psm_mode, info->periodic_tau, info->active_time))
    {
        ERROR("Get %s module psm info failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

#endif /* MB26_USING_NETSERV_OPS */

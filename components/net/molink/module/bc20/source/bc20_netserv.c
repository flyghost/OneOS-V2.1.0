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
 * @file        bc20_netserv.c
 *
 * @brief       bc20 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc20_netserv.h"
#include "bc20.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "bc20.netserv"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef BC20_USING_NETSERV_OPS

#define BC20_EARFCN_MAX        (65535)
#define BC20_PCI_MAX           (0x1F7)

os_err_t bc20_set_attach(mo_object_t *module, os_uint8_t attach_stat)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 85 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%hhu", attach_stat);
}

os_err_t bc20_get_attach(mo_object_t *module, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 85 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT:%hhu", attach_stat) <= 0)
    {
        ERROR("Get %s module attach state failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc20_set_reg(mo_object_t *module, os_uint8_t reg_n)
{
    /* TODO currently, all module only support reg_n:0 */
#if 0
    if (0 != reg_n)
    {
        WARN("%s module only support reg_n = 0", __func__);
        return OS_EINVAL;
    }
#endif

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%hhu", reg_n);
}

os_err_t bc20_get_reg(mo_object_t *module, eps_reg_info_t *info)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG:%hhu,%hhu", &info->reg_n, &info->reg_stat) <= 0)
    {
        ERROR("Get %s module register state failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc20_set_cgact(mo_object_t *module, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%hhu,%hhu", act_stat, cid);
}

os_err_t bc20_get_cgact(mo_object_t *module, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT:%hhu,%hhu", cid, act_stat) <= 0)
    {
        ERROR("Get %s module cgact state failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc20_get_csq(mo_object_t *module, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    /* bc20 dosen't support ber, 99 will be set forever */
    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ:%hhu,%hhu", rssi, ber) <= 0)
    {
        ERROR("Get %s module signal quality failed", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t bc20_get_radio(mo_object_t *module, radio_info_t *radio_info)
{
    at_parser_t *parser = &module->parser;

    memset(radio_info, 0, sizeof(radio_info_t));

    int  at_ret         =  0;
    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+QENG=0");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    /**
     *  <mode>=0:
     *  +QENG: 0,<sc_EARFCN>,<sc_EARFCN_offset>,<sc_pci>,<sc_cellID>,[<sc_RSRP>],[<sc_RSRQ>],[<sc_RSSI>],[<sc_SINR>],
     *           <sc_band>,<sc_TAC>,[<sc_ECL>],[<sc_Tx_pwr>],<operation_mode> 
     * [+QENG: 1,<nc_EARFCN>,<nc_EARFCN_offset>,<nc_pci>,<nc_RSRP>,[…]]
     *   SINR: Signal to Interference plus Noise Ratio
     *   SNR : Signal Noise Ratio
     * */
    at_ret = at_resp_get_data_by_kw(&resp, "+QENG: 0", "+QENG: 0,%d,%*d,%*d,\"%[^\"]\",%*d,%d,%*d,%d,%*d,%*[^,],%d,%d,",
                                                        &radio_info->earfcn, 
                                                         radio_info->cell_id, 
                                                        &radio_info->rsrq, 
                                                        &radio_info->snr,
                                                        &radio_info->ecl, 
                                                        &radio_info->signal_power);
    if (0 >= at_ret)
    {
        ERROR("Get %s module signal power failed", module->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

#endif /* BC20_USING_NETSERV_OPS */

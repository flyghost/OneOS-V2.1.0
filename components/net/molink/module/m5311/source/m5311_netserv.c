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
 * @file        m5311_netserv.c
 *
 * @brief       m5311 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5311_netserv.h"
#include "m5311.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "m5311.netserv"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef M5311_USING_NETSERV_OPS

#define M5311_PSM_QUOTES_LEN      (2)
#define M5311_EARFCN_MAX          (262143)
#define M5311_PCI_MAX             (503)
#define M5311_EARFCN_OFFSET_MAX   (4)
#define M5311_NETSERV_TIMEOUT_DFT (10 * OS_TICK_PER_SECOND)

os_err_t m5311_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 90 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%hhu", attach_stat);
}

os_err_t m5311_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(0 >= at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT: %hhu", attach_stat))
    {
        ERROR("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%hhu", reg_n);
}

os_err_t m5311_get_reg(mo_object_t *self, eps_reg_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG: %hhu,%hhu", &info->reg_n, &info->reg_stat))
    {
        ERROR("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 90 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%hhu,%hhu", act_stat, cid);
}

os_err_t m5311_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT: %hhu,%hhu", cid, act_stat))
    {
        ERROR("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ: %hhu,%hhu", rssi, ber))
    {
        ERROR("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
#define M5311_NETSTAT_REGEX "*ENGINFOSC: %hd,%*[^\"]\"%[^\"]\"\n,%*d,%d,%*d,%d,%*d,%*[^,],%hhu,"

    at_parser_t *parser = &self->parser;

    memset(radio_info, 0, sizeof(radio_info_t));
    
    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT*ENGINFO=0");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    
    /* return eg: *ENGINFOSC: 3684,2,131,"85B0F15"\n,-107,-13,-93,-3,8,"5A21",0, */
    /* <sc_earfcn>,<sc_earfcn_offset>,<sc_pci>,<sc_cellid>,
    [<sc_rsrp>],[<sc_rsrq>],[<sc_rssi>],[<sc_snr>],<sc_band>,<sc_tac>,[<sc_ecl>],[<sc_tx_pwr>] */
    if (0 >= at_resp_get_data_by_kw(&resp, "*ENGINFOSC:", M5311_NETSTAT_REGEX, 
                                                        &radio_info->earfcn,
                                                        &radio_info->cell_id, 
                                                        &radio_info->rsrq, 
                                                        &radio_info->snr,
                                                        &radio_info->ecl))
    {
        ERROR("Get %s module radio_info failed", self->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

os_err_t m5311_set_psm(mo_object_t *self, mo_psm_info_t info)
{
    at_parser_t *parser = &self->parser;
    
    /* M5311 needs double quotes */
    os_int8_t periodic_rau[PSM_TIMER_MAX_STR_LEN + M5311_PSM_QUOTES_LEN]     = {0};
    os_int8_t gprs_ready_timer[PSM_TIMER_MAX_STR_LEN + M5311_PSM_QUOTES_LEN] = {0};
    os_int8_t periodic_tau[PSM_TIMER_MAX_STR_LEN + M5311_PSM_QUOTES_LEN]     = {0};
    os_int8_t active_time[PSM_TIMER_MAX_STR_LEN + M5311_PSM_QUOTES_LEN]      = {0};

    /* only insert double quotes when the value is valid */
    if (0 != strlen(info.periodic_rau))
        snprintf((char *)periodic_rau, sizeof(periodic_rau), "\"%s\"", info.periodic_rau);
    if (0 != strlen(info.gprs_ready_timer))
        snprintf((char *)gprs_ready_timer, sizeof(gprs_ready_timer), "\"%s\"", info.gprs_ready_timer);
    if (0 != strlen(info.periodic_tau))
        snprintf((char *)periodic_tau, sizeof(periodic_tau), "\"%s\"", info.periodic_tau);
    if (0 != strlen(info.active_time))
        snprintf((char *)active_time, sizeof(active_time), "\"%s\"", info.active_time);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    return at_parser_exec_cmd(parser, &resp, "AT+CPSMS=%d,%s,%s,%s,%s", 
                                              info.psm_mode, 
                                              periodic_rau,
                                              gprs_ready_timer,
                                              periodic_tau,
                                              active_time);
}

os_err_t m5311_get_psm(mo_object_t *self, mo_psm_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

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

os_err_t m5311_set_edrx_cfg(mo_object_t *self, mo_edrx_cfg_t cfg)
{
    at_parser_t *parser = &self->parser;
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    return at_parser_exec_cmd(parser, &resp, "AT+CEDRXS=%d,5,%s", 
                                              cfg.mode, 
                                              cfg.edrx.req_edrx_value);
}

os_err_t m5311_get_edrx_cfg(mo_object_t *self, mo_edrx_t *edrx_local)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CEDRXS?"))
    {
        ERROR("%s fail: AT+CEDRXS? cmd exec fail.", __func__);
        return OS_ERROR;
    }

    memset(edrx_local, 0, sizeof(mo_edrx_t));

    if (0 >= at_resp_get_data_by_kw(&resp, "+CEDRXS", "+CEDRXS: %d,\"%[^\"]", 
                                                       &edrx_local->act_type, 
                                                        edrx_local->req_edrx_value))
    {
        ERROR("Get %s module edrx local config failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_get_edrx_dynamic(mo_object_t *self, mo_edrx_t *edrx_dynamic)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CEDRXRDP"))
    {
        ERROR("%s fail: AT+CEDRXRDP cmd exec fail.", __func__);
        return OS_ERROR;
    }

    memset(edrx_dynamic, 0, sizeof(mo_edrx_t));

    if (0 >= at_resp_get_data_by_kw(&resp, "+CEDRXRDP", "+CEDRXRDP: %d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", 
                                                        &edrx_dynamic->act_type, 
                                                         edrx_dynamic->req_edrx_value,
                                                         edrx_dynamic->nw_edrx_value,
                                                         edrx_dynamic->paging_time_window))
    {
        ERROR("Get %s module edrx dynamic config failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

#if 0
os_err_t m5311_set_band(mo_object_t *self, char band_list[], os_uint8_t num)
{
    /* invalid input check */
    if (0 == num)
    {
        ERROR("%s input invalid band count num.", __func__);
        return OS_EINVAL;
    }

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_ERROR;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    char exec_cmd[512] = "AT*CMBAND=";
    for (int idex=0; idex < num-1; idex++)
    {
        sprintf(exec_cmd, "%s%d,", exec_cmd, band_list[idex]);
    }
    sprintf(exec_cmd, "%s%d", exec_cmd, band_list[num - 1]);

    result = at_parser_exec_cmd(parser, &resp, exec_cmd);
    if (OS_EOK != result)
    {
        ERROR("%s fail: %s cmd exec fail.", __func__, exec_cmd);
    }

    return result;
}

os_err_t m5311_set_earfcn(mo_object_t *self, mo_earfcn_t earfcn)
{
    /* AT*FRCLLCK NOT SUPPORT M5311_CM */

    if (M5311_EARFCN_MAX        < earfcn.earfcn ||
        M5311_PCI_MAX           < earfcn.pci    ||
        M5311_EARFCN_OFFSET_MAX < earfcn.earfcn_offset)
    {
        ERROR("%s input invalid.", __func__);
        return OS_EINVAL;
    }

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_ERROR;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    /* TODO TEST THIS FORMAT */
    result = at_parser_exec_cmd(parser, &resp, "AT*FRCLLCK=%u,%u,%u,%u", 
                                                earfcn.mode, 
                                                earfcn.earfcn,
                                                earfcn.earfcn_offset,
                                                earfcn.pci);
    if (OS_EOK != result)
    {
        ERROR("%s fail: AT*FRCLLCK exec fail.", __func__);
    }

    return result;
}

os_err_t m5311_get_earfcn(mo_object_t *self, mo_earfcn_t *earfcn)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT*FRCLLCK"))
    {
        ERROR("%s fail: AT*FRCLLCK cmd exec fail.", __func__);
        return OS_ERROR;
    }

    memset(earfcn, 0, sizeof(mo_earfcn_t));

    /* TODO TEST RETURN VALUE ON MUTIPLE BAND MODULE */
    /* return eg. *FRCLLCK: <lock>[,<earfcn>,<earfcn_offset>[,<pci>]] */
    if (0 >= at_resp_get_data_by_kw(&resp, "*FRCLLCK:", "*FRCLLCK: %hhu,%u,%hu,%hhu", 
                                                         &earfcn->mode, 
                                                         &earfcn->earfcn,
                                                         &earfcn->earfcn_offset,
                                                         &earfcn->pci))
    {
        ERROR("%s get earfcn( frequency lock info) failed", __func__);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t m5311_clear_plmn(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;
    os_uint32_t  status = 0;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETSERV_TIMEOUT_DFT};

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CLPLMN"))
    {
        ERROR("%s fail: AT+CLPLMN cmd exec fail.", __func__);
        return OS_ERROR;
    }

    if (0 >= at_resp_get_data_by_kw(&resp, "+CLPLMN:", "+CLPLMN: %u", &status))
    {
        ERROR("%s failed.", __func__);
        return OS_ERROR;
    }

    if (0 != status)
    {
        ERROR("%s failed with error code[%u]", __func__, status);
    }

    return OS_EOK;
}

#endif /* Put on hold */

void m5311_pdp_urc_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    /* m5311 may return 'NO CARRIER' after 'OK'... vresion:M5311-POSH0S04 */
    WARN("%s-%d: module prompt: \"NO CARRIER\".", __func__, __LINE__);

    return;
}

void m5311_edrx_urc_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_edrx_t edrx_urc;
    memset(&edrx_urc, 0, sizeof(mo_edrx_t));
    sscanf(data, "+CEDRXP: %d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", 
                  (int *)&edrx_urc.act_type,
                   edrx_urc.req_edrx_value,
                   edrx_urc.nw_edrx_value,
                   edrx_urc.paging_time_window);
    INFO("Get %s module edrx urc act_type[%d]", parser->name, edrx_urc.act_type);
    INFO("Get %s module edrx urc req_edrx_value[%s]", parser->name, edrx_urc.req_edrx_value);
    INFO("Get %s module edrx urc nw_edrx_value[%s]", parser->name, edrx_urc.nw_edrx_value);
    INFO("Get %s module edrx urc paging_time_window[%s]", parser->name, edrx_urc.paging_time_window);

    return;
}

static at_urc_t m5311_netserv_urc_table[] = {
    {.prefix = "NO CARRIER", .suffix = "\r\n", .func = m5311_pdp_urc_handler},
    {.prefix = "+CEDRXP:",   .suffix = "\r\n", .func = m5311_edrx_urc_handler},
};

void m5311_netserv_init(mo_m5311_t *module)
{
    /* Set netserv urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, m5311_netserv_urc_table, sizeof(m5311_netserv_urc_table) / sizeof(at_urc_t));
    return;
}

#endif /* M5311_USING_NETSERV_OPS */

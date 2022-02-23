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
 * @file        ml302_netserv.c
 *
 * @brief       ml302 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ml302_netserv.h"
#include "ml302.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MO_LOG_TAG "ml302.netserv"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef ML302_USING_NETSERV_OPS

os_err_t ml302_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGATT=%d", attach_stat);
}

os_err_t ml302_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if(at_resp_get_data_by_kw(&resp, "+CGATT:", "+CGATT:%hhu", attach_stat) <= 0)
    {
        ERROR("Get %s module attach state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ml302_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%d", reg_n);
}

os_err_t ml302_get_reg(mo_object_t *self, eps_reg_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG:%hhu,%hhu", &info->reg_n, &info->reg_stat) <= 0)
    {
        ERROR("Get %s module register state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ml302_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+CGACT=%d,%d", act_stat, cid);
}

os_err_t ml302_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CGACT:", "+CGACT:%hhu,%hhu", cid, act_stat) <= 0)
    {
        ERROR("Get %s module cgact state failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t ml302_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CSQ");
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+CSQ:", "+CSQ:%hhu,%hhu", rssi, ber) <= 0)
    {
        ERROR("Get %s module signal quality failed", self->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

#define GET_ML302_RSSI(rxlev)   (0 - (63 - rxlev + 48))
#define ML302_MODULE_NET_TYPE   5

os_err_t ml302_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info)  
{  
    os_err_t     ret       = OS_EOK;  
    os_size_t    i         = 0;  
    cell_info_t *cell_info = OS_NULL;  
    char        *temp_buff = OS_NULL;  
    os_uint32_t  cell_num  = 0;  
    os_uint32_t  frequency = 0;  
    os_uint32_t  rsrp      = 0;  
    os_uint32_t  rsrq      = 0;  
    os_uint32_t  rxlev     = 0;  
    os_uint32_t  pcid      = 0;  
    char         imsi[20];  
    char         earfcn[10];  
    os_uint32_t  roming_flag;  
    os_uint32_t  bandinfo;   
    at_parser_t *parser = &self->parser;  
  
    if (parser == OS_NULL)  
    {  
        ERROR("ML302 ping: at parser is NULL.");  
        return OS_ERROR;  
    }  
    if(!onepos_cell_info)  
    {  
        ERROR("input param is error!");  
        return OS_ERROR;  
    }  
  
    at_resp_t resp = {.buff      = os_calloc(1, 1024),  
                      .buff_size = 1024,  
                      .timeout   = 50 * OS_TICK_PER_SECOND  
                     };  
  
    if (OS_NULL == resp.buff)  
    {  
        ERROR("Calloc ml302 cell info response memory failed!");  
        ret = OS_ENOMEM;  
        goto __exit;  
    }  
  
    /* neighbor cell */  
    if (at_parser_exec_cmd(parser, &resp, "AT+CCED=0,2") < 0)  
    {  
        ret = OS_ERROR;  
        ERROR("AT cmd exec fail: AT+CCED=0,2\n");  
        goto __exit;  
    }  
    DEBUG("resp->line_counts : %d\r\n", resp.line_counts);  
  
    cell_info = os_calloc((resp.line_counts), sizeof(cell_info_t)); 
    ERROR("resp.line_counts =%d",resp.line_counts);    
    if(NULL == cell_info)  
    {  
        ERROR("malloc cell_info is null");  
        ret = OS_ENOMEM;  
        return ret;  
    }  
  
    INFO("                MCC         MNC          CID         LAC       RSSI\n");  
    INFO("           ------------ ------------ ------------ ------------ ----\n");  
  
    for (i = 1; i < resp.line_counts; i++)  
    {  
        temp_buff = (char*)at_resp_get_line(&resp, i);  
        if(strlen(temp_buff) > 25) {  
            sscanf(temp_buff,  
                   "+CCED:LTE neighbor cell:%u,%u,%u,%u,%u,%u,%u,%u,%u",  
                   &cell_info[cell_num].mcc,  
                   &cell_info[cell_num].mnc,  
                   &frequency,  
                   &cell_info[cell_num].cid,  
                   &rsrp,  
                   &rsrq,  
                   &cell_info[cell_num].lac,  
                   &rxlev,  
                   &pcid);  
  
            cell_info[cell_num].ss = GET_ML302_RSSI(rxlev);  
            if(cell_info[cell_num].mcc)  
            {  
                INFO("cell_info: %-12u %-12u %-12u %-12u %-4d\n",  
                          cell_info[cell_num].mcc, cell_info[cell_num].mnc, cell_info[cell_num].cid,  
                          cell_info[cell_num].lac, cell_info[cell_num].ss);  
                cell_num ++;  
            }  
        }  
    }  
    /* main cell */  
    if (at_parser_exec_cmd(parser, &resp, "AT+CCED=0,1") < 0)  
    {  
        ret = OS_ERROR;  
        os_free(cell_info);  
        ERROR("AT cmd exec fail: AT+CCED=0,1\n");  
        goto __exit;  
    }  
    DEBUG("resp->line_counts : %d\r\n", &resp.line_counts);  
  
    temp_buff = (char*)at_resp_get_line(&resp, 1);  
    if(strlen(temp_buff) > 25)  
    {  
        sscanf(temp_buff,  
               "+CCED:LTE current cell:%u,%u,%[^,],%u,%u,%*[^,],%[^,],%u,%u,%u,%u,%u,%u",  
               &cell_info[cell_num].mcc,  
               &cell_info[cell_num].mnc,  
               imsi,  
               &roming_flag,  
               &bandinfo,   
               earfcn,  
               &cell_info[cell_num].cid,  
               &rsrp,  
               &rsrq,  
               &cell_info[cell_num].lac,  
               &rxlev,  
               &pcid);  
  
        cell_info[cell_num].ss = GET_ML302_RSSI(rxlev);  
        INFO("cell_info: %-12u %-12u %-12u %-12u %-4d\n",  
                  cell_info[cell_num].mcc, cell_info[cell_num].mnc, cell_info[cell_num].cid,  
                  cell_info[cell_num].lac, cell_info[cell_num].ss);  
  
    }  
    else  
    {  
        cell_num --;  
    }  
    INFO("           ------------ ------------ ------------ ------------ ----\n");  
  
    onepos_cell_info->cell_num = ++cell_num;  
    onepos_cell_info->cell_info = cell_info;  
    onepos_cell_info->net_type = ML302_MODULE_NET_TYPE;  
  
__exit:  
  
    if (resp.buff != OS_NULL)  
    {  
        os_free(resp.buff);  
    }  
  
    return ret;  
}  

#endif /* ml302_USING_NETSERV_OPS */


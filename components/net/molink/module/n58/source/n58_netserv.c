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
 * @file        n58_netserv.c
 *
 * @brief       n58 module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-30   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "n58_netserv.h"
#include "n58.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MO_LOG_TAG "n58.netserv"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef N58_USING_NETSERV_OPS

os_err_t n58_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CGATT=%d", attach_stat);

    if (result != OS_EOK)
    {
        return result;
    }

    if (1 == attach_stat)
    {
        os_task_msleep(3000);
        result = at_parser_exec_cmd(parser, &resp, "AT+XIIC=1");
    }

    return result;
}

os_err_t n58_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

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

os_err_t n58_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "AT+CEREG=%d", reg_n);
}

os_err_t n58_get_reg(mo_object_t *self, eps_reg_info_t *info)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

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

os_err_t n58_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

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

#define N58_MODULE_NET_TYPE   5

os_err_t n58_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info)  
{  
    os_err_t     ret       = OS_EOK;  
    cell_info_t *cell_info = OS_NULL;  
    char        *temp_buff = OS_NULL;  
    os_uint32_t  cell_num  = 0;   
    
    at_parser_t *parser = &self->parser;    
    if (parser == OS_NULL)  
    {  
        ERROR("N58 get_cell_info: at parser is NULL.");  
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
        ERROR("Calloc n58 cell info response memory failed!");  
        ret = OS_ENOMEM;  
        goto __exit;  
    }  
  
    /* neighbor cell */  
    if (at_parser_exec_cmd(parser, &resp, "AT+NETDMSG") < 0)  
    {  
        ret = OS_ERROR;  
        ERROR("AT cmd exec fail: AT+MCELL=0,26\n");  
        goto __exit;  
    }  
    DEBUG("resp->line_counts : %d\r\n", resp.line_counts);  
  
    cell_info = os_calloc(1, sizeof(cell_info_t));   
    if(NULL == cell_info)  
    {  
        ERROR("malloc cell_info is null");  
        ret = OS_ENOMEM;  
        return ret;  
    }  
  
    INFO("                MCC         MNC          CID         LAC       RSSI\n");  
    INFO("           ------------ ------------ ------------ ------------ ----\n");  
  
    temp_buff = (char*)at_resp_get_line(&resp, 1);  
    if(strlen(temp_buff) > 25) 
    {  
        sscanf(temp_buff,  
               "+NETDMSG: \"%u+%u\", 0x%x, 0x%x,%*[^,],%*[^,],%*[^,],%*[^,], %d,%*[^\r]",  
               &cell_info[cell_num].mcc,  
               &cell_info[cell_num].mnc,  
               &cell_info[cell_num].lac,  
               &cell_info[cell_num].cid,   
               &cell_info[cell_num].ss
               );  
  
        if(cell_info[cell_num].mcc)  
        {  
            INFO("cell_info: %-12u %-12u %-12u %-12u %-4d\n",  
                      cell_info[cell_num].mcc, cell_info[cell_num].mnc, cell_info[cell_num].cid,  
                      cell_info[cell_num].lac, cell_info[cell_num].ss);  
            cell_num ++;  
        }  
    }  
      
    /* main cell */  
  
    INFO("           ------------ ------------ ------------ ------------ ----\n");  
  
    onepos_cell_info->cell_num = ++cell_num;  
    onepos_cell_info->cell_info = cell_info;  
    onepos_cell_info->net_type = N58_MODULE_NET_TYPE;  
  
__exit:  
  
    if (resp.buff != OS_NULL)  
    {  
        os_free(resp.buff);  
    }  
  
    return ret;  
}  

#endif /* N58_USING_NETSERV_OPS */


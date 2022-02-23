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
 * @file        ec20_ppp.c
 *
 * @brief       ec20 module link kit ppp api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ec20_ppp.h"
#include "mo_general.h"
#include "mo_netserv.h"
#include "mo_factory.h"

#define DBG_EXT_TAG "ec20.ppp"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#ifdef EC20_USING_PPP_OPS

extern os_err_t at_parser_exec_unlock(at_parser_t *parser);
extern os_err_t at_parser_unbind_device(at_parser_t *parser);
extern os_err_t at_parser_rebind_device(at_parser_t *parser);

os_err_t ec20_ppp_init(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    os_err_t result     = OS_EOK;
    at_parser_t *parser = &module->parser;

    char tmp_data[20]   = {0};
    char APN[10]        = {0};
    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    /* check net registration status */
    eps_reg_info_t info;
    memset(&info, 0, sizeof(eps_reg_info_t));

    result = mo_get_reg(module, &info);
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s exec mo_get_reg failed!", __func__);
        goto __exit;
    }

    if (1 != info.reg_stat)
    {
        LOG_EXT_E("%s mo_get_reg state:%d, Not registered", __func__, info.reg_stat);
        goto __exit;
    }

    /* Query current Network Operator */
    result = at_parser_exec_cmd(parser, &resp, "AT+COPS?");
    
    if (result != OS_EOK)
    {
        LOG_EXT_E("%s query APN failed!", __func__);
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "+COPS:", "+COPS: %*[^\"]\"%[^\"]", tmp_data) <= 0)
    {
        LOG_EXT_E("%s get APN failed!", __func__);
        goto __exit;
    }

    if (strcmp(tmp_data, "CHINA MOBILE") == 0)
    {
        strncpy(APN, "CMNET", strlen("CMNET"));
    }
    else if (strcmp(tmp_data, "CHN-UNICOM") == 0)
    {
        strncpy(APN, "UNINET", strlen("UNINET"));
    }
    else if (strcmp(tmp_data, "CHN-CT") == 0)
    {
        strncpy(APN, "CTNET", strlen("CTNET"));
    }

    /* Set PDP context */
    memset(resp_buff, 0, sizeof(resp_buff));
    result = at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IP\",\"%s\"", APN);
    
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s set apn failed!", __func__);
        goto __exit;
    }
    else LOG_EXT_I("%s set apn success", __func__);

    /* deact pdp max timeout:40s */
    resp.timeout = 40 * OS_TICK_PER_SECOND;
    result = at_parser_exec_cmd(parser, &resp, "AT+QIDEACT=1");
    
    if (result != OS_EOK)
    {
        LOG_EXT_E("%s deactice PDP context (QIDEACT) failed!", __func__);
        goto __exit;
    }

    /* act pdp max timeout:150s */
    resp.timeout = 150 * OS_TICK_PER_SECOND;
    result = at_parser_exec_cmd(parser, &resp, "AT+QIACT=1");
    
    if (result != OS_EOK)
    {
        LOG_EXT_E("%s actice PDP context (QIACT) failed!", __func__);
        goto __exit;
    }

__exit:
    if (result != OS_EOK)
    {
        LOG_EXT_E("EC20 ppp init failed");
    }

    return result;
}

os_err_t ec20_ppp_dial(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;
    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff,
                      .line_num = 2,
                      .buff_size = sizeof(resp_buff), 
                      .timeout = OS_TICK_PER_SECOND};

    /*  Start PPP dialing by ATD*99# */
    result = at_parser_exec_cmd(parser, &resp, "ATD*99#");
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s PPP dialing failed!", __func__);
        goto __exit;
    }

    if (OS_NULL == at_resp_get_line_by_kw(&resp, "CONNECT"))
    {
        LOG_EXT_E("%s ppp failed!", __func__);
        result = OS_ERROR;
        goto __exit;
    }

    /* unbind device for ppp using */
    result = at_parser_unbind_device(parser);
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s at unbind device failed!", __func__);
        goto __exit;
    }

__exit:
    if (OS_EOK != result)
    {
        LOG_EXT_I("%s failed!", __func__);
    }
    else
    {
        LOG_EXT_I("%s Into PPP data mode!", __func__);
    }
    
    return result;
}

os_err_t ec20_ppp_exit(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    /* ec20 needs more than 1s around +++ input */
    const int EC20_PPP_EXIT_DELAY_MS = 2000;
    
    os_err_t     result = OS_EOK;
    at_parser_t *parser = &module->parser;

    /* rebind device for ppp using */
    result = at_parser_rebind_device(parser);
    if (OS_EOK != result)
    {
        LOG_EXT_E("%s at rebind device failed!", __func__);
        goto __exit;
    }
    
    /* exit ppp, do not send anything to device in exit process */

    /* 1) Do not input any character within 1s or longer before inputting “+++”. */
    os_task_mdelay(EC20_PPP_EXIT_DELAY_MS);
    
    /* 2) Input “+++” within 1s, and no other characters can be inputted during the time. */
    os_device_write_block(parser->device, 0, "+++", 3);
    
    /* 3) Do not input any character within 1s after “+++” has been inputted. */
    os_task_mdelay(EC20_PPP_EXIT_DELAY_MS);

    /* unlock at_parser_exec lock */
    at_parser_exec_unlock(parser);
    
    /* test result by at cmd */
    for (int i = 0; i < 5; i++)
    {
        result = mo_at_test(module);
        if (OS_EOK == result) break;
    }
    
__exit:

    if (OS_EOK != result)
    {
        LOG_EXT_E("%s ppp exit failed", __func__);
    }
    else 
    {
        LOG_EXT_I("%s ppp exit success, into command mode.", __func__);
    }

    
    return result;
}



#endif /* EC20_USING_PPP_OPS */

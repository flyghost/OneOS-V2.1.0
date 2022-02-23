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
 * @file        mb26_ctm2m.h
 *
 * @brief       mb26 module link kit ctm2m api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_ctm2m.h"
#include "mb26_ctm2m.h"
#include "os_mq.h"
#include "os_task.h"
#include "os_mutex.h"
#include "os_sem.h"
#include "mo_lib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef MB26_USING_CTM2M_OPS

#define MO_LOG_TAG "mb26.ctm2m"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#define MB26_CTM2M_SEM_NAME            "mb26_ctm2m_sem"
#define MB26_CTM2M_MUTEX_NAME          "mb26_ctm2m_mt"
#define MB26_CTM2M_MQ_NAME             "mb26_ctm2m_mq"
#define MB26_CTM2M_TASK_NAME           "mb26_ctm2m"

typedef enum mb26_ctm2m_init_flag
{
    MB26_CTM2M_INIT_FLAG_PARM = 1,
    MB26_CTM2M_INIT_FLAG_HANDLE,
    MB26_CTM2M_INIT_FLAG_SEM,
    MB26_CTM2M_INIT_FLAG_MUTEX,
    MB26_CTM2M_INIT_FLAG_MQ,
    MB26_CTM2M_INIT_FLAG_TASK,  
    MB26_CTM2M_INIT_FLAG_LIST, 
    MB26_CTM2M_INIT_FLAG_EXEC, 
} mb26_ctm2m_init_flag_t;

#define MB26_CTM2M_INVALID_DFT          (-1)
#define MB26_CTM2M_IPV4_MAX_STR_LEN     (15)
#define MB26_CTM2M_PORT_MAX             (0xFFFF)
#define MB26_CTM2M_OBJECT_MAX           (0xFFFF)                /* LIERDA SPEC: 0xFFFF; OMA SPEC: 42768 */
#define MB26_CTM2M_RESOURCE_MAX         (0xFFFF)                /* LIERDA SPEC: 0xFFFF; OMA SPEC: 32768 */
#define MB26_CTM2M_INSTANCE_MAX         (0xFFFF)
#define MB26_CTM2M_MSG_ID_MAX           (0x7FFFFFFF)            /* SPEC: 2,147,483,647 */
#define MB26_CTM2M_LIFETIME_SEC_MAX     (0x7FFFFFFF)            /* SPEC: 2,147,483,647 */
#define MB26_CTM2M_LIFETIME_SEC_MIN     (300)
#define MB26_CTM2M_STR_BUFF_DEF         (128)
#define MB26_CTM2M_SEND_MAX_LEN         (1024)
#define MB26_CTM2M_URI_STR_MAX_LEN      (18)
#define MB26_CTM2M_STR_LEN_DEF          (20)
// #define MB26_CTM2M_LIST_NUM_MAX        (5)

/* ============================kconfig_options============================= */
#ifndef MB26_CTM2M_TASK_STACK_SIZE     
#define MB26_CTM2M_TASK_STACK_SIZE          (2048)
#endif /* MB26_CTM2M_TASK_STACK_SIZE */

#ifndef MB26_CTM2M_TASK_PRIORITY
#define MB26_CTM2M_TASK_PRIORITY            (0x10)
#endif /* MB26_CTM2M_TASK_PRIORITY */

#ifndef MB26_CTM2M_RESP_BUFF_SIZE
#define MB26_CTM2M_RESP_BUFF_SIZE           (128)
#endif /* MB26_CTM2M_RESP_BUFF_SIZE */

#ifndef MB26_CTM2M_MQ_THRESHOLD
#define MB26_CTM2M_MQ_THRESHOLD             (5)
#endif /* MB26_CTM2M_MQ_THRESHOLD */

/* ===========================kconfig_option_tail========================== */

static ctm2m_t *handler = OS_NULL;

static os_err_t mb26_ctm2m_task_create(ctm2m_t *handle);
static void     mb26_ctm2m_task_destroy(ctm2m_t *handle);

typedef struct mb26_ctm2m_urc
{
    const char   *prefix;                            /* URC data prefix      */
    os_err_t (*func) (ctm2m_t *handle, const char *data, os_size_t size); 
                                                     /* URC hanlder function */
} mb26_ctm2m_urc_t;

typedef struct mb26_ctm2m_msg
{
    const char   *data;                              /* URC data */
    os_size_t     len;                               /* URC data len */
    os_err_t (*func) (ctm2m_t *handle, const char *data, os_size_t size); 
                                                     /* URC hanlder function */
} mb26_ctm2m_msg_t;

os_err_t mb26_ctm2m_lock(os_mutex_t *mutex)
{
    OS_ASSERT(OS_NULL != mutex);
    return os_mutex_lock(mutex, OS_WAIT_FOREVER);
}

os_err_t mb26_ctm2m_unlock(os_mutex_t *mutex)
{
    OS_ASSERT(OS_NULL != mutex);
    return os_mutex_unlock(mutex);
}

ctm2m_t *mb26_ctm2m_create(mo_object_t *module, ctm2m_create_parm_t parm)
{
    OS_ASSERT(OS_NULL != &module->parser);

    os_int32_t   result = OS_EOK;
    at_parser_t *parser = &module->parser;
    int          ret    = MB26_CTM2M_INVALID_DFT;

    char cmd[MB26_CTM2M_STR_BUFF_DEF]         = {0};
    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    /* 1. Parm checking process */
    if (OS_NULL != handler)
    {
        ERROR("%s alreay have an handler, destroy it first.", __func__);
        result = MB26_CTM2M_INIT_FLAG_PARM;
        goto __exit;
    }

    if (OS_NULL == parm.server_ip.data || MB26_CTM2M_IPV4_MAX_STR_LEN < parm.server_ip.len)
    {
        ERROR("%s server ip address invalid", __func__);
        result = MB26_CTM2M_INIT_FLAG_PARM;
        goto __exit;
    }

    if (MB26_CTM2M_PORT_MAX < parm.port)
    {
        ERROR("%s server port invalid", __func__);
        result = MB26_CTM2M_INIT_FLAG_PARM;
        goto __exit;
    }

    if (MB26_CTM2M_LIFETIME_SEC_MIN > parm.lifetime || MB26_CTM2M_LIFETIME_SEC_MAX < parm.lifetime)
    {
        ERROR("%s lifetime invalid [%d,%d]", __func__, MB26_CTM2M_LIFETIME_SEC_MIN, MB26_CTM2M_LIFETIME_SEC_MAX);
        result = MB26_CTM2M_INIT_FLAG_PARM;
        goto __exit;
    }
    
    if (OS_NULL == parm.notify_cb || OS_NULL == parm.request_cb || OS_NULL == parm.receive_cb)
    {
        ERROR("%s user must registering msg receive callback", __func__);
        result = MB26_CTM2M_INIT_FLAG_PARM;
        goto __exit;
    }

    /* 2. Handle alloc process */
    handler = os_calloc(1, sizeof(ctm2m_t));
    if (OS_NULL == handler)
    {
        ERROR("%s-%d:no enough memory.", __func__, __LINE__);
        result = MB26_CTM2M_INIT_FLAG_HANDLE;
        goto __exit;
    }

    /* 3. Handle init& user cb register process */
    handler->module     = module;
    handler->notify_cb  = parm.notify_cb;
    handler->request_cb = parm.request_cb;
    handler->receive_cb = parm.receive_cb;

    /* 4. sem init process */
    handler->ctm2m_sem = os_sem_create(MB26_CTM2M_SEM_NAME, 0, OS_SEM_MAX_VALUE);
    if (OS_NULL == handler->ctm2m_sem)
    {   
        ERROR("%s-%d:ctm2m sem create failed.", __func__, __LINE__);
        result = MB26_CTM2M_INIT_FLAG_SEM;
        goto __exit;
    }

    /* 5. mutex init process */
    handler->ctm2m_mutex = os_mutex_create(MB26_CTM2M_MUTEX_NAME, OS_FALSE);
    if (OS_NULL == handler->ctm2m_mutex)
    {   
        ERROR("%s-%d:ctm2m mutex create failed.", __func__, __LINE__);
        result = MB26_CTM2M_INIT_FLAG_MUTEX;
        goto __exit;
    }

    /* 6. message queue init process */
    handler->ctm2m_mq = os_mq_create(MB26_CTM2M_MQ_NAME,
                                    sizeof(mb26_ctm2m_msg_t),
                                    MB26_CTM2M_MQ_THRESHOLD);
    if (OS_NULL == handler->ctm2m_mq)
    {
        ERROR("%s-%d:message queue init failed.", __func__, __LINE__);
        result = MB26_CTM2M_INIT_FLAG_MQ;
        goto __exit;
    }

    /* 7. URC handler task create process */
    if (OS_EOK != mb26_ctm2m_task_create(handler))
    {
        ERROR("%s-%d:ctm2m task create failed.", __func__, __LINE__);
        result = MB26_CTM2M_INIT_FLAG_TASK;
        goto __exit;
    }

    /* 10. execute AT cmd */
    /* AT+CTM2MSETPM=<Sever_IP>,<Port>,<Lifetime>[,<Object_Instance_List>] */
    if (0 != parm.obj_ins_list.len && OS_NULL != parm.obj_ins_list.data)
    {
        snprintf(cmd, MB26_CTM2M_STR_BUFF_DEF - 1, "AT+CTM2MSETPM=%s,%d,%d,\"%s\"", parm.server_ip.data, parm.port, parm.lifetime, parm.obj_ins_list.data);
    }
    else
    {
        snprintf(cmd, MB26_CTM2M_STR_BUFF_DEF - 1, "AT+CTM2MSETPM=%s,%d,%d", parm.server_ip.data, parm.port, parm.lifetime);
    }

    if (OS_EOK != at_parser_exec_cmd(parser, &resp, cmd))
    {
        if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2M ERROR:", "+CTM2M ERROR: %d", &ret))
        {
            ERROR("%s-%d:get error code failed", __func__, __LINE__);
        }
        else 
        {
            ERROR("%s-%d:error code[%d]", __func__, __LINE__, ret);
        }
        result = MB26_CTM2M_INIT_FLAG_EXEC;
        goto __exit;
    }

__exit:

    switch (result)
    {
    case OS_EOK:                        break;
    case MB26_CTM2M_INIT_FLAG_EXEC:     /* exec failed, deinit all */
    case MB26_CTM2M_INIT_FLAG_LIST:     os_task_destroy(handler->ctm2m_task);
    case MB26_CTM2M_INIT_FLAG_TASK:     os_mq_destroy(handler->ctm2m_mq);
    case MB26_CTM2M_INIT_FLAG_MQ:       os_mutex_destroy(handler->ctm2m_mutex);
    case MB26_CTM2M_INIT_FLAG_MUTEX:    os_sem_destroy(handler->ctm2m_sem);
    case MB26_CTM2M_INIT_FLAG_SEM:      {os_free(handler); handler = OS_NULL;}
    case MB26_CTM2M_INIT_FLAG_HANDLE:   break;
    case MB26_CTM2M_INIT_FLAG_PARM:     break;
    default:                            break;
    }

    return OS_EOK == result ? handler : OS_NULL;
}
os_err_t mb26_ctm2m_destroy(ctm2m_t *handle)
{
    // os_err_t result = OS_EOK;

    // result = mb26_ctm2m_deregister(handle);
    // if (OS_EOK != result)
    // {
    //     ERROR("%s-%d:deregister failed, please try again.", __func__, __LINE__);
    //     return result;
    // }

    mb26_ctm2m_task_destroy(handle);

    os_mq_destroy(handle->ctm2m_mq);

    os_mutex_destroy(handle->ctm2m_mutex);

    os_sem_destroy(handle->ctm2m_sem);
    
    os_free(handle);
    handler = OS_NULL;

    return OS_EOK;
}

os_err_t mb26_ctm2m_set_ue_cfg(ctm2m_t *handle, ctm2m_ue_cfg_t cfg)
{
    os_err_t     result = OS_EOK;
    at_parser_t *parser = &handle->module->parser;
    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};
    
    result = at_parser_exec_cmd(parser, &resp, "AT+CTM2MSETMOD=%d,%d", cfg.mode, cfg.cfg);
    if (OS_EOK != result)
    {
        int ret = 0;
        ERROR("%s-%d:set ue cfg failed", __func__, __LINE__);
        if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2M ERROR:", "+CTM2M ERROR: %d", &ret))
        {
            ERROR("%s-%d:get error code failed", __func__, __LINE__);
        }
        else 
        {
            ERROR("%s-%d:error code[%d]", __func__, __LINE__, ret);
        }
    }
    return result;
}

os_err_t mb26_ctm2m_get_ue_cfg(ctm2m_t *handle, ctm2m_ue_info_t *cfg_info)
{
    os_err_t     result = OS_EOK;
    at_parser_t *parser = &handle->module->parser;
    
    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    
    os_int32_t   auth_mode = 0;
    os_int32_t   tau_timer_mode = 0;
    os_int32_t   uq_mode   = 0;
    os_int32_t   ce_mode   = 0;
    os_int32_t   heartbeat_mode = 0; 
    os_int32_t   wakeup_mode = 0;
    os_int32_t   protocol_mode = 0;
    
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};
    
    result = at_parser_exec_cmd(parser, &resp, "AT+CTM2MSETMOD?");
    if (OS_EOK != result)
    {
        ERROR("%s-%d:get ue cfg failed", __func__, __LINE__);
        goto __exit;
    }
    if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2MSETMOD:", 
                                    "+CTM2MSETMOD: %d,%d,%d,%d,%d,%d,%d", 
                                    &auth_mode,
                                    &tau_timer_mode,
                                    &uq_mode,
                                    &ce_mode,
                                    &heartbeat_mode,
                                    &wakeup_mode,
                                    &protocol_mode))
    {
        ERROR("%s-%d:get ue cfg failed", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    cfg_info->auth_mode      = (ctm2m_auth_cfg_t)auth_mode;
    cfg_info->tau_timer_mode = (ctm2m_tautimer_cfg_t)tau_timer_mode;
    cfg_info->uq_mode        = (ctm2m_uq_cfg_t)uq_mode;
    cfg_info->ce_mode        = (ctm2m_ce_cfg_t)ce_mode;
    cfg_info->heartbeat_mode = (ctm2m_heartbeat_cfg_t)heartbeat_mode;
    cfg_info->wakeup_mode    = (ctm2m_wakeup_cfg_t)wakeup_mode;
    cfg_info->protocol_mode  = (ctm2m_protocol_cfg_t)protocol_mode;

    DEBUG("%s-%d:get ue cfg:%d,%d,%d,%d,%d,%d,%d", __func__, __LINE__,
               cfg_info->auth_mode,
               cfg_info->tau_timer_mode,
               cfg_info->uq_mode,
               cfg_info->ce_mode,
               cfg_info->heartbeat_mode,
               cfg_info->wakeup_mode,
               cfg_info->protocol_mode);

__exit:

    return result;
}

os_err_t mb26_ctm2m_register(ctm2m_t *handle)
{
    os_err_t     result = OS_EOK;
    at_parser_t *parser = &handle->module->parser;
    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};
    
    result = at_parser_exec_cmd(parser, &resp, "AT+CTM2MREG");
    if (OS_EOK != result)
    {
        os_err_t ret = 0;
        if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2M ERROR:", "+CTM2M ERROR: %d", &ret))
        {
            ERROR("%s-%d:get error code failed", __func__, __LINE__);
        }
        else 
        {
            ERROR("%s-%d:error code[%d]", __func__, __LINE__, ret);
            return ret;
        }
    }
    return result;
}

os_err_t mb26_ctm2m_deregister(ctm2m_t *handle)
{
    os_err_t     result   = OS_EOK;
    at_parser_t *parser   = &handle->module->parser;
    os_int32_t   reg_stat = MB26_CTM2M_INVALID_DFT;
    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};
    
    result = at_parser_exec_cmd(parser, &resp, "AT+CTM2MREG?");
    if (OS_EOK != result)
    {
        ERROR("%s-%d: get CTM2M regist status failed", __func__, __LINE__);
        goto __exit;
    }

    if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2MREG:", "+CTM2MREG: %d", &reg_stat))
    {
        ERROR("%s-%d:get error code failed", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    switch (reg_stat)
    {
    case 0:
        INFO("%s-%d:CTM2M has already been deregistered, nothing todo.", __func__, __LINE__);
        goto __exit;
    case 1:
        DEBUG("%s-%d:CTM2M registered, destroy it.", __func__, __LINE__);
        break;
    default:
        ERROR("%s-%d:CTM2M registeration status unknown[%d].", __func__, __LINE__, reg_stat);
        result = OS_ERROR;
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CTM2MDEREG");
    if (OS_EOK != result)
    {
        int ret = 0;
        if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2M ERROR:", "+CTM2M ERROR: %d", &ret))
        {
            ERROR("%s-%d:get error code failed", __func__, __LINE__);
        }
        else 
        {
            ERROR("%s-%d:error code[%d]", __func__, __LINE__, ret);
        }
    }

__exit:

    return result;
}

os_err_t mb26_ctm2m_send(ctm2m_t *handle, ctm2m_send_t send, os_int32_t *msg_id)
{
    os_err_t     result  = OS_EOK;
    at_parser_t *parser  = &handle->module->parser;
    
    const char  *send_cmd  = "AT+CTM2MSEND=";
    os_size_t    sent_size = 0;
    // os_uint16_t  sem_rst   = 0;
    handle->msg_id = MB26_CTM2M_INVALID_DFT;
    *msg_id        = MB26_CTM2M_INVALID_DFT;
    
    char cmd_tail[AT_RESP_BUFF_SIZE_DEF] = {0};
    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};
    
    if (OS_NULL == send.msg.data || MB26_CTM2M_SEND_MAX_LEN < send.msg.len || 0 == send.msg.len)
    {
        ERROR("%s-%d:parm invalid.", __func__, __LINE__);
        return OS_ERROR;
    }
    
    char *hex_str = os_calloc(1, 2 * send.msg.len);
    if (OS_NULL == hex_str)
    {
        ERROR("%s-%d:no enough memory for hex string.", __func__, __LINE__);
        return OS_ERROR;
    }

    bytes_to_hexstr(send.msg.data, hex_str, send.msg.len);

    // result = os_sem_control(handle->ctm2m_sem, OS_IPC_CMD_RESET, &sem_rst);
    // if (OS_EOK != result)
    // {
    //     ERROR("%s-%d:sem reset failed", __func__, __LINE__);
    //     goto __exit;
    // }

    while (OS_EOK == os_sem_wait(handle->ctm2m_sem, OS_NO_WAIT));

    sent_size  = at_parser_send(parser, send_cmd, strlen(send_cmd));
    sent_size += at_parser_send(parser, hex_str, 2 * send.msg.len);
    
    if (2 * send.msg.len + strlen(send_cmd) != sent_size)
    {
        ERROR("%s-%d:send msg failed", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    sprintf(cmd_tail, ",%d", send.mode);

    result = at_parser_exec_cmd(parser, &resp, cmd_tail);
    if (OS_EOK != result)
    {
        int ret = 0;
        if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2M ERROR:", "+CTM2M ERROR: %d", &ret))
        {
            ERROR("%s-%d:get error code failed", __func__, __LINE__);
        }
        else 
        {
            ERROR("%s-%d:error code[%d]", __func__, __LINE__, ret);
        }
        goto __exit;
    }
    
    result = os_sem_wait(handle->ctm2m_sem, 6 * OS_TICK_PER_SECOND);
    if (OS_EOK != result)
    {
        ERROR("%s-%d:ctm2m send failed", __func__, __LINE__);
        goto __exit;
    }

    if (MB26_CTM2M_INVALID_DFT == handle->msg_id)
    {
        ERROR("%s-%d:get send msg ID failed", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {   /* FIXME CHANGE NAME */
        *msg_id = handle->msg_id;
        handle->msg_id = MB26_CTM2M_INVALID_DFT;
    }

__exit:
    
    if (OS_NULL != hex_str)
    {
        os_free(hex_str);
    }

    return result;
}

os_err_t mb26_ctm2m_resp(ctm2m_t *handle, ctm2m_resp_t resp)
{
    os_err_t     result   = OS_EOK;
    at_parser_t *parser   = &handle->module->parser;
    os_size_t    cmd_len  = 0;
    char        *cmd      = OS_NULL;
    char        *data_hex = OS_NULL;

    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    
    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};
    
    if (MB26_CTM2M_MSG_ID_MAX < resp.msg_id || OS_NULL == resp.token.data || OS_NULL == resp.uri.data)
    {
        ERROR("%s-%d:input parm invalid", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (OS_NULL != resp.data.data)
    {
        cmd_len = MB26_CTM2M_STR_BUFF_DEF + resp.token.len + resp.uri.len + resp.data.len;
        
        data_hex = os_calloc(1, 1 + 2 * resp.data.len);
        if (OS_NULL == data_hex)
        {
            ERROR("%s-%d:no enough memory", __func__, __LINE__);
            result = OS_ERROR;
            goto __exit;
        }
        
        bytes_to_hexstr(resp.data.data, data_hex, resp.data.len);
    }
    else
    {
        cmd_len = MB26_CTM2M_STR_BUFF_DEF + resp.token.len + resp.uri.len;
    }

    /* adjustable lenth */
    cmd = os_calloc(1, cmd_len);
    if (OS_NULL == cmd)
    {
        ERROR("%s-%d:no enough memory", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (OS_NULL == data_hex)
    {
        snprintf(cmd, cmd_len, "AT+CTM2MCMDRSP=%d,%s,%d,%s,%d", 
                                resp.msg_id, 
                                resp.token.data, 
                                resp.resp_code, 
                                resp.uri.data,
                                resp.observe);
    }
    else
    {
        snprintf(cmd, cmd_len, "AT+CTM2MCMDRSP=%d,%s,%d,%s,%d,%d,%s", 
                                resp.msg_id, 
                                resp.token.data, 
                                resp.resp_code,
                                resp.uri.data,
                                resp.observe,
                                resp.dataformate,
                                resp.data.data);
    }

    result = at_parser_exec_cmd(parser, &at_resp, cmd);
    if (OS_EOK != result)
    {
        int ret = 0;
        if (0 >= at_resp_get_data_by_kw(&at_resp, "+CTM2M ERROR:", "+CTM2M ERROR: %d", &ret))
        {
            ERROR("%s-%d:get error code failed", __func__, __LINE__);
        }
        else 
        {
            ERROR("%s-%d:error code[%d]", __func__, __LINE__, ret);
        }
    }

__exit:

    if (OS_NULL != cmd)         os_free(cmd);
    if (OS_NULL != data_hex)    os_free(data_hex);

    return result;
}

os_err_t mb26_ctm2m_update(ctm2m_t *handle, ctm2m_update_t update)
{
    os_err_t     result = OS_EOK;
    at_parser_t *parser = &handle->module->parser;

    char cmd[MB26_CTM2M_STR_BUFF_DEF]         = {0};
    char resp_buff[MB26_CTM2M_RESP_BUFF_SIZE] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};
    
    if (CTM2M_BINDING_NULL != update.mode)
    {
        snprintf(cmd, MB26_CTM2M_STR_BUFF_DEF - 1, "AT+CTM2MUPDATE=%d", update.mode);
    }
    else
    {
        snprintf(cmd, MB26_CTM2M_STR_BUFF_DEF - 1, "AT+CTM2MUPDATE");
    }

    result = at_parser_exec_cmd(parser, &resp, cmd);
    if (OS_EOK != result)
    {
        int ret = 0;
        if (0 >= at_resp_get_data_by_kw(&resp, "+CTM2M ERROR:", "+CTM2M ERROR: %d", &ret))
        {
            ERROR("%s-%d:get error code failed", __func__, __LINE__);
        }
        else
        {
            ERROR("%s-%d:error code[%d]", __func__, __LINE__, ret);
        }
        return OS_ERROR;
    }
    
    return result;
}

static void mb26_ctm2m_send_handler(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    
    os_int32_t msg_id = MB26_CTM2M_INVALID_DFT;
    
    if (OS_NULL == handler)
    {
        ERROR("[%d][%s] No handle created.", __LINE__, __func__);
        return;
    }

    sscanf(data, "+CTM2MSEND: %d", &msg_id);
    
    handler->msg_id = msg_id;
    os_sem_post(handler->ctm2m_sem);

    return;
}

static os_err_t mb26_ctm2m_recv_handler(ctm2m_t *handle, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != handle);
    OS_ASSERT(OS_NULL != data);

    ctm2m_string_t recv_data  = {0};
    os_err_t       result     = OS_EOK;
    char          *bytes_data = OS_NULL;

    if (OS_NULL == handle->receive_cb)
    {
        WARN("[%d][%s] No user receive callback registed.", __LINE__, __func__);
        return OS_ERROR;
    }

    /* -\r\n */
    recv_data.len  = (size - strlen("+CTM2MRECV: ") - 2) / 2;
    bytes_data     = os_calloc(1, recv_data.len * 2 + 1);
    
    if (OS_NULL == bytes_data)
    {
        ERROR("%s-%d:no enough memory.", __func__, __LINE__);
        result = OS_ENOMEM;
        goto __exit;
    }
    
    hexstr_to_bytes((char *)data + strlen("+CTM2MRECV: "), bytes_data, 2 * recv_data.len);
    recv_data.data = bytes_data;
    
    handle->receive_cb(recv_data);
    
__exit:

    if (OS_NULL != bytes_data)
    {
        os_free(bytes_data);
    }

    return result;
}

static os_err_t mb26_ctm2m_notify_handler(ctm2m_t *handle, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != handle);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == handle->notify_cb)
    {
        WARN("[%d][%s] No user notify callback registed.", __LINE__, __func__);
        return OS_ERROR;
    }

    os_int32_t     status_code = MB26_CTM2M_INVALID_DFT;
    os_int32_t     msg_id      = MB26_CTM2M_INVALID_DFT;
    ctm2m_notify_t notify;

    memset(&notify, 0, sizeof(ctm2m_notify_t));

    char notify_type_str[MB26_CTM2M_STR_LEN_DEF] = {0};

    /* format: +CTM2M: <operation>,<status code>[,<data1>,<data2>,<data3>] */
    if (2 > sscanf(data, "+CTM2M: %[^,],%d,%d", notify_type_str, &status_code, &msg_id))
    {
        ERROR("[%d][%s] Parse notify message error.", __LINE__, __func__);
        return OS_ERROR;
    }

    switch (*(char *)notify_type_str)
    {
    case 'r':
        notify.type = CTM2M_NTF_TYPE_REG;
        break;
    case 'o':
        notify.type = CTM2M_NTF_TYPE_OBS;
        break;
    case 'u':
        notify.type = CTM2M_NTF_TYPE_UPDATE;
        break;
    case 'p':
        notify.type = CTM2M_NTF_TYPE_PING;
        break;
    case 'd':
        notify.type = CTM2M_NTF_TYPE_DEREG;
        break;
    case 's':
        notify.type = CTM2M_NTF_TYPE_SEND;
        break;
    case 'l':
        notify.type = CTM2M_NTF_TYPE_LWSTATUS;
        break;
    default:
        ERROR("[%d][%s] error: unkonwn type.", __LINE__, __func__);
        return OS_ERROR;
    }

    switch (status_code)
    {
    case CTM2M_NTF_E_SUCCESS:
    case CTM2M_NTF_E_TIMEOUT:
    case CTM2M_NTF_E_NO_SENDOUT_PACK:
    case CTM2M_NTF_E_RECV_RST:    
    case CTM2M_NTF_E_PARM_INVALID:
    case CTM2M_NTF_E_UNKNOWN_ERR:       
    case CTM2M_NTF_E_AUTH_FAILED:        
    case CTM2M_NTF_E_UE_NOT_LOGIN:
    case CTM2M_NTF_E_VER_MISMATCH:
    case CTM2M_NTF_E_SESSION_INVALID:
    case CTM2M_NTF_E_SESSION_LOAD_FAILED:
    case CTM2M_NTF_E_ENGINE_ABNORMAL:
    case CTM2M_NTF_E_TAU_DUE:
    case CTM2M_NTF_E_SENT_ALREADY:
    case CTM2M_NTF_E_NO_OBJECT19:
        notify.status = (ctm2m_notify_err_t)status_code;
        break;
    default:
        ERROR("[%d][%s] error: unkonwn status code:[%d].", __LINE__, __func__, status_code);
        return OS_ERROR;
    }

    if (MB26_CTM2M_INVALID_DFT != msg_id) notify.msg_id = msg_id;

    handle->notify_cb(notify);
    
    return OS_EOK;
}

static os_err_t mb26_ctm2m_request_handler(ctm2m_t *handle, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != handle);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == handle->request_cb)
    {
        WARN("[%d][%s] No user request callback registed.", __LINE__, __func__);
        return OS_ERROR;
    }

    ctm2m_request_t request;
    os_err_t        result      = OS_EOK;
    os_int32_t      msg_id      = MB26_CTM2M_INVALID_DFT;
    os_int32_t      cmd_type    = MB26_CTM2M_INVALID_DFT;
    os_int32_t      obs_type    = MB26_CTM2M_INVALID_DFT;
    os_int32_t      data_format = MB26_CTM2M_INVALID_DFT;

    memset(&request, 0, sizeof(ctm2m_request_t));
    request.observe = CTM2M_OBS_NULL;

    /* FIXME TOKEN RANGE */
    char token_hex_str[MB26_CTM2M_STR_BUFF_DEF] = {0};
    char uri_str[MB26_CTM2M_STR_BUFF_DEF]       = {0};
    char cmd_data[MB26_CTM2M_STR_BUFF_DEF]      = {0};

    /* format: +CTM2MCMD:<msgid>,<cmdtype>,<token>,<uri_str>[,<observe>(,<dataformat>,<data>)] */
    char *data_p   = (char *)data;
    int   count    = 4; /* the first 4 parms */
    int   solved_n = 0;
    
    /* if with optional part, pointer to comma, otherwise point to \0 */
    while (count && *++data_p) if (',' == *data_p) count--;

    if (1 < count)
    {
        ERROR("[%d][%s] Parse request message error.", __LINE__, __func__);
        return OS_ERROR;
    }
    /* else count == 1, this case means pointer point to \0, no other info need to parse */
    else if (1 == count) goto __fin;
    
    /* solve 3 optional param: obs_type & data_format & cmd data */
    while (data_p)
    {
        switch(solved_n)
        {
        case 0:
            sscanf(data_p, ",%d", &obs_type);
            break;
        case 1:
            sscanf(data_p, "%d", &data_format);
            break;
        case 2:
            sscanf(data_p, "%[^\r^\0]", cmd_data);
            break;
        default:
            ERROR("[%d][%s] Parse request message error.", __LINE__, __func__);
            return OS_ERROR;
        }
        solved_n++;
        data_p = strstr(++data_p, ",");
    }

    if (CTM2M_OBS_SET == obs_type || CTM2M_OBS_CANCEL == obs_type)
        request.observe = (ctm2m_observe_cfg_t)obs_type;
    if (CTM2M_DATAFORM_TLV == data_format || (CTM2M_DATAFORM_COAP >= data_format && CTM2M_DATAFORM_JSON <= data_format))
        request.dataformate = (ctm2m_dataform_t)data_format;
    if (*cmd_data)
    {
        request.data.data = cmd_data;
        request.data.len  = strlen(cmd_data);
    }

__fin:

    if (4 > sscanf(data, "+CTM2MCMD: %d,%d,%[^,],%[^,^\r]", &msg_id, &cmd_type, token_hex_str, uri_str))
    {
        ERROR("[%d][%s] Parse request message error.", __LINE__, __func__);
        return OS_ERROR;
    }

    if (MB26_CTM2M_MSG_ID_MAX < msg_id   || 0 > msg_id)                      result = OS_ERROR;
    if (CTM2M_REQ_TYPE_DELETE < cmd_type || CTM2M_REQ_TYPE_READ > cmd_type)  result = OS_ERROR;
    if (MB26_CTM2M_STR_BUFF_DEF <= strlen(token_hex_str) || !*token_hex_str) result = OS_ERROR;
    if (MB26_CTM2M_STR_BUFF_DEF <= strlen(uri_str)       || !*uri_str)       result = OS_ERROR;

    if (OS_EOK != result)
    {
        ERROR("[%d][%s] received invalid parm.", __LINE__, __func__);
        return result;
    }

    request.msg_id     = msg_id;
    request.type       = (ctm2m_request_type_t)cmd_type;
    request.token.data = token_hex_str;
    request.token.len  = strlen(token_hex_str);
    request.uri.data   = uri_str;
    request.uri.len    = strlen(uri_str);

    handle->request_cb(request);
    
    return OS_EOK;
}

static mb26_ctm2m_urc_t mb26_ctm2m_urc_handler_table[] = {
    {.prefix = "CTM2MRECV",      .func = mb26_ctm2m_recv_handler   },
    {.prefix = "CTM2M",          .func = mb26_ctm2m_notify_handler },
    {.prefix = "CTM2MCMD",       .func = mb26_ctm2m_request_handler},
};

static void mb26_ctm2m_urc_receiver(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    
    char      *urc_data    = OS_NULL;
    os_int8_t  handler_ref = MB26_CTM2M_INVALID_DFT;
    char       urc_cmd_head[MB26_CTM2M_STR_LEN_DEF] = {0};
   
    if (OS_NULL == handler)
    {
        ERROR("[%d][%s] No handle created.", __LINE__, __func__);
        return;
    }

    /* parse from cmd */
    sscanf(data, "+%[^:]", urc_cmd_head);
    
    /* find registration */
    for (int i = 0; i < sizeof(mb26_ctm2m_urc_handler_table) / sizeof(mb26_ctm2m_urc_t); i++)
    {
        if (0 != strcmp(urc_cmd_head, mb26_ctm2m_urc_handler_table[i].prefix))
        {
            continue;
        }
        else
        {
            handler_ref = i;
            INFO("[%d][%s] Urc code received[%s].", __LINE__, __func__, urc_cmd_head);
            break;
        }
    }

    if (MB26_CTM2M_INVALID_DFT == handler_ref)
    {
        ERROR("[%d][%s] Urc code unrecognized[%s].", __LINE__, __func__, urc_cmd_head);
        return;
    }

    /* alloc mem for handler use */
    urc_data = os_calloc(1, size);
    if (OS_NULL == urc_data)
    {
        ERROR("%s-%d:no enough memory.", __func__, __LINE__);
        return;
    }

    memcpy(urc_data, data, size);

    /* send handle_msg to mq */
    mb26_ctm2m_msg_t handle_msg = {urc_data, 
                                   size, 
                                   mb26_ctm2m_urc_handler_table[handler_ref].func};
    
    if (OS_EOK != os_mq_send(handler->ctm2m_mq, &handle_msg, sizeof(handle_msg), OS_NO_WAIT))
    {
        ERROR("[%s] mq send failed, too many unsolved urc.", __func__);
        os_free(urc_data);
    }

    return;
}

static void mb26_ctm2m_manager_task(void *parameter)
{
    OS_ASSERT(OS_NULL != parameter);
    
    INFO("[%s] task start up.", __func__);

    mb26_ctm2m_msg_t handle_msg;
    ctm2m_t         *handle     = (ctm2m_t *)parameter;
    os_err_t         result     = OS_ERROR;
    os_size_t        recv_size  = 0;

    while (1)
    {
        memset(&handle_msg, 0, sizeof(mb26_ctm2m_msg_t));
        
        /* wait & pop */
        result = os_mq_recv(handle->ctm2m_mq,
                           &handle_msg, 
                            sizeof(handle_msg), 
                            OS_WAIT_FOREVER, 
                           &recv_size);
        if (OS_EOK != result)
        {
            INFO("[%s] mq received mq_control [exit] msg.[%d].", __func__, result);
            // os_free((void *)handle_msg.data);
            break;
        }

        /* execute urc handler */
        OS_ASSERT(OS_NULL != handle_msg.func);
        handle_msg.func(handle, handle_msg.data, handle_msg.len);

        /* free buffer alloced in mb26_ctm2m_urc_receiver */
        os_free((void *)handle_msg.data);
    }

    INFO("[%s] task exit.", __func__);
    return;
}

static at_urc_t mb26_ctm2m_urc_table[] = {
    {.prefix = "+CTM2MSEND:",      .suffix = "\r\n", .func = mb26_ctm2m_send_handler},
    {.prefix = "+CTM2MRECV:",      .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2M: reg",      .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2M: obsrv",    .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2M: update",   .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2M: ping",     .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2M: dereg",    .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2M: send",     .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2M: lwstatus", .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
    {.prefix = "+CTM2MCMD:",       .suffix = "\r\n", .func = mb26_ctm2m_urc_receiver},
};

static os_err_t mb26_ctm2m_task_create(ctm2m_t *handle)
{
    OS_ASSERT(OS_NULL != handle);

    mo_object_t *module = handle->module;
    at_parser_t *parser = &module->parser;

    /* create&start urc manager task */
    handle->ctm2m_task = os_task_create(MB26_CTM2M_TASK_NAME, 
                                        mb26_ctm2m_manager_task,
                                        handle,
                                        MB26_CTM2M_TASK_STACK_SIZE,
                                        MB26_CTM2M_TASK_PRIORITY);
    if (OS_NULL == handle->ctm2m_task)
    {
        ERROR("%s-%d:ctm2m task create failed.", __func__, __LINE__);
        return OS_ERROR;
    }

    /* ctm2m urc manager task startup */
    if (OS_EOK != os_task_startup(handle->ctm2m_task))
    {
        ERROR("%s-%d:ctm2m task start failed.", __func__, __LINE__);
        os_task_destroy(handle->ctm2m_task);
        return OS_ERROR;
    }

    /* Set netconn urc table */
    at_parser_set_urc_table(parser, mb26_ctm2m_urc_table, sizeof(mb26_ctm2m_urc_table) / sizeof(at_urc_t));

    INFO("%s-%d:ctm2m urc handler task create success.", __func__, __LINE__);

    return OS_EOK;
}

static void mb26_ctm2m_task_destroy(ctm2m_t *handle)
{
    OS_ASSERT(OS_NULL != handle);
    OS_ASSERT(OS_NULL != handle->ctm2m_task);

    mb26_ctm2m_msg_t     handle_msg;
    os_err_t             result     = OS_ERROR;
    os_size_t            recv_size  = 0;

    /* free buff in mq which alloced in mb26_ctm2m_urc_receiver */
    while (OS_TRUE)
    {
        memset(&handle_msg, 0, sizeof(mb26_ctm2m_msg_t));
        result = os_mq_recv(handle->ctm2m_mq,
                            &handle_msg, 
                            sizeof(handle_msg), 
                            OS_NO_WAIT, 
                            &recv_size);
        if (OS_EOK != result) break;
        os_free((void *)handle_msg.data);
    }
    
    /* auto deinit urc manager task */
    os_task_destroy(handle->ctm2m_task);
    // os_mq_control(handle->ctm2m_mq, OS_IPC_CMD_RESET, OS_NULL);

    return;
}


#endif /* MB26_USING_CTM2M_OPS */

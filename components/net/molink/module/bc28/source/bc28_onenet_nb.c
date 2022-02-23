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
 * @file        bc28_onenet_nb.c
 *
 * @brief       bc28 module link kit onenet nb api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc28.h"
#include "at_parser.h"
#include "bc28_onenet_nb.h"
#include "mo_onenet_nb.h"
#include "os_task.h"
#include "os_mq.h"
#include "os_mutex.h"
#include "os_memory.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "bc28.onenet_nb"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define BC28_URC_HEAD_MAXLEN            (16)
#define BC28_REGEX_LEN_DEFAULT          (64)
#define BC28_TIMEOUT_DEFAULT            (5 * OS_TICK_PER_SECOND)
#define BC28_URC_MQ_NAME                "BC28_URC_MQ"
#define BC28_URC_MQ_MSGNUM_MAX          (10)
#define BC28_URC_MANAGER_TASK_NAME      "bc28_manager"

/* ============================kconfig_options============================= */
#ifndef BC28_URC_MANAGER_STACK_SIZE     
#define BC28_URC_MANAGER_STACK_SIZE     (2048)
#endif /* BC28_URC_MANAGER_STACK_SIZE */

#ifndef BC28_RESP_BUFF_SIZE
#define BC28_RESP_BUFF_SIZE             (128)
#endif /* BC28_RESP_BUFF_SIZE */

#ifndef BC28_URC_MANAGER_TASK_PRIORITY
#define BC28_URC_MANAGER_TASK_PRIORITY  (0x10)
#endif /* BC28_URC_MANAGER_TASK_PRIORITY */
/* ===========================kconfig_option_tail========================== */

static os_mq_t         *bc28_nb_mq = OS_NULL;
static os_task_t        bc28_nb_manager_task;
static os_uint8_t       bc28_nb_manager_task_stack[BC28_URC_MANAGER_STACK_SIZE];

static const os_int8_t  BC28_ONENETNB_INVALID_DEFAULT = -1;

typedef struct bc28_onenet_urc
{
    const char      *prefix;                        /* URC data prefix      */
    void (*func)(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size); 
                                                    /* URC hanlder function */
} bc28_onenet_urc_t;

typedef struct bc28_onenet_mq_msg
{
    mo_onenet_cb_t *regist_cb;                      /* user regist cb ptr */
    const char     *data;                           /* URC data */
    os_size_t       len;                            /* URC data len */
    void (*func)(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size);
                                                    /* URC hanlder function */
} bc28_onenet_mq_msg_t;

typedef enum bc28_config_mode
{
    BC28_ONENETNB_GUIDEMODE_DISABLE_ADDR = 0,
    BC28_ONENETNB_GUIDEMODE_ENABLE_ADDR,
    BC28_ONENETNB_RSP_TIMEOUT,
    BC28_ONENETNB_OBS_AUTOACK,
    BC28_ONENETNB_AUTH_CONFIG,
    BC28_ONENETNB_DTLS_CONFIG,
    BC28_ONENETNB_WRITE_FORMATE,
    BC28_ONENETNB_BUF_CONFIG,
} bc28_config_mode_t;

#define CALL_BC28_BASIC_FUNC(AT_FRONT_OPT)                                  \
do                                                                          \
{                                                                           \
    OS_ASSERT(module != OS_NULL);                                           \
    OS_ASSERT(format != OS_NULL);                                           \
                                                                            \
    at_parser_t *parser = &module->parser;                                  \
    os_err_t     result = OS_EOK;                                           \
                                                                            \
    char resp_buff[BC28_RESP_BUFF_SIZE] = {0};                              \
    at_resp_t at_resp = {.buff      = resp_buff,                            \
                         .buff_size = sizeof(resp_buff),                    \
                         .line_num  = 0,                                    \
                         .timeout   = timeout};                             \
                                                                            \
    char tmp_format[BC28_RESP_BUFF_SIZE] = AT_FRONT_OPT;                    \
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));       \
                                                                            \
    result = at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args); \
                                                                            \
    return result;                                                          \
} while (0);

os_err_t bc28_onenetnb_get_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* maximum return time out 300ms, suggest more than that. */
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(resp != OS_NULL);

    at_parser_t *parser         = &module->parser;
    os_err_t     result         = OS_EOK;
    os_uint32_t  line;
    char         tmp_buff[128]  = {0};
    os_int32_t   getline_num    = 0;
    os_int32_t   config_mode_ref;

    mo_config_resp_t *config_resp  = (mo_config_resp_t *)resp;
    memset(config_resp, 0, sizeof(mo_config_resp_t));

    char resp_buff[128] = {0};
    at_resp_t at_resp   = {.buff      = resp_buff,
                           .buff_size = sizeof(resp_buff),
                           .line_num  = 0,
                           .timeout   = timeout};

    result = at_parser_exec_cmd(parser, &at_resp, "AT+MIPLCONFIG?");
    if (OS_EOK != result)
    {
        ERROR("Get %s module onenetnb config failed", module->name);
        goto __exit;
    }

    /* set default invalid value */
    config_resp->guide_mode_enable  = BC28_ONENETNB_INVALID_DEFAULT;
    config_resp->obs_autoack_enable = BC28_ONENETNB_INVALID_DEFAULT;
    config_resp->auth_enable        = BC28_ONENETNB_INVALID_DEFAULT;
    config_resp->dtls_enable        = BC28_ONENETNB_INVALID_DEFAULT;
    config_resp->write_format       = BC28_ONENETNB_INVALID_DEFAULT;
    config_resp->buf_cfg            = BC28_ONENETNB_INVALID_DEFAULT;
    config_resp->buf_urc_mode       = BC28_ONENETNB_INVALID_DEFAULT;
    /* return mode with 7 lines */
    for (line = 1; ; line++)
    {   
        getline_num = at_resp_get_data_by_line(&at_resp, line, "+MIPLCONFIG:%d,%s", &config_mode_ref, tmp_buff);
        if (0 == getline_num) /* empty line */
        {
            continue;
        }
        else if (0 > getline_num) /* end of resp */
        {
            goto __exit;
        }
        DEBUG("mode:[%d],line[%u][%s]", config_mode_ref, line, tmp_buff);
        switch (config_mode_ref)
        {
        case BC28_ONENETNB_GUIDEMODE_DISABLE_ADDR:
            config_resp->guide_mode_enable = OS_FALSE;
            sscanf(tmp_buff, "%[^,],%hu", config_resp->ip, &config_resp->port);
            break;
        case BC28_ONENETNB_GUIDEMODE_ENABLE_ADDR:
            config_resp->guide_mode_enable = OS_TRUE;
            sscanf(tmp_buff, "%[^,],%hu", config_resp->ip, &config_resp->port);
            break;
        case BC28_ONENETNB_RSP_TIMEOUT:
            sscanf(tmp_buff, "%hhu", &config_resp->rsp_timeout);
            break;
        case BC28_ONENETNB_OBS_AUTOACK:
            sscanf(tmp_buff, "%hhd", &config_resp->obs_autoack_enable);
            break;
        case BC28_ONENETNB_AUTH_CONFIG:
            sscanf(tmp_buff, "%hhd,%s", &config_resp->auth_enable, config_resp->auth_code);
            break;
        case BC28_ONENETNB_DTLS_CONFIG:
            sscanf(tmp_buff, "%hhd,%s", &config_resp->dtls_enable, config_resp->psk);
            break;
        case BC28_ONENETNB_WRITE_FORMATE:
            sscanf(tmp_buff, "%hhd", &config_resp->write_format);
            break;
        case BC28_ONENETNB_BUF_CONFIG:
            sscanf(tmp_buff, "%hhd,%hhd", &config_resp->buf_cfg, &config_resp->buf_urc_mode);
            break;
        default:
            ERROR("Get %s module onenetnb mode:%d invalid", module->name, config_mode_ref);
            break;
        }
        
        memset(tmp_buff, 0, sizeof(tmp_buff));
    }

__exit:

    return result;
}

os_err_t bc28_onenetnb_set_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLCONFIG=");    /* maximum return time out 300ms, suggest more than it */
}

os_err_t bc28_onenetnb_create(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(resp != OS_NULL);

    at_parser_t *parser = &module->parser;

    char resp_buff[128] = {0};
    at_resp_t at_resp   = {.buff      = resp_buff,
                           .buff_size = sizeof(resp_buff),
                           .line_num  = 0,
                           .timeout   = timeout};

    char tmp_format[128] = "AT+MIPLCREATE";

    if (at_parser_exec_cmd(parser, &at_resp, tmp_format) != OS_EOK)
    {
        return OS_ERROR;
    }

    os_uint8_t ref = 0;
    
    if (at_resp_get_data_by_kw(&at_resp, "+MIPLCREATE:", "+MIPLCREATE:%hhu", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t bc28_onenetnb_delete(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLDELETE=");
}

os_err_t bc28_onenetnb_addobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLADDOBJ=");
}

os_err_t bc28_onenetnb_delobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLDELOBJ=");
}

os_err_t bc28_onenetnb_open(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* Lifetime : 16-268435454 (s) */
    /* Timeout  : [30]-65535   (s) */
    CALL_BC28_BASIC_FUNC("AT+MIPLOPEN=");
}

os_err_t bc28_onenetnb_close(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* NO RESP. return 3s */
    CALL_BC28_BASIC_FUNC("AT+MIPLCLOSE=");
}

os_err_t bc28_onenetnb_discoverrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLDISCOVERRSP=");
}

os_err_t bc28_onenetnb_observersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLOBSERVERSP=");
}

os_err_t bc28_onenetnb_readrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLREADRSP=");
}

os_err_t bc28_onenetnb_writersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLWRITERSP=");
}

os_err_t bc28_onenetnb_executersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLEXECUTERSP=");
}

os_err_t bc28_onenetnb_parameterrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLPARAMETERRSP=");
}

os_err_t bc28_onenetnb_notify(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    /* USER DATA <= 1000 BYTES */
    CALL_BC28_BASIC_FUNC("AT+MIPLNOTIFY=");
}

os_err_t bc28_onenetnb_update(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    CALL_BC28_BASIC_FUNC("AT+MIPLUPDATE=");
}

static void urc_discover_handler(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID> */
    OS_ASSERT(OS_NULL != regist_cb);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == regist_cb->discover_notify_cb)
    {
        WARN("[%d][%s] No discover callback registed.", __LINE__, __func__);
        return;
    }
    
    mo_onenet_discover_t    discover_info;
    memset(&discover_info, 0, sizeof(mo_onenet_discover_t));

    sscanf(data, "+MIPLDISCOVER: %u,%d,%d", 
          &discover_info.ref, 
          &discover_info.msg_id, 
          &discover_info.obj_id);
    
    regist_cb->discover_notify_cb(&discover_info);

    return;
}

static void urc_observe_handler(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<flag>,<objID>,<insID>,<resID> */
    OS_ASSERT(OS_NULL != regist_cb);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == regist_cb->discover_notify_cb)
    {
        WARN("[%d][%s] No observe callback registed.", __LINE__, __func__);
        return;
    }

    mo_onenet_observe_t     observe_info;
    memset(&observe_info, 0, sizeof(mo_onenet_observe_t));

    sscanf(data, "+MIPLOBSERVE: %u,%d,%hhd,%d,%d,%d", 
          &observe_info.ref, 
          &observe_info.msg_id, 
          &observe_info.flag, 
          &observe_info.obj_id, 
          &observe_info.ins_id, 
          &observe_info.res_id);
    
    regist_cb->observe_notify_cb(&observe_info);

    return;
}

static void urc_read_handler(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID> */
    OS_ASSERT(OS_NULL != regist_cb);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == regist_cb->read_notify_cb)
    {
        WARN("[%d][%s] No read callback registed.", __LINE__, __func__);
        return;
    }

    mo_onenet_read_t        read_info;
    memset(&read_info, 0, sizeof(mo_onenet_read_t));

    sscanf(data, "+MIPLREAD: %u,%d,%d,%d,%d", 
          &read_info.ref, 
          &read_info.msg_id,  
          &read_info.obj_id, 
          &read_info.ins_id, 
          &read_info.res_id);
    
    regist_cb->read_notify_cb(&read_info);

    return;
}

static void urc_write_handler(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID>,<value_type>,<len>,<value>,<flag>,<index> */
    /* quectel suggest that value < 1000, otherwise it may cause faliure */
    OS_ASSERT(OS_NULL != regist_cb);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == regist_cb->write_notify_cb)
    {
        WARN("[%d][%s] No write callback registed.", __LINE__, __func__);
        return;
    }

    mo_onenet_write_t       write_info;
    os_int8_t              *value       = OS_NULL;
    os_int8_t               value_trait = 0; /* value trait:string/hexstring */
    
    memset(&write_info, 0, sizeof(mo_onenet_write_t));

    char regex[BC28_REGEX_LEN_DEFAULT] = {0};
    sscanf(data, "+MIPLWRITE: %*d,%*d,%*d,%*d,%*d,%*d,%d,%c", &write_info.len, &value_trait);
    if ('\"' != value_trait)
    {
        value = os_calloc(1, write_info.len + 1);
        strcpy(regex, "+MIPLWRITE: %u,%d,%d,%d,%d,%hhd,%*d,\"%[^\"]\",%hhd,%d");
    }
    else
    {
        value = os_calloc(1, write_info.len * 2 + 1);
        strcpy(regex, "+MIPLWRITE: %u,%d,%d,%d,%d,%hhd,%*d,%[^,],%hhd,%d");
    }
    
    if (value == OS_NULL)
    {
        ERROR("Calloc onenetNB[%s] value str failed, no enough memory", __func__);
        return;
    }

    sscanf(data, regex, 
         &write_info.ref, 
         &write_info.msg_id,  
         &write_info.obj_id, 
         &write_info.ins_id, 
         &write_info.res_id,
         &write_info.value_type,
          value,
         &write_info.flag,
         &write_info.index);
    
    regist_cb->write_notify_cb(&write_info, (const char *)value);
    
    os_free(value);
    return;
}

static void urc_execute_handler(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID>[,<len>,<arguments>] */
    /* quectel suggest that arguments < 1000, otherwise it may cause faliure */
    OS_ASSERT(OS_NULL != regist_cb);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == regist_cb->execute_notify_cb)
    {
        WARN("[%d][%s] No execute callback registed.", __LINE__, __func__);
        return;
    }

    mo_onenet_execute_t     execute_info;
    memset(&execute_info, 0, sizeof(mo_onenet_execute_t));

    sscanf(data, "+MIPLEXECUTE: %*d,%*d,%*d,%*d,%*d,%d", &execute_info.len);
    
    os_uint8_t             *arguments = os_calloc(1, execute_info.len + 1);
    
    if (OS_NULL == arguments)
    {
        ERROR("Calloc onenetNB[%s] arguments str failed, no enough memory", __func__);
        return;
    }
    sscanf(data, "+MIPLEXECUTE: %u,%d,%d,%d,%d,%*d,\"%s\"",
          &execute_info.ref, 
          &execute_info.msg_id,  
          &execute_info.obj_id, 
          &execute_info.ins_id, 
          &execute_info.res_id,
           arguments);
    
    regist_cb->execute_notify_cb(&execute_info, (const char *)arguments);
    
    os_free(arguments);
    return;
}

static void urc_parameter_handler(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size)
{
    /* <ref>,<msgID>,<objID>,<insID>,<resID>,<len>,<parameter> */
    OS_ASSERT(OS_NULL != regist_cb);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == regist_cb->parameter_notify_cb)
    {
        WARN("[%d][%s] No parameter callback registed.", __LINE__, __func__);
        return;
    }

    mo_onenet_parameter_t    parameter_info;
    memset(&parameter_info, 0, sizeof(mo_onenet_parameter_t));

    sscanf(data, "+MIPLPARAMETER: %*d,%*d,%*d,%*d,%*d,%d", &parameter_info.len);
    
    os_uint8_t              *arguments = os_calloc(1, parameter_info.len + 1);
    
    if (OS_NULL == arguments)
    {
        ERROR("Calloc onenetNB[%s] arguments str failed, no enough memory", __func__);
        return;
    }
    sscanf(data, "+MIPLPARAMETER: %u,%d,%d,%d,%d,%*d,\"%s\"",
          &parameter_info.ref, 
          &parameter_info.msg_id,  
          &parameter_info.obj_id, 
          &parameter_info.ins_id, 
          &parameter_info.res_id,
           arguments);
    
    regist_cb->parameter_notify_cb(&parameter_info, (const char *)arguments);
    
    os_free(arguments);
    return;
}

static void urc_event_handler(mo_onenet_cb_t *regist_cb, const char *data, os_size_t size)
{
    /* <ref>,<evtID>[,<extend>][,<ackID>][,<time_stamp>,<cache_command_flag>] */
    OS_ASSERT(OS_NULL != regist_cb);
    OS_ASSERT(OS_NULL != data);

    if (OS_NULL == regist_cb->event_notify_cb)
    {
        WARN("[%d][%s] No event callback registed.", __LINE__, __func__);
        return;
    }

    mo_onenet_event_t    event_info;
    memset(&event_info, 0, sizeof(mo_onenet_event_t));
    event_info.extend             = BC28_ONENETNB_INVALID_DEFAULT;
    event_info.cache_command_flag = BC28_ONENETNB_INVALID_DEFAULT;

    sscanf(data, "+MIPLEVENT: %u,%hhu,%d,%hu,%s,%hhd",
          &event_info.ref, 
          &event_info.evt_id,  
          &event_info.extend, 
          &event_info.ack_id, 
           event_info.time_stamp,
          &event_info.cache_command_flag);
    
    regist_cb->event_notify_cb(&event_info);
    
    return;
}

static bc28_onenet_urc_t bc28_nb_urc_handler_table[] = {
    {.prefix = "MIPLDISCOVER",  .func = urc_discover_handler },
    {.prefix = "MIPLOBSERVE",   .func = urc_observe_handler  },
    {.prefix = "MIPLREAD",      .func = urc_read_handler     },
    {.prefix = "MIPLWRITE",     .func = urc_write_handler    },
    {.prefix = "MIPLEXECUTE",   .func = urc_execute_handler  },
    {.prefix = "MIPLPARAMETER", .func = urc_parameter_handler},
    {.prefix = "MIPLEVENT",     .func = urc_event_handler    },
};

static void bc28_urc_receiver(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    
    char      *urc_data                           = OS_NULL;
    os_int8_t  handler_ref                        = BC28_ONENETNB_INVALID_DEFAULT;
    char       urc_cmd_head[BC28_URC_HEAD_MAXLEN] = {0};
   
    /* parse from cmd */
    sscanf(data, "+%[^:]", urc_cmd_head);
    
    /* find registration */
    for (int i = 0; i < sizeof(bc28_nb_urc_handler_table) / sizeof(bc28_onenet_urc_t); i++)
    {
        if (0 != strcmp(urc_cmd_head, bc28_nb_urc_handler_table[i].prefix))
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

    if (BC28_ONENETNB_INVALID_DEFAULT == handler_ref)
    {
        WARN("[%d][%s] Urc code unrecognized[%s].", __LINE__, __func__, urc_cmd_head);
        return;
    }

    /* alloc mem for handler use */
    urc_data = os_calloc(1, size);
    memcpy(urc_data, data, size);

    /* get regist_cb ptr for urc_handler */
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc28_t *module = os_container_of(module, mo_bc28_t, parent);

    /* send handle_msg to mq */
    bc28_onenet_mq_msg_t handle_msg = {module->regist_cb, 
                                       urc_data, 
                                       size, 
                                       bc28_nb_urc_handler_table[handler_ref].func};
    
    if (OS_EOK != os_mq_send(bc28_nb_mq, &handle_msg, sizeof(handle_msg), OS_NO_WAIT))
    {
        WARN("[%s] mq send failed, too many unsolved urc.", __func__);
    }

    return;
}

static void bc28_urc_manager_task(void *parameter)
{
    INFO("[%s] task start up.", __func__);
    bc28_onenet_mq_msg_t handle_msg;
    os_err_t             result     = OS_ERROR;
    os_size_t            recv_size  = 0;

    while (1)
    {
        memset(&handle_msg, 0, sizeof(bc28_onenet_mq_msg_t));
        
        /* wait & pop */
        result = os_mq_recv(bc28_nb_mq,
                           &handle_msg, 
                            sizeof(handle_msg), 
                            OS_WAIT_FOREVER, 
                           &recv_size);
        if (OS_EOK != result)
        {
            INFO("[%s] mq received mq_control [exit] msg.[%d].", __func__, result);
            os_free((void *)handle_msg.data);
            /* auto deinit */
            return;
        }

        /* [*] add recursive lock, NB AT session */


        /* execute urc handler */
        OS_ASSERT(OS_NULL != handle_msg.func);
        handle_msg.func(handle_msg.regist_cb, handle_msg.data, handle_msg.len);


        /* [*] unlock */

        /* free buffer alloced in bc28_urc_receiver */
        os_free((void *)handle_msg.data);
    }
}

static at_urc_t bc28_nb_urc_table[] = {
    {.prefix = "+MIPLDISCOVER:",  .suffix = "\r\n", .func = bc28_urc_receiver},
    {.prefix = "+MIPLOBSERVE:",   .suffix = "\r\n", .func = bc28_urc_receiver},
    {.prefix = "+MIPLREAD:",      .suffix = "\r\n", .func = bc28_urc_receiver},
    {.prefix = "+MIPLWRITE:",     .suffix = "\r\n", .func = bc28_urc_receiver},
    {.prefix = "+MIPLEXECUTE:",   .suffix = "\r\n", .func = bc28_urc_receiver},
    {.prefix = "+MIPLPARAMETER:", .suffix = "\r\n", .func = bc28_urc_receiver},
    {.prefix = "+MIPLEVENT:",     .suffix = "\r\n", .func = bc28_urc_receiver},
};

os_err_t bc28_onenetnb_init(mo_bc28_t *module)
{
    OS_ASSERT(OS_NULL != module);
    os_err_t     result = OS_ERROR;
    at_parser_t *parser = OS_NULL;

    module->regist_cb = os_calloc(1, sizeof(mo_onenet_cb_t));
    if (OS_NULL == module->regist_cb)
    {
        ERROR("Module %s NB init failed, no enough memory!", module->parent.name, __func__);
        result = OS_ENOMEM;
        goto __exit;
    }

    /* create mq for urc manager */
    bc28_nb_mq = os_mq_create(BC28_URC_MQ_NAME,
                              sizeof(bc28_onenet_mq_msg_t),
                              BC28_URC_MQ_MSGNUM_MAX);
    if (OS_NULL == bc28_nb_mq)
    {
        ERROR("Module %s NB mq create failed, no enough memory!", module->parent.name);
        result = OS_ENOMEM;
        goto __exit;
    }

    /* create&start urc manager task */
    result = os_task_init(&bc28_nb_manager_task, 
                           BC28_URC_MANAGER_TASK_NAME,
                           bc28_urc_manager_task,
                           OS_NULL,
                           bc28_nb_manager_task_stack,
                           BC28_URC_MANAGER_STACK_SIZE,
                           BC28_URC_MANAGER_TASK_PRIORITY);
    if (OS_EOK != result)
    {
        ERROR("Module %s NB create manager task failed[%d]!", module->parent.name, result);
        goto __exit;
    }

    result = os_task_startup(&bc28_nb_manager_task);
    if (OS_EOK != result)
    {
        os_task_deinit(&bc28_nb_manager_task);
        goto __exit;
    }

    /* Set netconn urc table */
    parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, bc28_nb_urc_table, sizeof(bc28_nb_urc_table) / sizeof(at_urc_t));

    INFO("Module %s NB init success.", module->parent.name);
    return OS_EOK;

__exit:

    if (OS_NULL != module->regist_cb)
    {
        os_free(module->regist_cb);
        module->regist_cb = OS_NULL;
    }

    if (OS_NULL != bc28_nb_mq)
    {
        os_mq_destroy(bc28_nb_mq);
        bc28_nb_mq = OS_NULL;
    }

    return result;
}

void bc28_onenetnb_deinit(mo_bc28_t *module)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != bc28_nb_mq);

    bc28_onenet_mq_msg_t handle_msg;
    os_err_t             result     = OS_ERROR;
    os_size_t            recv_size  = 0;

    /* free buff in mq which alloced in bc28_urc_receiver */
    while (OS_TRUE)
    {
        memset(&handle_msg, 0, sizeof(bc28_onenet_mq_msg_t));
        result = os_mq_recv(bc28_nb_mq,
                            &handle_msg, 
                            sizeof(handle_msg), 
                            OS_NO_WAIT, 
                            &recv_size);
        if (OS_EOK != result) break;
        os_free((void *)handle_msg.data);
    }
    
    /* auto deinit urc manager task */
    // os_mq_control(bc28_nb_mq, OS_IPC_CMD_RESET, OS_NULL); /* TODO make sure */
    os_mq_destroy(bc28_nb_mq);
    bc28_nb_mq = OS_NULL;

    os_free(module->regist_cb);
    module->regist_cb = OS_NULL;

    return;
}

os_err_t bc28_onenetnb_cb_register(mo_object_t *module, mo_onenet_cb_t user_callbacks)
{
    mo_bc28_t *module = os_container_of(module, mo_bc28_t, parent);
    memcpy(module->regist_cb, &user_callbacks, sizeof(mo_onenet_cb_t));
    return OS_EOK;
}

#ifdef OS_USING_SHELL
os_err_t bc28_onenetnb_all(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);
    
    at_parser_t *parser = &module->parser;

    char resp_buff[256] = {0};
    at_resp_t at_resp   = {.buff      = resp_buff,
                           .buff_size = sizeof(resp_buff),
                           .line_num  = 0,
                           .timeout   = timeout};

    if (at_parser_exec_cmd_valist(parser, &at_resp, format, args) == OS_EOK)
    {
        return OS_EOK;
    }

    return OS_ERROR;
}
#endif

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
 * @file        m5311_onenet_nb.c
 *
 * @brief       m5311 module link kit onenet nb api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "at_parser.h"
#include "m5311_onenet_nb.h"
#include "mo_onenet_nb.h"
#include "mo_general.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MO_LOG_TAG "m5311.onenet_nb"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define M5311_ONENET_EVENTID_BOOTSTRAP_OK       (2)
#define M5311_ONENET_EVENTID_BOOTSTRAP_FAIL     (3)
#define M5311_ONENET_EVENTID_REG_OK             (6)
#define M5311_ONENET_EVENTID_REG_FAIL           (7)
#define M5311_ONENET_EVENTID_UPDATE_OK          (11)
#define M5311_ONENET_EVENTID_LOGOUT             (15)
#define M5311_ONENET_EVENTID_NOTIFY_OK          (26)
#define M5311_ONENET_EVENTID_DELETE_BY_PLATFORM (31)
#define M5311_ONENET_EVENTID_DTLS_IP_AGINT      (32)

#define M5311_ONENET_EVENT_BOOTSTRAP_OK         (1L << 0)
#define M5311_ONENET_EVENT_BOOTSTRAP_FAIL       (1L << 1)
#define M5311_ONENET_EVENT_REG_OK               (1L << 2)
#define M5311_ONENET_EVENT_REG_FAIL             (1L << 3)
#define M5311_ONENET_EVENT_UPDATE_OK            (1L << 4)
#define M5311_ONENET_EVENT_LOGOUT               (1L << 5)
#define M5311_ONENET_EVENT_NOTIFY_OK            (1L << 6)
#define M5311_ONENET_EVENT_DELETE_BY_PLATFORM   (1L << 7)
#define M5311_ONENET_EVENT_DTLS_IP_AGINT        (1L << 8)

static os_err_t m5311_nb_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t m5311_nb_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_create, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    os_uint8_t   ref    = 0;
    at_parser_t *parser = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_128] = "AT+MIPLCREATE=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+MIPLCREATE:", "+MIPLCREATE:%hhu", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        result = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_delete, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLDELETE=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    return at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args);
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_createex, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &module->parser;

    os_uint8_t ref = 0;

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLCREATEEX=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+MIPLCREATEEX:", "+MIPLCREATEEX:%hhu", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        result = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_addobj, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLADDOBJ=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    return at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args);
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_delobj, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLDELOBJ=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    return (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args));
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_discoverrsp, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_128] = "AT+MIPLDISCOVERRSP=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_set_nmi, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLNMI=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_open, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    os_uint32_t  event  = 0;
    at_parser_t *parser = &module->parser;
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);

    m5311_nb_lock(&m5311->onenetnb_lock);

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLOPEN=";
    char resp_buff [AT_RESP_BUFF_SIZE_128] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .timeout   = os_tick_from_ms(timeout)};

    os_event_clear(&m5311->onenetnb_evt, M5311_ONENET_EVENT_REG_OK | M5311_ONENET_EVENT_REG_FAIL);

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&m5311->onenetnb_evt,
                           M5311_ONENET_EVENT_REG_OK | M5311_ONENET_EVENT_REG_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           &event);

    if (result != OS_EOK)
    {
        ERROR("Module %s recv register event timeout.", module->name);
        goto __exit;
    }

    if (M5311_ONENET_EVENT_REG_OK & event)
    {
        DEBUG("Module %s register to OneNET OK.", module->name);
        result = OS_EOK;
    }

    if (M5311_ONENET_EVENT_REG_FAIL & event)
    {
        ERROR("Module %s register to OneNET failed.", module->name);
        result = OS_ERROR;
    }

__exit:

    m5311_nb_unlock(&m5311->onenetnb_lock);

    return result;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_close, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    os_uint32_t  event  = 0;
    at_parser_t *parser = &module->parser;
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);

    m5311_nb_lock(&m5311->onenetnb_lock);

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLCLOSE=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .timeout   = os_tick_from_ms(timeout)};

    os_event_clear(&m5311->onenetnb_evt, M5311_ONENET_EVENT_LOGOUT);

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&m5311->onenetnb_evt,
                           M5311_ONENET_EVENT_LOGOUT,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           &event);

    if (result != OS_EOK)
    {
        ERROR("Module %s recv logout event timeout.", module->name);
        goto __exit;
    }

    if (M5311_ONENET_EVENT_LOGOUT & event)
    {
        DEBUG("Module %s recv logout event success.", module->name);
        result = OS_EOK;
    }

__exit:

    m5311_nb_unlock(&m5311->onenetnb_lock);

    return result;
}

os_err_t m5311_get_onenetnb_notify_ackid(const char *format, va_list args, os_uint16_t *id)
{
    /* if has ackid, must be the 11 element */
    int        num_count = 1;
    os_int32_t id_tmp    = -1;
    os_uint8_t qualifier;

    for (; *format; ++format)
    {
        if (*format == ',')
        {
            ++num_count;
        }

        if (*format != '%')
        {
            continue;
        }
        ++format; /* ignore % */

        /* Get the conversion qualifier */
        qualifier = 0;
        if ((*format == 'h') || (*format == 'l') || (*format == 'L'))
        {
            qualifier = *format;
            ++format;

            if ((qualifier == 'l') && (*format == 'l'))
            {
                qualifier = 'L';
                ++format;
            }
        }

        switch (*format)
        {
        case 'c':
        {
            id_tmp = va_arg(args, int);
            continue;
        }
        case 's':
        {
            va_arg(args, char *);
            continue;
        }
        case 'p':
        {
            va_arg(args, void *);
            continue;
        }
        case '%':
            continue;
        case 'o':
        case 'X':
        case 'x':
        case 'd':
        case 'i':
        case 'u':
            break;
        }

        if (qualifier == 'L')
        {
            id_tmp = (os_int32_t)va_arg(args, long long);
        }
        else if (qualifier == 'l')
        {
            id_tmp = (os_int32_t)va_arg(args, os_uint32_t);
        }
        else if (qualifier == 'h')
        {
            id_tmp = (os_int32_t)va_arg(args, os_int32_t);
        }
        else
        {
            id_tmp = (os_int32_t)va_arg(args, os_uint32_t);
        }

        if (num_count == 11) /* must 11 element */
        {
            *id = (os_uint16_t)id_tmp;
            return OS_EOK;
        }
    }

    return OS_ERROR;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_notify, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    os_uint32_t  event  = 0;
    at_parser_t *parser = &module->parser;
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);

    m5311_nb_lock(&m5311->onenetnb_lock);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};
    char tmp_format[AT_RESP_BUFF_SIZE_128] = "AT+MIPLNOTIFY=";

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .timeout   = os_tick_from_ms(timeout)};

    os_event_clear(&m5311->onenetnb_evt, M5311_ONENET_EVENT_NOTIFY_OK);

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&m5311->onenetnb_evt,
                           M5311_ONENET_EVENT_NOTIFY_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           &event);

    if (result != OS_EOK)
    {
        ERROR("Module %s recv notify OK event timeout.", module->name);
        goto __exit;
    }

    if (M5311_ONENET_EVENT_NOTIFY_OK & event)
    {
        DEBUG("Module %s recv notify OK event success.", module->name);
        result = OS_EOK;
    }

__exit:

    m5311_nb_unlock(&m5311->onenetnb_lock);

    return result;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_update, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    at_parser_t *parser   = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_DEF] = "AT+MIPLUPDATE=";
    char resp_buff [AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    return at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args);
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_get_write, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    module_mgr_resp_t *mgr = (module_mgr_resp_t *)resp;
    if (mgr->value == OS_NULL) /* value ptr is null */
    {
        return OS_ERROR;
    }

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &module->parser;

    char tmp_format[AT_RESP_BUFF_SIZE_128] = "AT+MIPLMGR=";
    char resp_buff [AT_RESP_BUFF_SIZE_256] = {0};

    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp,
                               "+MIPLWRITE:",
                               "+MIPLWRITE:%d,%d,%d,%d,%d,%d,%d,%s",
                               &mgr->ref,
                               &mgr->mid,
                               &mgr->objid,
                               &mgr->insid,
                               &mgr->resid,
                               &mgr->type,
                               &mgr->len,
                               mgr->value) > 0)
    {
        mgr->value[mgr->len] = '\0';
        result = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_writersp, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &module->parser;

    char tmp_format[128] = "AT+MIPLWRITERSP=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}

/* for inner */
DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_get_nmi, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(resp != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd(parser, &at_resp, "AT+MIPLNMI?") != OS_EOK) /* get nmi config */
    {
        goto __exit;
    }

    int ref, nnmi, nsmi;
    ref = nnmi = nsmi = 0;

    /* parser nmi config */
    if (at_resp_get_data_by_kw(&at_resp, "+MIPLNMI:", "+MIPLNMI:%d,%d,%d", &ref, &nnmi, &nsmi) <= 0)
    {
        goto __exit;
    }
    ((m5311_nmi_t *)resp)->ref  = ref;
    ((m5311_nmi_t *)resp)->nnmi = nnmi;
    ((m5311_nmi_t *)resp)->nsmi = nsmi;

    result = OS_EOK;

__exit:

    return result;
}

static void urc_onenetnb_miplnsmi_msg_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    os_int32_t  ref     = 0;
    os_int32_t  status  = 0;
    os_int32_t  num     = 0;

    if (0 >= sscanf(data, "+MIPLNSMI:%d,%d,%d", &ref, &status, &num))
    {
        ERROR("Module %s parser +MIPLNSMI info failed.", module->name);
        return;
    }

    if (status == 0)
    {
        WARN("Module %s OneNET: sent msg to base station port failed.", module->name);
    }
    else
    {
        DEBUG("Module %s OneNET: sent msg to base station port successfully.", module->name);
    }

    return;
}

static void urc_onenetnb_evt_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);
    os_int32_t   ref    = 0;
    os_int32_t   evtid  = 0;

    /* +MIPLEVENT:0,2\r\n */
    if (0 >= sscanf(data, "+MIPLEVENT:%d,%d", &ref, &evtid))
    {
        ERROR("Module %s parser +MIPLEVENT failed.", module->name);
        return;
    }

    switch (evtid)
    {
    case M5311_ONENET_EVENTID_BOOTSTRAP_OK:
        DEBUG("Module %s OneNET bootstrap OK.", module->name);
        break;

    case M5311_ONENET_EVENTID_BOOTSTRAP_FAIL:
        ERROR("Module %s OneNET bootstrap failed.", module->name);
        break;

    case M5311_ONENET_EVENTID_REG_OK:
        DEBUG("Module %s OneNET register OK.", module->name);
        os_event_send(&m5311->onenetnb_evt, M5311_ONENET_EVENT_REG_OK);
        break;

    case M5311_ONENET_EVENTID_REG_FAIL:
        ERROR("Module %s OneNET register failed.", module->name);
        os_event_send(&m5311->onenetnb_evt, M5311_ONENET_EVENT_REG_FAIL);
        break;

    case M5311_ONENET_EVENTID_UPDATE_OK:
        DEBUG("Module %s update OneNet platform OK.", module->name);
        break;

    case M5311_ONENET_EVENTID_LOGOUT:
        DEBUG("Module %s OneNET logout.", module->name);
        os_event_send(&m5311->onenetnb_evt, M5311_ONENET_EVENT_LOGOUT);
        break;

    case M5311_ONENET_EVENTID_NOTIFY_OK:
        DEBUG("Module %s OneNET notify report response OK.", module->name);
        os_event_send(&m5311->onenetnb_evt, M5311_ONENET_EVENT_NOTIFY_OK);
        break;

    case M5311_ONENET_EVENTID_DELETE_BY_PLATFORM:
        WARN("Module %s devicec deleted by OneNET platform.", module->name);
        break;

    case M5311_ONENET_EVENTID_DTLS_IP_AGINT:
        ERROR("Module %s DTLS IP aging.", module->name);
        break;

    default:
        ERROR("Module %s OneNET eventid unidentify.", module->name);
        break;
    }

    return;
}

static at_urc_t onenet_nb_urc_table[] = {
    {.prefix = "+MIPLEVENT:",  .suffix = "\r\n", .func = urc_onenetnb_evt_recv_func},
    {.prefix = "+MIPLNSMI:",   .suffix = "\r\n", .func = urc_onenetnb_miplnsmi_msg_func},
};

void m5311_onenetnb_init(mo_m5311_t *module)
{
    /* Set onenet nb urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, onenet_nb_urc_table, sizeof(onenet_nb_urc_table) / sizeof(onenet_nb_urc_table[0]));
}

#ifdef OS_USING_SHELL
DEFINE_M5311_ONENET_FUNC(m5311_onenetnb_all, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &module->parser;

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}
#endif

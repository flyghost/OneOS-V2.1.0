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
 * @file        m5310a_onenet_nb.c
 *
 * @brief       m5310-a module link kit onenet nb api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "at_parser.h"
#include "m5310a_onenet_nb.h"
#include "mo_onenet_nb.h"
#include "mo_general.h"

#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "m5310a.onenet_nb"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_create, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    os_uint8_t   ref    = 0;
    at_parser_t *parser = &self->parser;

    char tmp_format[128] = "AT+MIPLCREATE=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+MIPLCREATE:", "+MIPLCREATE:%hhu", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        result              = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_createex, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    os_uint8_t ref            = 0;
    char       tmp_format[64] = "AT+MIPLCREATEEX=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+MIPLCREATEEX:", "+MIPLCREATEEX:%hhu", &ref) > 0)
    {
        *(os_uint8_t *)resp = ref;
        result              = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_addobj, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    char tmp_format[64] = "AT+MIPLADDOBJ=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_discoverrsp, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    char tmp_format[128] = "AT+MIPLDISCOVERRSP=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_set_nmi, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    char tmp_format[28] = "AT+MIPLNMI=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_open, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    char tmp_format[28] = "AT+MIPLOPEN=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[128] = {0};

    at_resp_t at_resp = {.buff     = resp_buff,
                        .buff_size = sizeof(resp_buff),
                        .line_num  = 3,
                        .timeout   = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (OS_NULL != at_resp_get_line_by_kw(&at_resp, "+MIPLEVENT:0,6"))
    {
        result = OS_EOK; /* success */
    }

__exit:

    return result;
}

os_err_t m5310_get_onenetnb_notify_ackid(const char *format, va_list args, os_uint16_t *id)
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

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_notify, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_bool_t    is_ack = OS_FALSE;
    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;
    m5310a_nmi_t nmi;
    if (m5310_onenetnb_get_nmi(self, -1, &nmi, OS_NULL, args) != OS_EOK) /* get nsmi */
    {
        return OS_ERROR;
    }

    os_uint16_t ackid_in  = 0;
    os_int32_t  ackid_out = 0;
    if (m5310_get_onenetnb_notify_ackid(format, args, &ackid_in) == OS_EOK) /* whether has ackid */
    {
        is_ack = OS_TRUE;
    }

    char tmp_format[128] = "AT+MIPLNOTIFY=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    int line_num = 1; /* normal: \r\n OK\r\n */
    line_num = ((nmi.nsmi == 1) ? line_num + 1 : line_num);
    line_num = (is_ack ? line_num + 1 : line_num);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff     = resp_buff,
                        .buff_size = sizeof(resp_buff),
                        .line_num  = line_num,
                        .timeout   = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (!is_ack)
    {
        result = OS_EOK;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+MIPLEVENT:0,26", "+MIPLEVENT:0,26,%d", &ackid_out) <= 0)
    {
        goto __exit;
    }

    if (ackid_in == ackid_out)
    {
        result = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_update, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    os_uint8_t  ref            = 0;
    os_uint16_t event_id       = 0;
    char        tmp_format[64] = "AT+MIPLUPDATE=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .line_num = 2, .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, tmp_format, args) != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+MIPLEVENT:", "+MIPLEVENT:%hhu,%hu", &ref, &event_id) <= 0)
    {
        goto __exit;
    }
    if (event_id == 11)
    {
        result = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_get_write, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    module_mgr_resp_t *mgr = (module_mgr_resp_t *)resp;
    if (mgr->value == OS_NULL) /* value ptr is null */
    {
        return OS_ERROR;
    }

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    char tmp_format[128] = "AT+MIPLMGR=";
    strncpy(tmp_format + strlen(tmp_format), format, strlen(format));

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

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
        result               = OS_EOK;
    }

__exit:

    return result;
}

DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_writersp, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

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
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_get_nmi, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(resp != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

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
    ((m5310a_nmi_t *)resp)->ref  = ref;
    ((m5310a_nmi_t *)resp)->nnmi = nnmi;
    ((m5310a_nmi_t *)resp)->nsmi = nsmi;
    result                       = OS_EOK;

__exit:

    return result;
}

#ifdef OS_USING_SHELL
DEFINE_M5310A_ONENET_FUNC(m5310_onenetnb_all, ONENET_NB_FUNC_ARGS)
{
    OS_ASSERT(self != OS_NULL);
    OS_ASSERT(format != OS_NULL);

    os_err_t     result = OS_ERROR;
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = os_tick_from_ms(timeout)};

    if (at_parser_exec_cmd_valist(parser, &at_resp, format, args) == OS_EOK)
    {
        result = OS_EOK;
    }

    return result;
}
#endif

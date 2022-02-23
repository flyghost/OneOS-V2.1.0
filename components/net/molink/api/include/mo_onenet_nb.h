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
 * @file        mo_onenet_nb.h
 *
 * @brief       module link kit onenet nb api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_ONENET_H__
#define __MO_ONENET_H__

#include "mo_object.h"
#include "mo_ipaddr.h"
#include <oneos_config.h>

#ifdef MOLINK_USING_ONENET_NB_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NBCONFIG_AUTHCODE_MAX_LEN       (16)
#define NBCONFIG_PSK_MAX_LEN            (16)
#define NBEVENT_TIME_STAMP_MAX_LEN      (14)
#define NBCONFIG_AUTHCODE_MAX_STR_LEN   (NBCONFIG_AUTHCODE_MAX_LEN + 3)
#define NBCONFIG_PSK_MAX_STR_LEN        (NBCONFIG_PSK_MAX_LEN + 3)
#define NBEVENT_TIME_STAMP_MAX_STR_LEN  (NBEVENT_TIME_STAMP_MAX_LEN + 1)

/**
 ***********************************************************************************************************************
 * @struct      mo_config_resp
 *
 * @brief       OneNET access configurations (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_config_resp
{
    os_int8_t   guide_mode_enable;                  /* Guide mode enable; -1:invalid */
    os_int8_t   ip[IPADDR_MAX_STR_LEN + 1];         /* Server ip */
    os_uint16_t port;                               /* Server port; [0-15535] */
    os_uint8_t  rsp_timeout;                        /* Respond timeout; 0:invalid, range:[2-20](s) */
    os_int8_t   obs_autoack_enable;                 /* Obs autoack mode enable; -1:invalid */
    os_int8_t   auth_enable;                        /* Auth mode enable; -1:invalid */
    os_int8_t   auth_code[NBCONFIG_AUTHCODE_MAX_STR_LEN];
                                                    /* Auth code string with Double quotation marks */
    os_int8_t   dtls_enable;                        /* DTLS mode enable; -1:invalid */
    os_int8_t   psk[NBCONFIG_PSK_MAX_STR_LEN];      /* PSK string with Double quotation marks */
    os_int8_t   write_format;                       /* Write format; -1:invalid */
    os_int8_t   buf_cfg;                            /* Downlink data cache configuration; -1:invalid */
    os_int8_t   buf_urc_mode;                       /* Downlink data cache enabled indication;-1:invalid */
} mo_config_resp_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_discover
 *
 * @brief       The returning messages of URC code: +MIPLDISCOVER (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_discover
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_int32_t  msg_id;                             /* MessageID */
    os_int32_t  obj_id;                             /* ObjectID */
} mo_onenet_discover_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_observe
 *
 * @brief       The returning messages of URC code: +MIPLOBSERVE (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_observe
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_int32_t  msg_id;                             /* MessageID */
    os_int8_t   flag;                               /* Observe enable flag */
    os_int32_t  obj_id;                             /* ObjectID */
    os_int32_t  ins_id;                             /* InstanceID; -1:request all ins */
    os_int32_t  res_id;                             /* ResourceID; -1:request all res */
} mo_onenet_observe_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_read
 *
 * @brief       The returning messages of URC code: +MIPLREAD (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_read
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_int32_t  msg_id;                             /* MessageID */
    os_int32_t  obj_id;                             /* ObjectID */
    os_int32_t  ins_id;                             /* InstanceID; -1:request all ins */
    os_int32_t  res_id;                             /* ResourceID; -1:request all res */
} mo_onenet_read_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_write
 *
 * @brief       The returning messages of URC code: +MIPLWRITE (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_write
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_int32_t  msg_id;                             /* MessageID */
    os_int32_t  obj_id;                             /* ObjectID */
    os_int32_t  ins_id;                             /* InstanceID */
    os_int32_t  res_id;                             /* ResourceID */
    os_int8_t   value_type;                         /* Value type; @see +MIPLWRITE */
    os_int32_t  len;                                /* Value len;  @see +MIPLWRITE */
    os_int8_t   flag;                               /* Message flag, indicate messages position of value */
    os_int32_t  index;                              /* Index of write request messages */
} mo_onenet_write_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_execute
 *
 * @brief       The returning messages of URC code: +MIPLEXECUTE (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_execute
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_int32_t  msg_id;                             /* MessageID */
    os_int32_t  obj_id;                             /* ObjectID */
    os_int32_t  ins_id;                             /* InstanceID */
    os_int32_t  res_id;                             /* ResourceID */
    os_int32_t  len;                                /* Execute arguements length; (optional) */
} mo_onenet_execute_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_parameter
 *
 * @brief       The returning messages of URC code: +MIPLPARAMETER (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_parameter
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_int32_t  msg_id;                             /* MessageID */
    os_int32_t  obj_id;                             /* ObjectID */
    os_int32_t  ins_id;                             /* InstanceID; -1:request all ins */
    os_int32_t  res_id;                             /* ResourceID; -1:request all res, checkout IPSO */
    os_int32_t  len;                                /* Parameter length */
} mo_onenet_parameter_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_event
 *
 * @brief       The returning messages of URC code: +MIPLEVENT (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_event
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_uint8_t  evt_id;                             /* EventID; 0:invalid */
    os_int32_t  extend;                             /* Extend;                      (optional) -1:invalid */
    os_uint16_t ack_id;                             /* AckID;                       (optional)  0:invalid range:[1-65535] */
    os_uint8_t  time_stamp[NBEVENT_TIME_STAMP_MAX_STR_LEN];
                                                    /* Time stamp;                  (optional) YYYYMMDDHHmmSS */
    os_int8_t   cache_command_flag;                 /* Cache command enable Flag;   (optional) -1:invalid */
} mo_onenet_event_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_onenetnb_cb
 *
 * @brief       Struct contains urc messages receiving callbacks (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef void (*discover_notify_cb_t)  (mo_onenet_discover_t  *discover_notify);
typedef void (*observe_notify_cb_t)   (mo_onenet_observe_t   *observe_notify);
typedef void (*read_notify_cb_t)      (mo_onenet_read_t      *read_notify);
typedef void (*write_notify_cb_t)     (mo_onenet_write_t     *write_notify, const char *value);
typedef void (*execute_notify_cb_t)   (mo_onenet_execute_t   *execute_notify, const char *arguments);
typedef void (*parameter_notify_cb_t) (mo_onenet_parameter_t *parameter_notify, const char *parameter);
typedef void (*event_notify_cb_t)     (mo_onenet_event_t     *event_notify);

typedef struct mo_onenetnb_cb
{
    discover_notify_cb_t    discover_notify_cb;
    observe_notify_cb_t     observe_notify_cb;
    read_notify_cb_t        read_notify_cb;
    write_notify_cb_t       write_notify_cb;
    execute_notify_cb_t     execute_notify_cb;
    parameter_notify_cb_t   parameter_notify_cb;
    event_notify_cb_t       event_notify_cb;
} mo_onenet_cb_t;

/**
 ***********************************************************************************************************************
 * @struct      module_mgr_resp
 *
 * @brief       respond messages given by mo_onenetnb_get_write (Now support Quectel platform only)
 ***********************************************************************************************************************
 */
typedef struct module_mgr_resp
{
    os_uint32_t ref;                                /* The descriptor of molink module instance */
    os_uint32_t mid;                                /* +MIPLREAD mid */
    os_uint32_t objid;                              /* ObjectID */
    os_uint32_t insid;                              /* InstanceID */
    os_uint32_t resid;                              /* ResourceID */
    os_uint32_t type;                               /* Value type */
    os_uint32_t len;                                /* Value length */
    os_int8_t   *value;                             /* Value */
} module_mgr_resp_t;

#define ONENET_NB_FUNC_ARGS (mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args)

/**
 ***********************************************************************************************************************
 * @struct      mo_onenet_ops
 *
 * @brief       MoLink supports OneNet options
 ***********************************************************************************************************************
 */
typedef struct mo_onenet_ops
{
#define DEFINE_ONENET_OPT_FUNC(NAME, ARGS) os_err_t (*NAME) ARGS
    DEFINE_ONENET_OPT_FUNC(onenetnb_get_config, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_set_config, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_create, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_createex, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_delete, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_addobj, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_delobj, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_nmi, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_open, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_close, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_discoverrsp, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_observersp, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_readrsp, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_writersp, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_executersp, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_parameterrsp, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_notify, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_update, ONENET_NB_FUNC_ARGS);
    DEFINE_ONENET_OPT_FUNC(onenetnb_get_write, ONENET_NB_FUNC_ARGS);
    os_err_t (*onenetnb_cb_register)(mo_object_t *module, mo_onenet_cb_t user_callbacks);
#ifdef OS_USING_SHELL
    DEFINE_ONENET_OPT_FUNC(onenetnb_all, ONENET_NB_FUNC_ARGS);
#endif
} mo_onenet_ops_t;

os_err_t mo_onenetnb_get_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_set_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_create(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_createex(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_delete(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_addobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_delobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_nmi(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_open(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_close(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_discoverrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_observersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_readrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_writersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_executersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_parameterrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_notify(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_update(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_get_write(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
os_err_t mo_onenetnb_cb_register(mo_object_t *module, mo_onenet_cb_t user_callbacks);
#ifdef OS_USING_SHELL
os_err_t mo_onenetnb_all(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_ONENET_NB_OPS */

#endif /* __MO_ONENET_H__ */

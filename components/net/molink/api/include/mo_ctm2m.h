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
 * @file        mo_ctm2m.h
 *
 * @brief       module link kit ctm2m api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-21   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_CTM2M_H__
#define __MO_CTM2M_H__

#include "mo_object.h"
#include "mo_ipaddr.h"
#include "os_mq.h"
#include "os_task.h"
#include "os_mutex.h"
#include "os_sem.h"
#include <oneos_config.h>

#ifdef MOLINK_USING_CTM2M_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_string
 *
 * @brief       molink ctm2m basic string type
 ***********************************************************************************************************************
 */
typedef struct ctm2m_string
{
    char     *data;
    os_size_t len;
} ctm2m_string_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_uri (not support mb26)
 *
 * @brief       uniform resource identifier information
 ***********************************************************************************************************************
 */
typedef struct ctm2m_uri
{
    os_int32_t obj_id;
    os_int32_t ins_id;
    os_int32_t res_id;
} ctm2m_uri_t;


/***********************************************************************************************************************
 * @brief       Create session related definitions
 **********************************************************************************************************************/

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_reg_list (not support mb26)
 *
 * @brief       regist list information
 ***********************************************************************************************************************
 */
typedef struct ctm2m_reg_list
{
    os_int32_t obj_id;
    os_int32_t ins_id;
} ctm2m_reg_list_t;


/***********************************************************************************************************************
 * @brief       UE configuration related definitions
 **********************************************************************************************************************/
typedef enum ctm2m_observe_cfg
{
    CTM2M_OBS_NULL      = -1,
    CTM2M_OBS_SET       =  0,
    CTM2M_OBS_CANCEL    =  1,
    CTM2M_OBS_RSP       =  8,
    CTM2M_OBS_WITH_PARM =  9,
} ctm2m_observe_cfg_t;

typedef enum ctm2m_dataform
{
    CTM2M_DATAFORM_NULL = 0,
    CTM2M_DATAFORM_TLV,                 /* 1: TLV format(application/vnd.oma.lwm2m+tlv) */
    CTM2M_DATAFORM_OPAQUE,              /* 2: opaque format(application/octet-stream) */
    CTM2M_DATAFORM_COAP = 6,            /* 6: CoAP Param */
    CTM2M_DATAFORM_TEXT,                /* 7: TEXT format(text/plain) */
    CTM2M_DATAFORM_JSON,                /* 8: JSON format(application/vnd.oma.lwm2m+json) */
    CTM2M_DATAFORM_CLP,                 /* 9: Core Link Param format(application/link-format) */
} ctm2m_dataform_t;

typedef enum ctm2m_uemode
{
    CTM2M_UEMODE_NULL = 0,              /* invalid value */
    CTM2M_UEMODE_IDAUTH,                /* 1: IDAuth_Mode */
    CTM2M_UEMODE_TAUTIMER_UPDATE,       /* 2: Auto_TAUTimer_Update */
    CTM2M_UEMODE_UQ_MODE,               /* 3: ON_UQMode */
//    CTM2M_UEMODE_CE_LVL2_POLICY,        /* 4: ON_CELevel2Policy */ //UMN601 undefine
    CTM2M_UEMODE_HEARTBEAT_UPDATE,      /* 5: Auto_Heartbeat */
    CTM2M_UEMODE_WAKEUP_NOTIFY,         /* 6: Wakeup_Notify */
    CTM2M_UEMODE_PROTOCOL_MODE,         /* 7: Protocol_Mode */
} ctm2m_uemode_t;

typedef enum ctm2m_auth_cfg
{
    CTM2M_AUTH_NULL = 0,                /* invalid value */
    CTM2M_AUTH_DEFAULT,                 /* default value, no authentication string */
    CTM2M_AUTH_OUTSIDE_SIMD,            /* SIMD authentication string from outside of module */
    CTM2M_AUTH_OUTSIDE_SM9,             /* SM9  authentication string from outside of module */
    CTM2M_AUTH_INSIDE_SIMD,             /* SIMD authentication string from inside of module  */
    CTM2M_AUTH_INSIDE_SM9,              /* SM9  authentication string from inside of module  */
    CTM2M_AUTH_OUTSIDE_IMEI_IMSI,       /* IMEI-IMSI authentication string from inside of module  */
} ctm2m_auth_cfg_t;

typedef enum ctm2m_tautimer_cfg
{
    CTM2M_TAU_NULL = 0,                 /* invalid value */
    CTM2M_TAU_DEFAULT,                  /* default no action */
    CTM2M_TAU_NOTIFY_MCU_ON,            /* notify MCU */
    CTM2M_TAU_NOTIFY_MCU_OFF,           /* not notify MCU, auto update inside of module */
} ctm2m_tautimer_cfg_t;

typedef enum ctm2m_uq_cfg
{
    CTM2M_UQ_NULL = 0,                  /* invalid value */
    CTM2M_UQ_MODE_OFF,                  /* UQ mode off */
    CTM2M_UQ_MODE_ON,                   /* UQ mode on */
} ctm2m_uq_cfg_t;

typedef enum ctm2m_ce_cfg
{
    CTM2M_CE_LVL2_NULL = 0,             /* invalid value */
    CTM2M_CE_LVL2_SEND_ON,              /* default send under CE level2 */
    CTM2M_CE_LVL2_SEND_OFF,             /* not send under CE level2 */
} ctm2m_ce_cfg_t;

typedef enum ctm2m_heartbeat_cfg
{
    CTM2M_AUTO_HEARTBEAT_NULL = 0,      /* invalid value */
    CTM2M_AUTO_HEARTBEAT_OFF,           /* no auto heartbeat */
    CTM2M_AUTO_HEARTBEAT_ON,            /* default auto heartbeat */
} ctm2m_heartbeat_cfg_t;

typedef enum ctm2m_wakeup_cfg
{
    CTM2M_WAKEUP_NOTIFY_NULL = 0,       /* invalid value */
    CTM2M_WAKEUP_NOTIFY_ON,             /* not notify to MCU */
    CTM2M_WAKEUP_NOTIFY_OFF,            /* default notify to MCU */
} ctm2m_wakeup_cfg_t;

typedef enum ctm2m_protocol_cfg
{
    CTM2M_PROTOCOL_NULL = 0,            /* invalid value */
    CTM2M_PROTOCOL_NOTIFY_ON,           /* protocol mode normal */
    CTM2M_PROTOCOL_NOTIFY_OFF,          /* protocol mode enhance */
} ctm2m_protocol_cfg_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_ue_info
 *
 * @brief       molink ctm2m UE info @ref mo_ctm2m_get_ue_cfg
 ***********************************************************************************************************************
 */
typedef struct ctm2m_ue_info
{
    ctm2m_auth_cfg_t         auth_mode;
    ctm2m_tautimer_cfg_t     tau_timer_mode;
    ctm2m_uq_cfg_t           uq_mode;
    ctm2m_ce_cfg_t           ce_mode;
    ctm2m_heartbeat_cfg_t    heartbeat_mode;
    ctm2m_wakeup_cfg_t       wakeup_mode;
    ctm2m_protocol_cfg_t     protocol_mode;
} ctm2m_ue_info_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_ue_cfg
 *
 * @brief       molink ctm2m UE setting @ref mo_ctm2m_set_ue_cfg
 ***********************************************************************************************************************
 */
typedef struct ctm2m_ue_cfg
{
    ctm2m_uemode_t mode;
    os_uint32_t    cfg;
} ctm2m_ue_cfg_t;


/***********************************************************************************************************************
 * @brief       Notify session related definitions
 **********************************************************************************************************************/

typedef enum ctm2m_notify_type
{
    CTM2M_NTF_TYPE_NULL = 0,
    CTM2M_NTF_TYPE_REG,
    CTM2M_NTF_TYPE_OBS,
    CTM2M_NTF_TYPE_UPDATE,
    CTM2M_NTF_TYPE_PING,
    CTM2M_NTF_TYPE_DEREG,
    CTM2M_NTF_TYPE_SEND,
    CTM2M_NTF_TYPE_LWSTATUS,
} ctm2m_notify_type_t;

typedef enum ctm2m_notify_err
{
    CTM2M_NTF_E_SUCCESS              = 0,            /*  0: success */
    CTM2M_NTF_E_TIMEOUT              = 1,            /*  1: timeout */
    CTM2M_NTF_E_NO_SENDOUT_PACK      = 2,            /*  2: not send out packet */
    CTM2M_NTF_E_RECV_RST             = 9,            /*  9: receive platform RST packet and mean can’t send UL to platform */
    CTM2M_NTF_E_PARM_INVALID         = 10,           /* 10: parameter error */
    CTM2M_NTF_E_UNKNOWN_ERR          = 11,           /* 11: other errors */
    CTM2M_NTF_E_AUTH_FAILED          = 13,           /* 13: authentication error */
    CTM2M_NTF_E_UE_NOT_LOGIN         = 14,           /* 14: UE not login */
    CTM2M_NTF_E_VER_MISMATCH         = 22,           /* 22: iot protocol or lwm2m version mismatch */
    CTM2M_NTF_E_SESSION_INVALID      = 24,           /* 24: lwm2m session invalid */
    CTM2M_NTF_E_SESSION_LOAD_FAILED  = 25,           /* 25: session load failure when quitting from sleep or after reboot */
    CTM2M_NTF_E_ENGINE_ABNORMAL      = 26,           /* 26: Engine abnormal, need reboot by MCU */
    CTM2M_NTF_E_TAU_DUE              = 28,           /* 28: TAU is due */
    CTM2M_NTF_E_SESSION_LOAD_SUCCESS = 29,           /* 29: session load success when quitting from sleep or after reboot */
    CTM2M_NTF_E_SENT_ALREADY         = 31,           /* 31: packet is already sent out */
    CTM2M_NTF_E_NO_OBJECT19          = 32,           /* 32: object 19 not exist */
} ctm2m_notify_err_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_notify
 *
 * @brief       molink ctm2m notify callback info
 ***********************************************************************************************************************
 */
typedef struct ctm2m_notify
{
    ctm2m_notify_type_t   type;
    ctm2m_notify_err_t    status;
    os_int32_t            msg_id;
} ctm2m_notify_t;


/***********************************************************************************************************************
 * @brief       Request session related definitions
 **********************************************************************************************************************/

typedef enum ctm2m_request_type
{
    CTM2M_REQ_TYPE_READ = 0,                    /* 0: Read */
    CTM2M_REQ_TYPE_OBSERVE,                     /* 1: Observe */
    CTM2M_REQ_TYPE_WRITE,                       /* 2: Write */
    CTM2M_REQ_TYPE_WRITE_PARTICAL,              /* 3: Write-Partial */
    CTM2M_REQ_TYPE_WRITE_ATTRIBUTE,             /* 4: Write-Attribute */
    CTM2M_REQ_TYPE_DISCOVER,                    /* 5: Discover */
    CTM2M_REQ_TYPE_EXECUTE,                     /* 6: Execute */
    CTM2M_REQ_TYPE_CREATE,                      /* 7: Create */
    CTM2M_REQ_TYPE_DELETE,                      /* 8: Delete */
} ctm2m_request_type_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_request
 *
 * @brief       molink ctm2m request callback info
 ***********************************************************************************************************************
 */
typedef struct ctm2m_request
{
    os_int32_t           msg_id;
    ctm2m_request_type_t type;
    ctm2m_string_t       token;
    ctm2m_string_t       uri;
    ctm2m_observe_cfg_t  observe;
    ctm2m_dataform_t     dataformate;
    ctm2m_string_t       data;
} ctm2m_request_t;


/***********************************************************************************************************************
 * @brief       Send session related definitions
 **********************************************************************************************************************/

typedef enum ctm2m_send_mode
{
    CTM2M_SEND_MODE_CON = 0, /* 0---CON mode */
    CTM2M_SEND_MODE_NON,     /* 1---NON mode */
    CTM2M_SEND_MODE_NON_RAI, /* 2---NON with RAI flag */
    CTM2M_SEND_MODE_CON_RAI, /* 3---CON with RAI flag */
} ctm2m_send_mode_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_send
 *
 * @brief       molink ctm2m send info struct
 ***********************************************************************************************************************
 */
typedef struct ctm2m_send
{
    ctm2m_send_mode_t mode;                   /* (Optional) send mode */
    ctm2m_string_t    msg;                    /* send data            */
} ctm2m_send_t;


/***********************************************************************************************************************
 * @brief       Respond session related definitions
 **********************************************************************************************************************/

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_resp
 *
 * @brief       molink ctm2m resp info struct
 ***********************************************************************************************************************
 */
typedef struct ctm2m_resp
{
    os_int32_t          msg_id;               /* get from callback */
    ctm2m_string_t      token;                /* get from callback */
    os_int32_t          resp_code;
    ctm2m_string_t      uri;                  /* get from callback */
    ctm2m_observe_cfg_t observe;              /* see spec. */
    ctm2m_dataform_t    dataformate;          /* (optional) dataformate */
    ctm2m_string_t      data;                 /* (optional) resp data */
} ctm2m_resp_t;


/***********************************************************************************************************************
 * @brief       Update session related definitions
 **********************************************************************************************************************/

typedef enum ctm2m_binding_mode
{
    CTM2M_BINDING_NULL = 0,                     /* 0: Binding Not set */
    CTM2M_BINDING_UQ_MODE,                      /* 1: Binding UQ mode */
    CTM2M_BINDING_U_MODE,                       /* 2: Binding U mode */
} ctm2m_binding_mode_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_update
 *
 * @brief       molink ctm2m update info struct
 ***********************************************************************************************************************
 */
typedef struct ctm2m_update
{
    ctm2m_binding_mode_t mode;                  /* (optional) Binding mode */
    ctm2m_string_t       obj_list;              /* (optional) Object list  */
} ctm2m_update_t;


/***********************************************************************************************************************
 * @brief       Create session related definitions
 **********************************************************************************************************************/

/**
 ***********************************************************************************************************************
 * @cb          ctm2m_notify_cb_t/ctm2m_request_cb_t/ctm2m_receive_cb_t
 *
 * @brief       ctm2m callbacks
 ***********************************************************************************************************************
 */
typedef void (*ctm2m_notify_cb_t)  (ctm2m_notify_t notify);
typedef void (*ctm2m_request_cb_t) (ctm2m_request_t request);
typedef void (*ctm2m_receive_cb_t) (ctm2m_string_t data);

/**
 ***********************************************************************************************************************
 * @struct      ctm2m_create_parm_t
 *
 * @brief       molink ctm2m create session's parameters
 ***********************************************************************************************************************
 */
typedef struct ctm2m_create_parm
{
    ctm2m_string_t      server_ip;                          /* LWM2M server IP address, void relying on ip_addr_t */
    os_uint32_t         port;                               /* Port number for LWM2M server */
    os_uint32_t         lifetime;                           /* Lifetime for LWM2M server and unit is second with
                                                               minimum value 300 */
    ctm2m_string_t      obj_ins_list;                       /* (Optional) with format like </3303/0>,</3303/1> */
    ctm2m_notify_cb_t   notify_cb;                          /* callback: server notify message */
    ctm2m_request_cb_t  request_cb;                         /* callback: server request command */
    ctm2m_receive_cb_t  receive_cb;                         /* callback: receive server data */
} ctm2m_create_parm_t;

/**
 ***********************************************************************************************************************
 * @struct      ctm2m
 *
 * @brief       molink ctm2m basic handler
 ***********************************************************************************************************************
 */
typedef struct ctm2m
{
    mo_object_t        *module;
    ctm2m_notify_cb_t   notify_cb;                          /* callback: server notify message */
    ctm2m_request_cb_t  request_cb;                         /* callback: server request command */
    ctm2m_receive_cb_t  receive_cb;                         /* callback: receive server data */
    os_mq_t            *ctm2m_mq;
    os_task_t          *ctm2m_task;
    os_mutex_t         *ctm2m_mutex;
    os_sem_t           *ctm2m_sem;
    os_int32_t          msg_id;
} ctm2m_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_ctm2m_ops
 *
 * @brief       MoLink supports CTM2M options
 ***********************************************************************************************************************
 */
typedef struct mo_ctm2m_ops
{
    ctm2m_t *(*create)         (mo_object_t *module, ctm2m_create_parm_t parm);
    os_err_t (*destroy)        (ctm2m_t *handle);
    os_err_t (*set_ue_cfg)     (ctm2m_t *handle, ctm2m_ue_cfg_t cfg);
    os_err_t (*get_ue_cfg)     (ctm2m_t *handle, ctm2m_ue_info_t *cfg_info);
    os_err_t (*registering)    (ctm2m_t *handle);
    os_err_t (*deregistering)  (ctm2m_t *handle);
    os_err_t (*send)           (ctm2m_t *handle, ctm2m_send_t send, os_int32_t *send_msg_id);
    os_err_t (*resp)           (ctm2m_t *handle, ctm2m_resp_t resp);
    os_err_t (*update)         (ctm2m_t *handle, ctm2m_update_t update);
} mo_ctm2m_ops_t;

ctm2m_t *ctm2m_create(mo_object_t *module, ctm2m_create_parm_t parm);
os_err_t ctm2m_destroy(ctm2m_t *handle);

os_err_t ctm2m_set_ue_cfg(ctm2m_t *handle, ctm2m_ue_cfg_t cfg);
os_err_t ctm2m_get_ue_cfg(ctm2m_t *handle, ctm2m_ue_info_t *cfg_info);

os_err_t ctm2m_register(ctm2m_t *handle);
os_err_t ctm2m_deregister(ctm2m_t *handle);

os_err_t ctm2m_send(ctm2m_t *handle, ctm2m_send_t send, os_int32_t *send_msg_id);
os_err_t ctm2m_resp(ctm2m_t *handle, ctm2m_resp_t resp);
os_err_t ctm2m_update(ctm2m_t *handle, ctm2m_update_t update);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_CTM2M_OPS */

#endif /* __MO_CTM2M_H__ */

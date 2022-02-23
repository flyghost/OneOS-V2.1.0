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
 * @file        mo_netserv.h
 *
 * @brief       module link kit netservice api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_NETSERV_H__
#define __MO_NETSERV_H__

#include "mo_type.h"
#include "mo_object.h"

#ifdef MOLINK_USING_NETSERV_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CELL_ID_MAX_LEN             (28)

#define PSM_TIMER_MAX_STR_LEN       (9)             /* 3GPP timer len 8 plus 1 byte for character array's '\0' */
#define REG_TAC_MAX_STR_LEN         (5)             /* 2 bytes hex plus 1 byte for character array's '\0' */
#define REG_CELL_ID_MAX_STR_LEN     (9)             /* 4 bytes hex plus 1 byte for character array's '\0' */
#define DRX_EXTEND_MAX_STR_LEN      (5)             /* Extended DRX parameters information element string len */

/**
 ***********************************************************************************************************************
 * @enum        ps_attach_state_t
 *
 * @brief       enum type that indicate the molink module PS attach state
 ***********************************************************************************************************************
 */
typedef enum ps_attach_state
{
    DETACHED = 0,
    ATTACHED,
    UNDEF_ATTACH_STATE
} ps_attach_state_t;

/**
 ***********************************************************************************************************************
 * @enum        pdp_context_state_t
 *
 * @brief       enum type that indicate the molink module PDP context state
 ***********************************************************************************************************************
 */
typedef enum pdp_context_state
{
    DEACTIVATED = 0,
    ACTIVATED,
    UNDEF_CGACT_STATE
} pdp_context_state_t;

/**
 ***********************************************************************************************************************
 * @struct      eps_reg_info_t
 *
 * @brief       struct that holds molink module mo_get_reg's returning information
 ***********************************************************************************************************************
 */
typedef struct eps_registration_info
{
    os_uint8_t reg_n;
    os_uint8_t reg_stat;                            /* EPS registration status                              */
    char       tac[REG_TAC_MAX_STR_LEN];            /* tracking area code, HEX string                       */
    char       cell_id[REG_CELL_ID_MAX_STR_LEN];    /* cell ID, HEX string                                  */
    os_uint8_t act;                                 /* Access technology of the registered network          */
    os_uint8_t cause_type;                          /* indicates the type of <reject_cause>                 */
    os_uint8_t reject_cause;                        /* contains the cause of the failed registration        */
    char       active_time[PSM_TIMER_MAX_STR_LEN];  /* GPRS Timer 2 IE in 3GPP TS 24.008 Table 10.5.163     */
    char       periodic_tau[PSM_TIMER_MAX_STR_LEN]; /* GPRS Timer 3 IE in 3GPP TS 24.008 Table 10.5.163a    */
} eps_reg_info_t;

/**
 ***********************************************************************************************************************
 * @struct      t5g_reg_info_t
 *
 * @brief       struct that holds molink module mo_get_5g_reg's returning information
 ***********************************************************************************************************************
 */
typedef struct t5g_registration_info
{
    os_uint8_t  reg_n;
    os_uint8_t  reg_stat;                            /* EPS registration status                              */
    char        tac[REG_TAC_MAX_STR_LEN];            /* tracking area code, HEX string                       */
    char        cell_id[REG_CELL_ID_MAX_STR_LEN];    /* cell ID, HEX string                                  */
    os_uint8_t  act;                                 /* Access technology of the registered network          */
    os_uint32_t allowed_nssai_len;                   /* The length of allowed_nssai */
    char       *allowed_nssai;                       /* <S-NSSAI> in 3GPP 27.007 subclause 10.1.1 */
} t5g_reg_info_t;

/**
 ***********************************************************************************************************************
 * @struct      radio_info_t
 *
 * @brief       struct that holds molink module mo_get_radio's returning information
 ***********************************************************************************************************************
 */
typedef struct radio_info
{
    char cell_id[REG_CELL_ID_MAX_STR_LEN];

    os_int32_t ecl;
    os_int32_t snr;
    os_int32_t earfcn;
    os_int32_t signal_power;
    os_int32_t rsrp;
    os_int32_t rsrq;
    os_int32_t pci;
} radio_info_t;

/**
 ***********************************************************************************************************************
 * @struct      cell_info_t
 *
 * @brief       struct that holds molink module cell information, belongs to onepos_cell_info_t
 ***********************************************************************************************************************
 */
typedef struct{
    os_uint32_t mnc;
    os_uint32_t mcc;
    os_uint32_t lac;
    os_uint32_t cid;
    os_int32_t  ss;
    // os_uint32_t ta;
} cell_info_t;

/**
 ***********************************************************************************************************************
 * @struct      onepos_cell_info_t
 *
 * @brief       struct that holds molink module mo_get_cell_info return information
 ***********************************************************************************************************************
 */
typedef struct{
    os_uint32_t  cell_num;
    os_uint8_t   net_type;
    cell_info_t *cell_info;
} onepos_cell_info_t;


/**
 ***********************************************************************************************************************
 * @struct      mo_psm_mode_t
 *
 * @brief       molink set_psm/get_psm op's psm_mode type
 ***********************************************************************************************************************
 */
typedef enum mo_psm_mode
{
    MO_PSM_DISABLE = 0,                             /* Disable the use of PSM. */
    MO_PSM_ENABLE,                                  /* Enable  the use of PSM. */
    MO_PSM_DISABLE_RESET,                           /* Disable the use of PSM &
                                                       reset to the manufacturer specific default values. */
} mo_psm_mode_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_psm_info_t
 *
 * @brief       molink set_psm & get_psm's info struct
 ***********************************************************************************************************************
 */
typedef struct mo_psm_info
{
    mo_psm_mode_t psm_mode;                         /* See mo_psm_mode_t */
    char periodic_rau[PSM_TIMER_MAX_STR_LEN];       /* Reserved */
    char gprs_ready_timer[PSM_TIMER_MAX_STR_LEN];   /* Reserved */
    char periodic_tau[PSM_TIMER_MAX_STR_LEN];       /* See GPRS Timer (3GPP TS 24.008) */
    char active_time[PSM_TIMER_MAX_STR_LEN];        /* See GPRS Timer (3GPP TS 24.008) */
} mo_psm_info_t;

/**
 ***********************************************************************************************************************
 * @struct      edrx_mode_t
 *
 * @brief       enum type that indicate eDRX mode
 ***********************************************************************************************************************
 */
typedef enum edrx_mode
{
    MO_EDRX_DISABLE = 0,                            /* Disable the use of eDRX */
    MO_EDRX_ENABLE,                                 /* Enable the use of eDRX */
    MO_EDRX_ENABLE_WITH_URC,                        /* Enable the use of eDRX and enable the unsolicited result code */
    MO_EDRX_RESET,                                  /* Disable the use of eDRX and discard all parameters for eDRX or,
                                                       if available,reset to the manufacturer specific default values */
} edrx_mode_t;

/**
 ***********************************************************************************************************************
 * @struct      edrx_act_type_t
 *
 * @brief       enum type that indicate eDRX act type
 ***********************************************************************************************************************
 */
typedef enum edrx_act_type
{
    MO_EDRX_NOT_USING = 0,                          /* Access technology is not using eDRX. Only use in URC. */
    MO_EDRX_EC_GSM_IOT,                             /* EC-GSM-IoT (A/Gb mode) */
    MO_EDRX_GSM,                                    /* GSM (A/Gb mode) */
    MO_EDRX_UTRAN,                                  /* UTRAN (Iu mode) */
    MO_EDRX_E_UTRAN_WB_S1,                          /* E-UTRAN (WB-S1 mode) */
    MO_EDRX_E_UTRAN_NB_S1,                          /* E-UTRAN (NB-S1 mode) */
} edrx_act_type_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_edrx_cfg_t
 *
 * @brief       molink Read eDRX Dynamic Parameters with this struct
 ***********************************************************************************************************************
 */
typedef struct mo_edrx
{
    edrx_act_type_t act_type;                       /* Access technology type */
    char req_edrx_value[DRX_EXTEND_MAX_STR_LEN];    /* Requested eDRX value, sub-clause 10.5.5.32 of 3GPP TS 24.008   */
    char nw_edrx_value[DRX_EXTEND_MAX_STR_LEN];     /* NW provided eDRX value, sub-clause 10.5.5.32 of 3GPP TS 24.008 */
    char paging_time_window[DRX_EXTEND_MAX_STR_LEN];/* Paging time window, see sub-clause 10.5.5.32 of 3GPP TS 24.008 */
} mo_edrx_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_edrx_cfg_t
 *
 * @brief       molink eDRX configuration struct
 ***********************************************************************************************************************
 */
typedef struct mo_edrx_cfg
{
    edrx_mode_t mode;                               /* mode that the use of eDRX in the UE */
    mo_edrx_t  edrx;
} mo_edrx_cfg_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_earfcn_t
 *
 * @brief       molink EARFCN (frequency lock) configuration struct
 ***********************************************************************************************************************
 */
typedef struct mo_earfcn
{
    os_uint8_t  mode;                               /* Search(lock) mode */
    os_uint32_t earfcn;                             /* On witch EARFCN to lock; range:Quectel[1-65535] OneMo[0-262143] */
    os_uint16_t pci;                                /* Physical cell ID; range[0-503]/[0x00-0x1F7] */
    os_uint8_t  earfcn_offset;                      /* Requested EARFCN offset; Only valid on OneMo */
} mo_earfcn_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_netserv_ops_t
 *
 * @brief       molink module network service ops table
 ***********************************************************************************************************************
 */
typedef struct mo_netserv_ops
{
    os_err_t (*set_attach)(mo_object_t *self, os_uint8_t attach_stat);
    os_err_t (*get_attach)(mo_object_t *self, os_uint8_t *attach_stat);
    os_err_t (*set_reg)(mo_object_t *self, os_uint8_t reg_n);
    os_err_t (*get_reg)(mo_object_t *self, eps_reg_info_t *info);
    os_err_t (*get_5g_reg)(mo_object_t *self, t5g_reg_info_t *info);
    os_err_t (*set_cgact)(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat);
    os_err_t (*get_cgact)(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat);
    os_err_t (*get_csq)(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber);
    os_err_t (*get_radio)(mo_object_t *self, radio_info_t *radio_info);
    os_err_t (*get_cell_info)(mo_object_t *self, onepos_cell_info_t *onepos_cell_info);
    os_err_t (*set_psm)(mo_object_t *self, mo_psm_info_t info);
    os_err_t (*get_psm)(mo_object_t *self, mo_psm_info_t *info);
    os_err_t (*set_edrx_cfg)(mo_object_t *self, mo_edrx_cfg_t cfg);
    os_err_t (*get_edrx_cfg)(mo_object_t *self, mo_edrx_t *edrx_local);
    os_err_t (*get_edrx_dynamic)(mo_object_t *self, mo_edrx_t *edrx_dynamic);
    os_err_t (*set_band)(mo_object_t *self, char band_list[], os_uint8_t num);
    os_err_t (*set_earfcn)(mo_object_t *self, mo_earfcn_t earfcn);
    os_err_t (*get_earfcn)(mo_object_t *self, mo_earfcn_t *earfcn);
    os_err_t (*clear_stored_earfcn)(mo_object_t *self);
    os_err_t (*clear_plmn)(mo_object_t *self);

} mo_netserv_ops_t;

os_err_t mo_set_attach(mo_object_t *self, os_uint8_t attach_stat);
os_err_t mo_get_attach(mo_object_t *self, os_uint8_t *attach_stat);
os_err_t mo_set_reg(mo_object_t *self, os_uint8_t reg_n);
os_err_t mo_get_reg(mo_object_t *self, eps_reg_info_t *info);
os_err_t mo_get_5g_reg(mo_object_t *self, t5g_reg_info_t *info);
os_err_t mo_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat);
os_err_t mo_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat);
os_err_t mo_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber);
os_err_t mo_get_radio(mo_object_t *self, radio_info_t *radio_info);
os_err_t mo_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info);
os_err_t mo_set_psm(mo_object_t *self, mo_psm_info_t info);
os_err_t mo_get_psm(mo_object_t *self, mo_psm_info_t *info);
os_err_t mo_set_edrx_cfg(mo_object_t *self, mo_edrx_cfg_t cfg);
os_err_t mo_get_edrx_cfg(mo_object_t *self, mo_edrx_t *edrx_local);
os_err_t mo_get_edrx_dynamic(mo_object_t *self, mo_edrx_t *edrx_dynamic);
os_err_t mo_set_band(mo_object_t *self, char band_list[], os_uint8_t num);
os_err_t mo_set_earfcn(mo_object_t *self, mo_earfcn_t earfcn);
os_err_t mo_get_earfcn(mo_object_t *self, mo_earfcn_t *earfcn);
os_err_t mo_clear_stored_earfcn(mo_object_t *self);
os_err_t mo_clear_plmn(mo_object_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_NETSERV_OPS */

#endif /* __MO_NETSERV_H__ */

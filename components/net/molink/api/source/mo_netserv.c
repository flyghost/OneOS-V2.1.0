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
 * @file        mo_netserv.c
 *
 * @brief       module link kit netservice api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_netserv.h"

#include <stdio.h>
#include <string.h>

#define MO_LOG_TAG "molink.nerserv"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_NETSERV_OPS

static mo_netserv_ops_t *get_netserv_ops(mo_object_t *self)
{
    mo_netserv_ops_t *ops = (mo_netserv_ops_t *)self->ops_table[MODULE_OPS_NETSERV];

    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support network service operates", self->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set the state of PS attachment
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       attach_stat     Indicates the state of PS attachment
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set the state of PS attachment successfully
 * @retval          OS_ERROR        Set the state of PS attachment error
 * @retval          OS_ETIMEOUT     Set the state of PS attachment timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_attach(mo_object_t *self, os_uint8_t attach_stat)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_attach)
    {
        ERROR("Module %s does not support set attach state operate", self->name);
        return OS_ERROR;
    }

    return ops->set_attach(self, attach_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the state of PS attachment
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      attach_stat     The buffer to store the state of PS attachment
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the state of PS attachment successfully
 * @retval          OS_ERROR        Get the state of PS attachment error
 * @retval          OS_ETIMEOUT     Get the state of PS attachment timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_attach(mo_object_t *self, os_uint8_t *attach_stat)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(attach_stat != OS_NULL);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_attach)
    {
        ERROR("Module %s does not support get attach state operate", self->name);
        return OS_ERROR;
    }

    return ops->get_attach(self, attach_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set the presentation of an network registration urc data
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       reg_n           The presentation of an network registration urc data
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set the presentation of an network registration urc data successfully
 * @retval          OS_ERROR        Set the presentation of an network registration urc data error
 * @retval          OS_ETIMEOUT     Set the presentation of an network registration urc data timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_reg(mo_object_t *self, os_uint8_t reg_n)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_reg)
    {
    ERROR("Module %s does not support set register state operate", self->name);
    return OS_ERROR;
	}

	return ops->set_reg(self, reg_n);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the presentation of an network registration urc data and 
 *                  the network registration status
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      info            The struct to store the presentation of an network registration info
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get successfully
 * @retval          OS_ERROR        Get error
 * @retval          OS_ETIMEOUT     Get timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_reg(mo_object_t *self, eps_reg_info_t *info)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != info);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_reg)
    {
        ERROR("Module %s does not support get register state operate", self->name);
        return OS_ERROR;
    }

    return ops->get_reg(self, info);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the presentation of an network registration urc data and 
 *                  the 5G network registration status
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      info            The struct to store the presentation of an network registration info
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get successfully
 * @retval          OS_ERROR        Get error
 * @retval          OS_ETIMEOUT     Get timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_5g_reg(mo_object_t *self, t5g_reg_info_t *info)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != info);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_5g_reg)
    {
        ERROR("Module %s does not support get register state operate", self->name);
        return OS_ERROR;
    }

    return ops->get_5g_reg(self, info);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to activate or deactivate PDP Context
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       cid             A numeric parameter which specifies a particular PDP context
 * @param[in]       act_stat        Indicates the state of PDP context activation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Activate or deactivate PDP Context successfully
 * @retval          OS_ERROR        Activate or deactivate PDP Context error
 * @retval          OS_ETIMEOUT     Activate or deactivate PDP Context timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_cgact)
    {
        ERROR("Module %s does not support set cgact operate", self->name);
        return OS_ERROR;
	}

	return ops->set_cgact(self, cid, act_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the state of PDP context activation 
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      cid             The buffer to store the cid
 * @param[out]      act_stat        The buffer to store the state of PDP context activation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the state of PDP context activation successfully
 * @retval          OS_ERROR        Get the state of PDP context activation error
 * @retval          OS_ETIMEOUT     Get the state of PDP context activation timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != cid);
    OS_ASSERT(OS_NULL != act_stat);
    
    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_cgact)
    {
        ERROR("Module %s does not support get cgact state operate", self->name);
        return OS_ERROR;
    }

    return ops->get_cgact(self, cid, act_stat);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get the signal strength indication <rssi> and 
 *                  channel bit error rate <ber> from the ME
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      rssi            The buffer to store the signal strength
 * @param[out]      ber             The buffer to store the channel bit error rate
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the signal quality successfully
 * @retval          OS_ERROR        Get the signal quality error
 * @retval          OS_ETIMEOUT     Get the signal quality timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != rssi);
    OS_ASSERT(OS_NULL != ber);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_csq)
    {
        ERROR("Module %s does not support get signal quality operate", self->name);
        return OS_ERROR;
    }

    return ops->get_csq(self, rssi, ber);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to query UE statistics
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      radio_info      The buffer to store the UE statistics
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get the signal quality successfully
 * @retval          OS_ERROR        Get the signal quality error
 * @retval          OS_ETIMEOUT     Get the signal quality timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_radio(mo_object_t *self, radio_info_t *radio_info)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != radio_info);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_radio)
    {
        ERROR("Module %s does not support get radio infomation operate", self->name);
        return OS_ERROR;
    }

    return ops->get_radio(self, radio_info);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get cell infomation
 *
 * @param[in]       self              The descriptor of molink module instance
 * @param[out]      onepos_cell_info  The buffer to store cell infomation
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK             Get cell infomation successfully
 * @retval          OS_ETIMEOUT        Get cell infomation timeout
 * @retval          OS_ERROR           Get cell infomation error
 ***********************************************************************************************************************
 */
os_err_t mo_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != onepos_cell_info);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_cell_info)
    {
        ERROR("Module %s does not support get_cell_info operate", self->name);
        return OS_ERROR;
    }

    return ops->get_cell_info(self, onepos_cell_info);	
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to set module PSM(power saving mode) configuration
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       info            The PSM setting info
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set module PSM(power saving mode) configuration successfully
 * @retval          OS_ERROR        Set module PSM(power saving mode) configuration error
 * @retval          OS_ETIMEOUT     Set module PSM(power saving mode) configuration timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_psm(mo_object_t *self, mo_psm_info_t info)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_psm)
    {
        ERROR("Module %s does not support set PSM function.", self->name);
        return OS_ERROR;
    }

    return ops->set_psm(self, info);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get module PSM(power saving mode) info
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       info            The pointer of mo_psm_info_t to store PSM info
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module PSM(power saving mode) info successfully
 * @retval          OS_ERROR        Get module PSM(power saving mode) info error
 * @retval          OS_ETIMEOUT     Get module PSM(power saving mode) info timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_psm(mo_object_t *self, mo_psm_info_t *info)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != info);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_psm)
    {
        ERROR("Module %s does not support getting PSM info.", self->name);
        return OS_ERROR;
    }

    return ops->get_psm(self, info);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to set module eDRX's configuration
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       cfg             The eDRX setting info
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set module eDRX's configuration successfully
 * @retval          OS_ERROR        Set module eDRX's configuration error
 * @retval          OS_ETIMEOUT     Set module eDRX's configuration timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_edrx_cfg(mo_object_t *self, mo_edrx_cfg_t cfg)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_edrx_cfg)
    {
        ERROR("Module %s does not support set eDRX configuration function.", self->name);
        return OS_ERROR;
    }

    return ops->set_edrx_cfg(self, cfg);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get module eDRX's configuration
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       edrx_local      Witch struct that holds eDRX setting info
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module eDRX's configuration successfully
 * @retval          OS_ERROR        Get module eDRX's configuration error
 * @retval          OS_ETIMEOUT     Get module eDRX's configuration timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_edrx_cfg(mo_object_t *self, mo_edrx_t *edrx_local)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != edrx_local);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_edrx_cfg)
    {
        ERROR("Module %s does not support get eDRX configuration function.", self->name);
        return OS_ERROR;
    }

    return ops->get_edrx_cfg(self, edrx_local);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get module eDRX's dynamic parameters (Effective parameters)
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       edrx_dynamic    Witch struct that holds Effective parameters
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module eDRX's dynamic parameters successfully
 * @retval          OS_ERROR        Get module eDRX's dynamic parameters error
 * @retval          OS_ETIMEOUT     Get module eDRX's dynamic parameters timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_edrx_dynamic(mo_object_t *self, mo_edrx_t *edrx_dynamic)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != edrx_dynamic);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_edrx_dynamic)
    {
        ERROR("Module %s does not support get dynamic eDRX function.", self->name);
        return OS_ERROR;
    }

    return ops->get_edrx_dynamic(self, edrx_dynamic);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to set module using bands
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       band_list       String that hold chosen using bands
 * @param[in]       num             band number in band_list
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set module using bands successfully
 * @retval          OS_ERROR        Set module using bands error
 * @retval          OS_ETIMEOUT     Set module using bands timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_band(mo_object_t *self, char band_list[], os_uint8_t num)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != band_list);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_band)
    {
        ERROR("Module %s does not support set band function.", self->name);
        return OS_ERROR;
    }

    return ops->set_band(self, band_list, num);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to set module locking to specific earfcn (specific frequency lock)
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       earfcn          Struct that hold earfcn configurations
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set module locking to specific earfcn successfully
 * @retval          OS_ERROR        Set module locking to specific earfcn error
 * @retval          OS_ETIMEOUT     Set module locking to specific earfcn timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_earfcn(mo_object_t *self, mo_earfcn_t earfcn)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_earfcn)
    {
        ERROR("Module %s does not support set earfcn function.", self->name);
        return OS_ERROR;
    }

    return ops->set_earfcn(self, earfcn);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get module earfcn
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       earfcn          Struct that hold returning earfcn configurations
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module earfcn successfully
 * @retval          OS_ERROR        Get module earfcn error
 * @retval          OS_ETIMEOUT     Get module earfcn timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_earfcn(mo_object_t *self, mo_earfcn_t *earfcn)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != earfcn);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_earfcn)
    {
        ERROR("Module %s does not support get earfcn function.", self->name);
        return OS_ERROR;
    }

    return ops->get_earfcn(self, earfcn);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to clear stored earfcn
 *
 * @param[in]       self            The descriptor of molink module instance
 *
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Clear stored earfcn successfully
 * @retval          OS_ERROR        Clear stored earfcn error
 * @retval          OS_ETIMEOUT     Clear stored earfcn timeout
 ***********************************************************************************************************************
 */
os_err_t mo_clear_stored_earfcn(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->clear_stored_earfcn)
    {
        ERROR("Module %s does not support clear stored earfcn operate", self->name);
        return OS_ERROR;
    }

    return ops->clear_stored_earfcn(self);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to clear PLMN, EARFCN, PCI attachment record
 *
 * @param[in]       self            The descriptor of molink module instance
 *
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Clear PLMN successfully
 * @retval          OS_ERROR        Clear PLMN error
 * @retval          OS_ETIMEOUT     Clear PLMN timeout
 ***********************************************************************************************************************
 */
os_err_t mo_clear_plmn(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self);

    mo_netserv_ops_t *ops = get_netserv_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->clear_plmn)
    {
        ERROR("Module %s does not support clear plmn operate", self->name);
        return OS_ERROR;
    }

    return ops->clear_plmn(self);
}

#ifdef OS_USING_SHELL

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <mo_common.h>
#include <os_util.h>

os_err_t module_get_reg_sh(int argc, char* argv[])
{    
    eps_reg_info_t info;
    
    mo_object_t *mo_def_obj = OS_NULL;
    
    mo_def_obj = mo_get_default();
    if (OS_NULL == mo_def_obj)
    {
        os_kprintf("module_get_reg_sh: get def mo obj fail!\n");
        return OS_ERROR;
    }

    if (mo_get_reg(mo_def_obj, &info) == OS_EOK)
    {
        INFO("Get reg reg_n:%d, reg_status:%d!\n", info.reg_n, info.reg_stat);
    }
    
    return OS_EOK;
}
SH_CMD_EXPORT(mo_get_reg, module_get_reg_sh, "Get module network reg status");

#endif /* OS_USING_SHELL */

#endif /* MOLINK_USING_NETSERV_OPS */

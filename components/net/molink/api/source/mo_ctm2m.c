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
 * @file        mo_ctm2m.c
 *
 * @brief       module link kit ctm2m api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-21   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_ctm2m.h"

#define MO_LOG_TAG "molink.ctm2m"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifndef CTM2M_TASK_STACK_SIZE
#define CTM2M_TASK_STACK_SIZE 1024
#endif

#ifndef CTM2M_TASK_PRIORITY
#define CTM2M_TASK_PRIORITY (20)
#endif

#ifndef CTM2M_TASK_TICKS
#define CTM2M_TASK_TICKS    (5)
#endif

#ifdef MOLINK_USING_CTM2M_OPS

static mo_ctm2m_ops_t *get_ctm2m_ops(mo_object_t *self)
{
    mo_ctm2m_ops_t *ops = (mo_ctm2m_ops_t *)self->ops_table[MODULE_OPS_CTM2M];

    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support ctm2m operates", self->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           This function create an instance of a molink ctm2m
 *
 * @param[in]       module          The descriptor of molink ctm2m instance
 * @param[in]       parm            The create options . @ref ctm2m_create_parm_t
 *
 * @return          On success, return a molink ctm2m instance descriptor;
 *                  On error, OS_NULL is returned.
 ***********************************************************************************************************************
 */
ctm2m_t *ctm2m_create(mo_object_t *module, ctm2m_create_parm_t parm)
{
    OS_ASSERT(OS_NULL != module);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(module);

    if (OS_NULL == ops)
    {
        return OS_NULL;
    }

    if (OS_NULL == ops->create)
    {
        ERROR("Module %s does not support ctm2m create operate", module->name);
        return OS_NULL;
    }

    return ops->create(module, parm);
}

/**
 ***********************************************************************************************************************
 * @brief           This function disconnect to server & destroy the ctm2m instance
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_destroy(ctm2m_t *handle)
{
    OS_ASSERT(OS_NULL != handle);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->destroy)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->destroy(handle);
}

/**
 ***********************************************************************************************************************
 * @brief           This function set the ctm2m UE configuration
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 * @param[in]       cfg             The UE configuration setting of ctm2m.
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_set_ue_cfg(ctm2m_t *handle, ctm2m_ue_cfg_t cfg)
{
    OS_ASSERT(OS_NULL != handle);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_ue_cfg)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->set_ue_cfg(handle, cfg);
}

/**
 ***********************************************************************************************************************
 * @brief           This function get the ctm2m UE configurations
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 * @param[in]       cfg_info        The UE configurations of ctm2m.
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_get_ue_cfg(ctm2m_t *handle, ctm2m_ue_info_t *cfg_info)
{
    OS_ASSERT(OS_NULL != handle);
    OS_ASSERT(OS_NULL != cfg_info);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_ue_cfg)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->get_ue_cfg(handle, cfg_info);
}

/**
 ***********************************************************************************************************************
 * @brief           This function request the ctm2m connecting to the server
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_register(ctm2m_t *handle)
{
    OS_ASSERT(OS_NULL != handle);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->registering)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->registering(handle);
}

/**
 ***********************************************************************************************************************
 * @brief           This function request the ctm2m disconnecting to the server
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_deregister(ctm2m_t *handle)
{
    OS_ASSERT(OS_NULL != handle);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->deregistering)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->deregistering(handle);
}

/**
 ***********************************************************************************************************************
 * @brief           This function send message info to the server
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 * @param[in]       send            Send message info @ref ctm2m_send_t
 * @param[in]       msg_id          User buffer that hold returning sending message id
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_send(ctm2m_t *handle, ctm2m_send_t send, os_int32_t *msg_id)
{
    OS_ASSERT(OS_NULL != handle);
    OS_ASSERT(OS_NULL != msg_id);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->send)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->send(handle, send, msg_id);
}

/**
 ***********************************************************************************************************************
 * @brief           This function send responding message to the ctm2m server
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 * @param[in]       resp            Respond message @ref ctm2m_resp_t
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_resp(ctm2m_t *handle, ctm2m_resp_t resp)
{
    OS_ASSERT(OS_NULL != handle);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->resp)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->resp(handle, resp);
}

/**
 ***********************************************************************************************************************
 * @brief           This function sends command to update binding mode
 *
 * @param[in]       handle          The descriptor of molink ctm2m instance
 * @param[in]       update          Update informations @ref ctm2m_update_t
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t ctm2m_update(ctm2m_t *handle, ctm2m_update_t update)
{
    OS_ASSERT(OS_NULL != handle);

    mo_ctm2m_ops_t *ops = get_ctm2m_ops(handle->module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->update)
    {
        ERROR("Module %s does not support ctm2m %s operate", handle->module->name, __func__);
        return OS_ERROR;
    }

    return ops->update(handle, update);
}

#endif /* MOLINK_USING_CTM2M_OPS */

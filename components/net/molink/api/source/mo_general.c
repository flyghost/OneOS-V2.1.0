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
 * @file        mo_general.c
 *
 * @brief       module link kit general api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_general.h"
#include <stdlib.h>

#define MO_LOG_TAG "molink.general"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_GENERAL_OPS

static mo_general_ops_t *get_general_ops(mo_object_t *self)
{
    mo_general_ops_t *ops = (mo_general_ops_t *)self->ops_table[MODULE_OPS_GENERAL];

    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support general operates", self->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT test command
 *
 * @param[in]       self            The descriptor of molink module instance
 *
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          AT test successfully
 * @retval          OS_ERROR        AT test error
 * @retval          OS_ETIMEOUT     AT test timeout
 ***********************************************************************************************************************
 */
os_err_t mo_at_test(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->at_test)
    {
        ERROR("Module %s does not support at test operate", self->name);
        return OS_ERROR;
    }

    return ops->at_test(self);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module imei
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      value           The buffer to store imei
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get imei successfully
 * @retval          OS_ERROR        Get imei error
 * @retval          OS_ETIMEOUT     Get imei timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_imei(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != value);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_imei)
    {
        ERROR("Module %s does not support get imei operate", self->name);
        return OS_ERROR;
    }

    return ops->get_imei(self, value, len);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module imsi
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      value           The buffer to store imsi
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get imsi successfully
 * @retval          OS_ERROR        Get imsi error
 * @retval          OS_ETIMEOUT     Get imsi timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_imsi(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != value);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_imsi)
    {
        ERROR("Module %s does not support get imsi operate", self->name);
        return OS_ERROR;
    }

    return ops->get_imsi(self, value, len);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module iccid
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      value           The buffer to store iccid
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get iccid successfully
 * @retval          OS_ERROR        Get iccid error
 * @retval          OS_ETIMEOUT     Get iccid timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_iccid(mo_object_t *self, char *value, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != value);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_iccid)
    {
        ERROR("Module %s does not support get iccid operate", self->name);
        return OS_ERROR;
    }

    return ops->get_iccid(self, value, len);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to get module functionality level
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[out]      fun_lvl         The buffer to store module functionality level
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module functionality level successfully
 * @retval          OS_ERROR        Get module functionality level error
 * @retval          OS_ETIMEOUT     Get module functionality level timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != fun_lvl);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_cfun)
    {
        ERROR("Module %s does not support get level of functionality operate", self->name);
        return OS_ERROR;
    }

    return ops->get_cfun(self, fun_lvl);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute AT command to set module functionality level
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       fun_lvl         The functionality level
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Set module functionality level successfully
 * @retval          OS_ERROR        Set module functionality level error
 * @retval          OS_ETIMEOUT     Set module functionality level timeout
 ***********************************************************************************************************************
 */
os_err_t mo_set_cfun(mo_object_t *self, os_uint8_t fun_lvl)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->set_cfun)
    {
        ERROR("Module %s does not support set level of functionality operate", self->name);
        return OS_ERROR;
    }

    return ops->set_cfun(self, fun_lvl);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get module firmware app version
 *
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       value           The buffer to store app version
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module firmware app version successfully
 * @retval          OS_ERROR        Get module firmware app version error
 * @retval          OS_ETIMEOUT     Get module firmware app version timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_firmware_version)
    {
        ERROR("Module %s does not support get firmware version operate", self->name);
        return OS_ERROR;
    }

    return ops->get_firmware_version(self, version);
}

os_err_t mo_sleep_mode_set(mo_object_t *self, os_uint8_t fun_lvl)
{
    OS_ASSERT(OS_NULL != self);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->sleep_mode_set)
    {
        ERROR("Module %s does not support get firmware version operate", self->name);
        return OS_ERROR;
    }

    return ops->sleep_mode_set(self, fun_lvl);
}

void mo_get_firmware_version_free(mo_firmware_version_t *version)
{
    OS_ASSERT(OS_NULL != version);

    for (int i = 0; i < version->line_counts; i++)
    {
        if (version->ver_info[i] != OS_NULL)
        {
            os_free(version->ver_info[i]);
        }
    }

    if (version->ver_info != OS_NULL)
    {
        os_free(version->ver_info);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get module eID
 *                  Without spec for eID len, current api design for all typical cases
 * 
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       eid             The buffer to store eID
 * @param[in]       len             The buffer length
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module eID successfully
 * @retval          OS_ERROR        Get module eID error
 * @retval          OS_ETIMEOUT     Get module eID timeout
 ***********************************************************************************************************************
 */
os_err_t mo_get_eid(mo_object_t *self, char *eid, os_size_t len)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != eid);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->get_eid)
    {
        ERROR("Module %s does not support get SIM eID operate", self->name);
        return OS_ERROR;
    }

    return ops->get_eid(self, eid, len);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the AT command to get current broken-down time
 * 
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       l_tm            The user buffer to store tm struct
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module gmtime successfully
 * @retval          OS_ERROR        Get module gmtime error
 * @retval          OS_ETIMEOUT     Get module gmtime timeout
 ***********************************************************************************************************************
 */
os_err_t mo_gm_time(mo_object_t *self, struct tm *l_tm)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != l_tm);

    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->gm_time)
    {
        ERROR("Module %s does not support get local time operate", self->name);
        return OS_ERROR;
    }

    return ops->gm_time(self, l_tm);
}

/**
 ***********************************************************************************************************************
 * @brief           Execute the command to get current time
 * 
 * @param[in]       self            The descriptor of molink module instance
 * @param[in]       timep           The user buffer to store current time
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Get module current time successfully
 * @retval          OS_ERROR        Get module current time error
 * @retval          OS_ETIMEOUT     Get module current time timeout
 ***********************************************************************************************************************
 */
// #define __need_time_t
#include <string.h>
os_err_t mo_time(mo_object_t *self, time_t *timep)
{
    OS_ASSERT(OS_NULL != self);
    OS_ASSERT(OS_NULL != timep);

    struct tm l_tm  = {0};
    os_err_t result = OS_EOK;
    mo_general_ops_t *ops = get_general_ops(self);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->gm_time)
    {
        ERROR("Module %s does not support %s operate", self->name, __func__);
        return OS_ERROR;
    }

    result = ops->gm_time(self, &l_tm);
    
    if (OS_EOK == result)
    {
        time_t time = mktime(&l_tm);
        if (0 < time)
        {
            memcpy(timep, &time, sizeof(time_t));
        }
        else 
        {
            ERROR("Module %s %s maketime error", self->name, __func__);
            return OS_ERROR;
        }
    }

    return result;
}

#endif /* MOLINK_USING_GENERAL_OPS */

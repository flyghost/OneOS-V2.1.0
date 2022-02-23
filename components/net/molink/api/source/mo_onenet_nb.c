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
 * @file        mo_onenet_nb.c
 *
 * @brief       module link kit onenet nb api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_onenet_nb.h"
#include "mo_common.h"

#define MO_LOG_TAG "module.onenet_nb"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define CALL_MODULE_FUNC(FUNC_NAME, OPT_NAME)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        os_err_t error = OS_ERROR;                                                                                     \
        va_list  args  = {0};                                                                                          \
        va_start(args, format);                                                                                        \
                                                                                                                       \
        if (OS_NULL == module)                                                                                           \
        {                                                                                                              \
            module = mo_get_default();                                                                                   \
        }                                                                                                              \
        mo_onenet_ops_t *ops = module_get_onenet_ops(module);                                                            \
        if (OS_NULL == ops)                                                                                            \
        {                                                                                                              \
            return error;                                                                                              \
        }                                                                                                              \
        if (OS_NULL == ops->FUNC_NAME)                                                                                 \
        {                                                                                                              \
            ERROR("Module %s does not support %s operate", module->name, OPT_NAME);                                      \
            return OS_ERROR;                                                                                           \
        }                                                                                                              \
        error = ops->FUNC_NAME(module, timeout, resp, format, args);                                                     \
        va_end(args);                                                                                                  \
                                                                                                                       \
        return error;                                                                                                  \
    } while (0)

OS_INLINE mo_onenet_ops_t *module_get_onenet_ops(mo_object_t *module)
{
    if(OS_NULL == module)
    {
        ERROR("%s Input module is NULL.", __func__);
        return OS_NULL;
    }

    mo_onenet_ops_t *ops = (mo_onenet_ops_t *)module->ops_table[MODULE_OPS_ONENET_NB];
    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support onenet operates", module->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           Get OneNET access configurations
 *                  OneNet LwM2M basic function,set configurations(Quectel platform only)
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            A user-supplied mo_config_resp_t type pointer, receive returning messages.
 * @param[in]       format          Reserved, please set an empty sting "".
 * @param[in]       ...             Reserved.
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_get_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_get_config, "MIPLGETCONFIG");
}

/**
 ***********************************************************************************************************************
 * @brief           Set OneNET access configurations
 *                  OneNet LwM2M basic function,set configurations(Quectel platform only)
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_set_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_set_config, "MIPLSETCONFIG");
}

/**
 ***********************************************************************************************************************
 * @brief           Create an OneNET communication suite instance
 *                  OneNet LwM2M basic function,create an OneNET communication suite instance
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            A user-supplied os_uint8_t type pointer, return instance ret of create.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_create(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_create, "MIPLCREATE");
}

/**
 ***********************************************************************************************************************
 * @brief           Create an OneNET communication suite instance by createex
 *                  OneNet LwM2M basic function,create an OneNET communication suite instance(OneMo platform only)
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            A user-supplied os_uint8_t type pointer, return instance ret of create.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_createex(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_createex, "MIPLCREATEEX");
}

/**
 ***********************************************************************************************************************
 * @brief           Delete an OneNET communication suite instance
 *                  OneNet LwM2M basic function,delete an OneNET communication suite instance
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_delete(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_delete, "MIPLDELETE");
}

/**
 ***********************************************************************************************************************
 * @brief           Add OneNET LwM2M objects
 *                  OneNet LwM2M basic function,add OneNET LwM2M objects
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_addobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_addobj, "MIPLADDOBJ");
}

/**
 ***********************************************************************************************************************
 * @brief           Delete OneNET LwM2M objects
 *                  OneNet LwM2M basic function,delete OneNET LwM2M objects
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_delobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_delobj, "MIPLDELOBJ");
}

/**
 ***********************************************************************************************************************
 * @brief           Set the mode of notifying the terminal
 *                  OneNet LwM2M extend function,set the mode of notifying the terminal(OneMo platform only)
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_nmi(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_nmi, "MIPLNMI");
}

/**
 ***********************************************************************************************************************
 * @brief           Open OneNET request registration process
 *                  OneNet LwM2M basic function,open OneNET request registration process
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_open(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_open, "MIPLOPEN");
}

/**
 ***********************************************************************************************************************
 * @brief           Logout,send logout OneNET request
 *                  OneNet LwM2M basic function,send logout OneNET request
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_close(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_close, "MIPLCLOSE");
}

/**
 ***********************************************************************************************************************
 * @brief           Respond to the 'discover' request of OneNET
 *                  OneNet LwM2M basic function,respond to the discover request of OneNET
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_discoverrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_discoverrsp, "MIPLDISCOVERRSP");
}

/**
 ***********************************************************************************************************************
 * @brief           Respond to the 'observe' request of OneNET
 *                  OneNet LwM2M basic function,respond to the observe request of OneNET
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_observersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_observersp, "MIPLOBSERVERSP");
}

/**
 ***********************************************************************************************************************
 * @brief           Respond to the 'read' request of OneNET
 *                  OneNet LwM2M basic function,respond to the read request of OneNET
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_readrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_readrsp, "MIPLREADRSP");
}

/**
 ***********************************************************************************************************************
 * @brief           Respond to the 'write' request of OneNET
 *                  OneNet LwM2M basic function,respond to the write request of OneNET
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_writersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_writersp, "MIPLWRITERSP");
}

/**
 ***********************************************************************************************************************
 * @brief           Respond to the 'execute' request of OneNET
 *                  OneNet LwM2M basic function,respond to the execute request of OneNET
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_executersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_executersp, "MIPLEXECUTERSP");
}

/**
 ***********************************************************************************************************************
 * @brief           Respond to the 'Write-Attributes' request of OneNET
 *                  OneNet LwM2M basic function,respond to the Write-Attributes request of OneNET
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_parameterrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_parameterrsp, "MIPLPARAMETERRSP");
}

/**
 ***********************************************************************************************************************
 * @brief           Report data to the OneNET platform or application server
 *                  OneNet LwM2M basic function,Report data to the OneNET platform or application server
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_notify(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_notify, "MIPLNOTIFY");
}

/**
 ***********************************************************************************************************************
 * @brief           Send an update request to update the OneNET device lifecycle and object list
 *                  OneNet LwM2M basic function,Send an update request to update the device lifecycle and object list
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            Reserved.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_update(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_update, "MIPLUPDATE");
}

/**
 ***********************************************************************************************************************
 * @brief           Fetch unread 'write' operations in URC non-default mode
 *                  OneNet LwM2M extend function,Fetch unread 'write' operations in URC non-default mode(OneMo platform only)
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       timeout         The AT parser get resp timeout value, set by os ticks.
 * @param[in]       resp            A user-supplied module_mgr_resp_t type pointer, receive returning messages.
 * @param[in]       format          The parameter expression of command @see AT Commands Manual.
 * @param[in]       ...             The expression arguments..
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 * @retval          OS_EBUSY        Busy
 * @retval          OS_ETIMEOUT     Response timeout
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_get_write(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_get_write, "MIPLMGR");
}

/**
 ***********************************************************************************************************************
 * @brief           Reigster the user callbacks that is used to get the URC messages
 *                  Extend function,Reigster the user callbacks that is used to get the URC messages(Quectel platform only)
 *
 * @param[in]       module            The descriptor of molink module instance.
 * @param[in]       user_callbacks  A mo_onenet_cb_t type struct contains user callbacks.
 *
 * @return          On success, return OS_EOK; 
 *                  On error, return an error code. 
 * @retval          OS_EOK          Success
 * @retval          OS_ERROR        Failed
 ***********************************************************************************************************************
 */
os_err_t mo_onenetnb_cb_register(mo_object_t *module, mo_onenet_cb_t user_callbacks)
{
    if (OS_NULL == module)
    {
        module = mo_get_default();
    }

    /* module_get_onenet_ops will recheck module ptr */
    mo_onenet_ops_t *ops = module_get_onenet_ops(module);

    if (OS_NULL == ops || OS_NULL == ops->onenetnb_cb_register)
    {
        ERROR("Module %s does not support NBCBREGIST operate", module->name);
        return OS_ERROR;
    }

    return ops->onenetnb_cb_register(module, user_callbacks);
}

#ifdef OS_USING_SHELL

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>

os_err_t module_onenetnb_create_sh(int argc, char *argv[])
{
    os_uint8_t ref_num = 0;

    if (argc < 2)
    {
        ERROR("Usage:mo_onenetnb_create len config\n");
        return OS_EOK;
    }

    if (mo_onenetnb_create(OS_NULL, 5000, &ref_num, "%d,%s,0,%d,0", atoi(argv[1]), argv[2], atoi(argv[1])) == OS_EOK)
    {
        ERROR("nb device instance:%d\n", ref_num);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_create, module_onenetnb_create_sh, "create nb device instance");

os_err_t module_onenetnb_createex_sh(int argc, char *argv[])
{
    os_uint8_t ref_num = 0;

    if (mo_onenetnb_createex(OS_NULL, 5000, &ref_num, "\"183.230.40.39\",1") == OS_EOK)
    {
        ERROR("nb device instance:%d\n", ref_num);
    }
    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_createex, module_onenetnb_createex_sh, "createex nb device instance");

os_err_t module_onenetnb_addobj_sh(int argc, char *argv[])
{

    if (argc < 2)
    {
        ERROR("Usage:mo_onenetnb_addobj ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_addobj(OS_NULL, 2000, OS_NULL, "%d,3200,1,\"1\",0,0", atoi(argv[1])) == OS_EOK)
    {
        ERROR("add obj obj:3200, inscount:1, bitmap:1, atts:0, acts:0\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_addobj, module_onenetnb_addobj_sh, "add nb obj");

os_err_t module_onenetnb_discoverrsp_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        ERROR("Usage:mo_onenetnb_discoverrsp ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_discoverrsp(OS_NULL, 2000, OS_NULL, "%d,3200,1,14,\"5500;5501;5750\"", atoi(argv[1])) == OS_EOK)
    {
        ERROR("discoverrsp obj:3200, result:1, length:4, data:5500;5501;5750\n");
    }
    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_discoverrsp, module_onenetnb_discoverrsp_sh, "add nb resource");

os_err_t module_onenetnb_nmi_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        ERROR("Usage:mo_onenetnb_nmi ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_nmi(OS_NULL, 2000, OS_NULL, "%d,1,1", atoi(argv[1])) == OS_EOK)
    {
        ERROR("set numi nnmi:1, nsmi:1\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_nmi, module_onenetnb_nmi_sh, "set nmi");

os_err_t module_onenetnb_open_sh(int argc, char *argv[])
{
    if (argc < 3)
    {
        ERROR("Usage:mo_onenetnb_open ref_num lifetime\n");
        return OS_EOK;
    }

    if (mo_onenetnb_open(OS_NULL, 30000, OS_NULL, "%d,%d,30", atoi(argv[1]), atoi(argv[2])) == OS_EOK)
    {
        ERROR("open successed!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_open, module_onenetnb_open_sh, "reg to onenet");

os_err_t module_onenetnb_notify_sh(int argc, char *argv[])
{
    if (argc < 4)
    {
        ERROR("Usage:mo_onenetnb_notify ref_num len value\n");
        return OS_EOK;
    }

    static os_uint8_t uip = 0;
    if (mo_onenetnb_notify(OS_NULL,
                           15000,
                           OS_NULL,
                           "0,%d,3200,0,5750,1,%d,\"%s\",0,0,%d",
                           atoi(argv[1]),
                           atoi(argv[2]),
                           argv[3],
                           uip) == OS_EOK)
    {
        ++uip;
        ERROR("notify mid:0, obj:3200, insid:0, resid:5750, type:1, len: %d, value: %s, index:0, flag:0, ack_id:%d\n",
               atoi(argv[1]),
               argv[2],
               uip);
    }
    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_notify, module_onenetnb_notify_sh, "notify data");

os_err_t module_onenetnb_update_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        ERROR("Usage:mo_onenetnb_update ref_num\n");
        return OS_EOK;
    }

    if (mo_onenetnb_update(OS_NULL, 15000, OS_NULL, "%d,3600,1", atoi(argv[1])) == OS_EOK)
    {
        ERROR("update lifetime success!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_update, module_onenetnb_update_sh, "update lifetime");

os_err_t module_onenetnb_get_write_sh(int argc, char *argv[])
{
    module_mgr_resp_t mgr;
    mgr.value = malloc(50);
    if (mo_onenetnb_get_write(OS_NULL, 2000, &mgr, "0") == OS_EOK)
    {
        ERROR("result ref:%d, mid:%d, objid:%d, insid:%d, resid:%d, type:%d, len:%d, value:%s\n",
               mgr.ref,
               mgr.mid,
               mgr.objid,
               mgr.insid,
               mgr.resid,
               mgr.type,
               mgr.len,
               mgr.value);
    }
    free(mgr.value);

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_get_write, module_onenetnb_get_write_sh, "mgr");

os_err_t module_onenetnb_writersp_sh(int argc, char *argv[])
{
    if (argc < 3)
    {
        ERROR("Usage:mo_onenetnb_writersp ref_num mid\n");
        return OS_EOK;
    }

    if (mo_onenetnb_writersp(OS_NULL, 8000, OS_NULL, "%d,%d,2", atoi(argv[1]), atoi(argv[2])) == OS_EOK)
    {
        ERROR("write resp ok!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_writersp, module_onenetnb_writersp_sh, "write resp");

os_err_t mo_onenetnb_all(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, ...)
{
    CALL_MODULE_FUNC(onenetnb_all, format);
}

os_err_t module_onenetnb_all_sh(int argc, char *argv[])
{
    if (argc < 2)
    {
        ERROR("Usage:mo_onenetnb_all at_cmd config\n");
        return OS_EOK;
    }

    if (mo_onenetnb_all(OS_NULL, 15000, OS_NULL, argv[1], argv[2]) == OS_EOK)
    {
        ERROR("write resp ok!\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mo_onenetnb_all, module_onenetnb_all_sh, "for all set");

#endif /* OS_USING_SHELL */

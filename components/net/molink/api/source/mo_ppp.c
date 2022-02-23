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
 * @file        mo_ppp.c
 *
 * @brief       module ppp api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_ppp.h"
#include "mo_common.h"

#define MO_LOG_TAG "molink.ppp"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_PPP_OPS

static mo_ppp_ops_t *get_ppp_ops(mo_object_t *module)
{
    mo_ppp_ops_t *ops = (mo_ppp_ops_t *)module->ops_table[MODULE_OPS_PPP];

    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support PPP service operates", module->name);
    }

    return ops;
}

/**
 ***********************************************************************************************************************
 * @brief           module ppp init session, check status & active pdp context
 *
 * @param[in]       module          The descriptor of molink module instance
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Init successfully
 * @retval          OS_ETIMEOUT     Init timeout
 * @retval          OS_ERROR        Init error
 ***********************************************************************************************************************
 */
os_err_t mo_ppp_init(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_ppp_ops_t *ops = get_ppp_ops(module);
    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->ppp_init)
    {
        ERROR("Module %s does not support ppp init operate", module->name);
        return OS_ERROR;
    }

    return ops->ppp_init(module);

}

/**
 ***********************************************************************************************************************
 * @brief           module require into ppp mode & release device for lwip ppp
 *                  execute this func before init lwip ppp create
 *
 * @param[in]       module          The descriptor of molink module instance
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Dail successfully
 * @retval          OS_ETIMEOUT     Dail timeout
 * @retval          OS_ERROR        Dail error
 ***********************************************************************************************************************
 */
os_err_t mo_ppp_dial(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_ppp_ops_t *ops = get_ppp_ops(module);
    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->ppp_dial)
    {
        ERROR("Module %s does not support ppp dail operate", module->name);
        return OS_ERROR;
    }

    return ops->ppp_dial(module);
}

/**
 ***********************************************************************************************************************
 * @brief           module require exit ppp mode & restore device to AT parser
 *                  execute this func after ppp_close
 *
 * @param[in]       module          The descriptor of molink module instance
 * 
 * @return          On success, return OS_EOK; on error, return a error code. 
 * @retval          OS_EOK          Dail successfully
 * @retval          OS_ETIMEOUT     Dail timeout
 * @retval          OS_ERROR        Dail error
 ***********************************************************************************************************************
 */
os_err_t mo_ppp_exit(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_ppp_ops_t *ops = get_ppp_ops(module);
    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->ppp_exit)
    {
        ERROR("Module %s does not support ppp exit operate", module->name);
        return OS_ERROR;
    }

    return ops->ppp_exit(module);
}

#if PPP_DEBUG
#include <shell.h>

void ppp_init_sh(int argc, char *argv[])
{
    mo_ppp_init(mo_get_default());
}
SH_CMD_EXPORT(ppp_init, ppp_init_sh, "ppp init session");

void ppp_dial_sh(int argc, char *argv[])
{
    mo_ppp_dial(mo_get_default());
}
SH_CMD_EXPORT(ppp_dial, ppp_dial_sh, "ppp dail up");

void ppp_exit_sh(int argc, char *argv[])
{
    mo_ppp_exit(mo_get_default());
}
SH_CMD_EXPORT(ppp_exit, ppp_exit_sh, "exit data mode");
#endif /* PPP_DEBUG */

#endif /* MOLINK_USING_PPP_OPS */

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
 * @file        e7025.c
 *
 * @brief       e7025 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "e7025.h"

#include <stdlib.h>
#include <string.h>

#define DBG_EXT_TAG "e7025"
#define DBG_EXT_LVL LOG_LVL_INFO
#include <os_dbg_ext.h>

#ifdef MOLINK_USING_E7025

#define E7025_RETRY_TIMES (5)

#ifdef E7025_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = e7025_at_test,
    .get_imei  = e7025_get_imei,
    .get_imsi  = e7025_get_imsi,
    .get_iccid = e7025_get_iccid,
    .get_cfun  = e7025_get_cfun,
    .set_cfun  = e7025_set_cfun,
};
#endif /* E7025_USING_GENERAL_OPS */

#ifdef E7025_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach = e7025_set_attach,
    .get_attach = e7025_get_attach,
    .set_reg    = e7025_set_reg,
    .get_reg    = e7025_get_reg,
    .set_cgact  = e7025_set_cgact,
    .get_cgact  = e7025_get_cgact,
    .get_csq    = e7025_get_csq,
};
#endif /* E7025_USING_NETSERV_OPS */

#ifdef E7025_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping = e7025_ping,
};
#endif /* E7025_USING_PING_OPS */

#ifdef E7025_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig   = e7025_ifconfig,
    .get_ipaddr = e7025_get_ipaddr,
};
#endif /* E7025_USING_IFCONFIG_OPS */

#ifdef E7025_USING_NETCONN_OPS
extern void e7025_netconn_init(mo_e7025_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = e7025_netconn_create,
    .destroy       = e7025_netconn_destroy,
    .gethostbyname = e7025_netconn_gethostbyname,
    .connect       = e7025_netconn_connect,
    .send          = e7025_netconn_send,
    .get_info      = e7025_netconn_get_info,
};
#endif /* E7025_USING_NETCONN_OPS */

static os_err_t e7025_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, E7025_RETRY_TIMES);
    if (result != OS_EOK)
    {
        LOG_EXT_E("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_e7025_create(const char *name, void *parser_config)
{
    mo_e7025_t *module = (mo_e7025_t *)malloc(sizeof(mo_e7025_t));
    if (OS_NULL == module)
    {
        LOG_EXT_E("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }

    os_err_t result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        free(module);

        return OS_NULL;
    }

    result = e7025_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef E7025_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* E7025_USING_GENERAL_OPS */

#ifdef E7025_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* E7025_USING_NETSERV_OPS */

#ifdef E7025_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* E7025_USING_PING_OPS */

#ifdef E7025_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* E7025_USING_IFCONFIG_OPS */

#ifdef E7025_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    e7025_netconn_init(module);

    os_mutex_init(&module->netconn_lock, name, OS_IPC_FLAG_FIFO, OS_TRUE);
#endif /* E7025_USING_NETCONN_OPS */

__exit:
    if (result != OS_EOK)
    {
        if (mo_object_get_by_name(name) != OS_NULL)
        {
            mo_object_deinit(&module->parent);
        }

        free(module);
        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_e7025_destroy(mo_object_t *self)
{
    mo_e7025_t *module = os_container_of(self, mo_e7025_t, parent);

#ifdef E7025_USING_NETCONN_OPS
    os_mutex_deinit(&module->netconn_lock);
#endif /* E7025_USING_NETCONN_OPS */

    mo_object_deinit(self);

    free(module);

    return OS_EOK;
}

#ifdef E7025_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int e7025_auto_create(void)
{
    os_device_t *device = os_device_find(E7025_DEVICE_NAME);

    if (OS_NULL == device)
    {
        LOG_EXT_E("Auto create failed, Can not find E7025 interface device %s!", E7025_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = E7025_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = E7025_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = E7025_RECV_BUFF_LEN};

    mo_object_t *module = module_e7025_create(E7025_NAME, &parser_config);

    if (OS_NULL == module)
    {
        LOG_EXT_E("Auto create failed, Can not create %s module object!", E7025_NAME);
        return OS_ERROR;
    }

    LOG_EXT_I("Auto create %s module object success!", E7025_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(e7025_auto_create);

#endif /* E7025_AUTO_CREATE */

#endif /* MOLINK_USING_E7025 */

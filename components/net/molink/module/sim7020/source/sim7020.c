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
 * @file        sim7020.c
 *
 * @brief       sim7020 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "sim7020.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "sim7020"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef MOLINK_USING_SIM7020

#define SIM7020_RETRY_TIMES (5)

#ifdef SIM7020_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = sim7020_at_test,
    .get_imei  = sim7020_get_imei,
    .get_imsi  = sim7020_get_imsi,
    .get_iccid = sim7020_get_iccid,
    .get_cfun  = sim7020_get_cfun,
    .set_cfun  = sim7020_set_cfun,
};
#endif /* SIM7020_USING_GENERAL_OPS */

#ifdef SIM7020_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach = sim7020_set_attach,
    .get_attach = sim7020_get_attach,
    .set_reg    = sim7020_set_reg,
    .get_reg    = sim7020_get_reg,
    .set_cgact  = sim7020_set_cgact,
    .get_cgact  = sim7020_get_cgact,
    .get_csq    = sim7020_get_csq,
};
#endif /* SIM7020_USING_NETSERV_OPS */

#ifdef SIM7020_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping = sim7020_ping,
};
#endif /* SIM7020_USING_PING_OPS */

#ifdef SIM7020_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig   = sim7020_ifconfig,
    .get_ipaddr = sim7020_get_ipaddr,
};
#endif /* SIM7020_USING_IFCONFIG_OPS */

#ifdef SIM7020_USING_NETCONN_OPS
extern void sim7020_netconn_init(mo_sim7020_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = sim7020_netconn_create,
    .destroy       = sim7020_netconn_destroy,
    .gethostbyname = sim7020_netconn_gethostbyname,
    .connect       = sim7020_netconn_connect,
    .send          = sim7020_netconn_send,
    .get_info      = sim7020_netconn_get_info,
};
#endif /* SIM7020_USING_NETCONN_OPS */

static os_err_t sim7020_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, SIM7020_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_sim7020_create(const char *name, void *parser_config)
{
    mo_sim7020_t *module = (mo_sim7020_t *)os_malloc(sizeof(mo_sim7020_t));
    if (OS_NULL == module)
    {
        ERROR("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }

    os_err_t result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        os_free(module);
        return OS_NULL;
    }

    result = sim7020_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef SIM7020_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* SIM7020_USING_GENERAL_OPS */

#ifdef SIM7020_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* SIM7020_USING_NETSERV_OPS */

#ifdef SIM7020_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* SIM7020_USING_PING_OPS */

#ifdef SIM7020_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* SIM7020_USING_IFCONFIG_OPS */

#ifdef SIM7020_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    sim7020_netconn_init(module);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
#endif /* SIM7020_USING_NETCONN_OPS */

__exit:
    if (result != OS_EOK)
    {
        if (mo_object_get_by_name(name) != OS_NULL)
        {
            mo_object_deinit(&module->parent);
        }

        os_free(module);
        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_sim7020_destroy(mo_object_t *self)
{
    mo_sim7020_t *module = os_container_of(self, mo_sim7020_t, parent);

#ifdef SIM7020_USING_NETCONN_OPS
    os_mutex_deinit(&module->netconn_lock);
#endif /* SIM7020_USING_NETCONN_OPS */

    mo_object_deinit(self);

    os_free(module);

    return OS_EOK;
}

#ifdef SIM7020_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int sim7020_auto_create(void)
{
    os_device_t *device = os_device_find(SIM7020_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find SIM7020 interface device %s!", SIM7020_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = SIM7020_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = SIM7020_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = SIM7020_RECV_BUFF_LEN};

    mo_object_t *module = module_sim7020_create(SIM7020_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", SIM7020_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", SIM7020_NAME);
    return OS_EOK;
}

OS_CMPOENT_INIT(sim7020_auto_create, OS_INIT_SUBLEVEL_MIDDLE);
#endif /* SIM7020_AUTO_CREATE */

#endif /* MOLINK_USING_SIM7020 */

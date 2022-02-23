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
 * @file        a7670x.c
 *
 * @brief       a7670x factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "a7670x.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MO_LOG_TAG "a7670x"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef MOLINK_USING_A7670X

#define A7670X_RETRY_TIMES (5)

#ifdef A7670X_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = a7670x_at_test,
    .get_imei  = a7670x_get_imei,
    .get_imsi  = a7670x_get_imsi,
    .get_iccid = a7670x_get_iccid,
    .get_cfun  = a7670x_get_cfun,
    .set_cfun  = a7670x_set_cfun,
};
#endif /* A7670X_USING_GENERAL_OPS */

#ifdef A7670X_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach    = a7670x_set_attach,
    .get_attach    = a7670x_get_attach,
    .set_reg       = a7670x_set_reg,
    .get_reg       = a7670x_get_reg,
    .set_cgact     = a7670x_set_cgact,
    .get_cgact     = a7670x_get_cgact,
    .get_csq       = a7670x_get_csq,
    .get_cell_info = a7670x_get_cell_info,
};
#endif /* A7670X_USING_NETSERV_OPS */

#ifdef A7670X_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping = a7670x_ping,
};
#endif /* A7670X_USING_PING_OPS */

#ifdef A7670X_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig   = a7670x_ifconfig,
    .get_ipaddr = a7670x_get_ipaddr,
};
#endif /* A7670X_USING_IFCONFIG_OPS */

#ifdef A7670X_USING_NETCONN_OPS
extern void a7670x_netconn_init(mo_a7670x_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = a7670x_netconn_create,
    .destroy       = a7670x_netconn_destroy,
    .gethostbyname = a7670x_netconn_gethostbyname,
    .connect       = a7670x_netconn_connect,
    .send          = a7670x_netconn_send,
    .get_info      = a7670x_netconn_get_info,
};
#endif /* A7670X_USING_NETCONN_OPS */

static void urc_cereg_info_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    int stat   = 0;
    int access = 0;

    char area_code[AT_RESP_BUFF_SIZE_DEF] = {0};
    char cell_id  [AT_RESP_BUFF_SIZE_DEF] = {0};

    /* For ex: +CEREG: 1,"8109","0ee5e687",7\r\n, +CEREG: <stat>[,<tac>,<ci>[,<AcT>]] */
    sscanf(data, "+CEREG: %d,\"%[^\"]\",\"%[^\"]\",%d", &stat, area_code, cell_id, &access);

    WARN("The module %s access network base station has been switched. network status[%d], tac[%s], cell_id[%s], AcT[%d]",
         parser->name, stat, area_code, cell_id, access);

    return;
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+CEREG:", .suffix = "\r\n", .func = urc_cereg_info_func},
};

static os_err_t a7670x_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, A7670X_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 2 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_a7670x_create(const char *name, void *parser_config)
{
    mo_a7670x_t *module = (mo_a7670x_t *)os_malloc(sizeof(mo_a7670x_t));
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

    result = a7670x_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef A7670X_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* A7670X_USING_GENERAL_OPS */

#ifdef A7670X_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* A7670X_USING_NETSERV_OPS */

#ifdef A7670X_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* A7670X_USING_PING_OPS */

#ifdef A7670X_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* A7670X_USING_IFCONFIG_OPS */

#ifdef A7670X_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    a7670x_netconn_init(module);

    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
#endif /* A7670X_USING_NETCONN_OPS */

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

os_err_t module_a7670x_destroy(mo_object_t *self)
{
    mo_a7670x_t *module = os_container_of(self, mo_a7670x_t, parent);

#ifdef A7670X_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* A7670X_USING_NETCONN_OPS */

    mo_object_deinit(self);
    os_free(module);

    return OS_EOK;
}

#ifdef A7670X_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int a7670x_auto_create(void)
{
    os_device_t *device = os_device_find(A7670X_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find %s interface device!", A7670X_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = A7670X_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = A7670X_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = A7670X_RECV_BUFF_LEN};

    mo_object_t *module = module_a7670x_create(A7670X_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", A7670X_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", A7670X_NAME);

    return OS_EOK;
}
OS_CMPOENT_INIT(a7670x_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* A7670X_AUTO_CREATE */

#endif /* MOLINK_USING_A7670X */

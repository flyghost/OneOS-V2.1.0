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
 * @file        n58.c
 *
 * @brief       n58.c module api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-30   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "n58.h"

#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "n58"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef MOLINK_USING_N58

#define N58_RETRY_TIMES (5)

#ifdef N58_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = n58_at_test,
    .get_imei             = n58_get_imei,
    .get_imsi             = n58_get_imsi,
    .get_iccid            = n58_get_iccid,
    .get_cfun             = n58_get_cfun,
    .set_cfun             = n58_set_cfun,
    .get_firmware_version = n58_get_firmware_version,
};
#endif /* N58_USING_GENERAL_OPS */

#ifdef N58_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = n58_set_attach,
    .get_attach           = n58_get_attach,
    .set_reg              = n58_set_reg,
    .get_reg              = n58_get_reg,
    .get_csq              = n58_get_csq,
    .get_cell_info        = n58_get_cell_info,
};
#endif /* N58_USING_NETSERV_OPS */

#ifdef N58_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = n58_ping,
};
#endif /* N58_USING_PING_OPS */

#ifdef N58_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = n58_ifconfig,
    .get_ipaddr           = n58_get_ipaddr,
};
#endif /* N58_USING_IFCONFIG_OPS */

#ifdef N58_USING_NETCONN_OPS
extern void n58_netconn_init(mo_n58_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = n58_netconn_create,
    .destroy              = n58_netconn_destroy,
    .connect              = n58_netconn_connect,
    .send                 = n58_netconn_send,
    .get_info             = n58_netconn_get_info,
    /* The N58 module does not support domain name resolution */
};
#endif /* N58_USING_NETCONN_OPS */

static void urc_ready_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("N58 module initialization is successful");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+PBREADY", .suffix = "\r\n", .func = urc_ready_func},
};

static void n58_gernel_at_init(mo_object_t *self)
{
    char imsi[MO_IMSI_LEN + 1] = {0};

     os_uint8_t fun_lvl = 0;

    if (n58_get_imsi(self, imsi, sizeof(imsi)) != OS_EOK)
    {
        WARN("Get module %s imsi failed, please check the SIM card.", self->name);
        return;
    }

    if (n58_get_cfun(self, &fun_lvl) != OS_EOK)
    {
        ERROR("Get %s module level of functionality failed.", self->name);
        return;
    }

    if (0 == fun_lvl && (n58_set_cfun(self, 1) != OS_EOK))
    {
        ERROR("Set %s module level of functionality failed.", self->name);
        return;
    }

    return;
}

static void n58_netserv_open(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+COPS=0") != OS_EOK)
    {
        ERROR("The %s module set auto select network failed", self->name);
    }

    resp.timeout = 150 * OS_TICK_PER_SECOND;

    if (at_parser_exec_cmd(parser, &resp, "AT+XIIC=1") != OS_EOK)
    {
        ERROR("The %s module establishment of the PPP link failed", self->name);
    }

    return;
}

static void n58_netserv_at_init(mo_object_t *self)
{
    at_parser_t   *parser   = &self->parser;
    eps_reg_info_t reg_info = {0};

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 2 * OS_TICK_PER_SECOND};

    for (int i = 0; i < N58_RETRY_TIMES; i++)
    {

        if (n58_get_reg(self, &reg_info) != OS_EOK)
        {
            return;
        }

        if (1 == reg_info.reg_stat || 5 == reg_info.reg_stat)
        {
            INFO("Module %s network is registered", self->name);
            break;
        }

        os_task_msleep(3000);
    }

    if (at_parser_exec_cmd(parser, &resp, "AT+XIIC?") != OS_EOK)
    {
        return;
    }

    os_int32_t stat = 0;
    if (at_resp_get_data_by_kw(&resp, "+XIIC:", "+XIIC:    %d", &stat) <= 0)
    {
        WARN("Module %s check ipaddr failed", self->name);
        return;
    }

    if (0 == stat)
    {
        n58_netserv_open(self);
    }

    return;
}

static os_err_t n58_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    os_err_t result = at_parser_connect(parser, N58_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 1 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &resp, "ATE0");
    if (result != OS_EOK)
    {
        return result;
    }

    /* Try to initialize the module */
#ifdef N58_USING_GENERAL_OPS
    n58_gernel_at_init(self);
#endif /* N58_USING_GENERAL_OPS */

#ifdef N58_USING_NETSERV_OPS
    n58_netserv_at_init(self);
#endif /* N58_USING_NETSERV_OPS */

    return result;
}

static void n58_ops_table_init(mo_object_t *module)
{
#ifdef N58_USING_GENERAL_OPS
    module->ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif

#ifdef N58_USING_NETSERV_OPS
    module->ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* N58_USING_NETSERV_OPS */

#ifdef N58_USING_PING_OPS
    module->ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* N58_USING_PING_OPS */

#ifdef N58_USING_IFCONFIG_OPS
    module->ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* N58_USING_IFCONFIG_OPS */

#ifdef N58_USING_NETCONN_OPS
    module->ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
#endif /* N58_USING_NETCONN_OPS */

    return;
}

mo_object_t *module_n58_create(const char *name, void *parser_config)
{
    mo_n58_t *module = (mo_n58_t *)os_malloc(sizeof(mo_n58_t));
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

    result = n58_at_init(&(module->parent));
    if (result != OS_EOK)
    {
        goto __exit;
    }

    n58_ops_table_init(&module->parent);

#ifdef N58_USING_NETCONN_OPS
    n58_netconn_init(module);

    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

    module->curr_connect = -1;
#endif /* N58_USING_NETCONN_OPS */

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

os_err_t module_n58_destroy(mo_object_t *self)
{
    mo_n58_t *module = os_container_of(self, mo_n58_t, parent);

    mo_object_deinit(self);

#ifdef N58_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* N58_USING_NETCONN_OPS */

    self = OS_NULL;

    os_free(module);

    return OS_EOK;
}

#ifdef N58_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int n58_auto_create(void)
{
    os_device_t *device = os_device_find(N58_DEVICE_NAME);
    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find N58 interface device %s!", N58_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = N58_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = N58_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = N58_RECV_BUFF_LEN};

    mo_object_t *module = module_n58_create(N58_NAME, &parser_config);
    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", N58_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", N58_NAME);

    return OS_EOK;
}

OS_CMPOENT_INIT(n58_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* N58_AUTO_CREATE */
#endif /* MOLINK_USING_N58 */

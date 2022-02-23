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
 * @file        l610.c
 *
 * @brief       l610.c module api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "l610.h"

#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "l610"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef MOLINK_USING_L610

#define L610_RETRY_TIMES (10)

#ifdef L610_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = l610_at_test,
    .get_imei             = l610_get_imei,
    .get_imsi             = l610_get_imsi,
    .get_iccid            = l610_get_iccid,
    .get_cfun             = l610_get_cfun,
    .set_cfun             = l610_set_cfun,
    .get_firmware_version = l610_get_firmware_version,
};
#endif /* L610_USING_GENERAL_OPS */

#ifdef L610_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = l610_set_attach,
    .get_attach           = l610_get_attach,
    .set_reg              = l610_set_reg,
    .get_reg              = l610_get_reg,
    .set_cgact            = l610_set_cgact,
    .get_cgact            = l610_get_cgact,
    .get_csq              = l610_get_csq,
    .get_cell_info        = l610_get_cell_info,
};
#endif /* L610_USING_NETSERV_OPS */
 
#ifdef L610_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = l610_ping,
};
#endif /* L610_USING_PING_OPS */

#ifdef L610_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = l610_ifconfig,
    .get_ipaddr           = l610_get_ipaddr,
};
#endif /* L610_USING_IFCONFIG_OPS */

#ifdef L610_USING_NETCONN_OPS
extern void l610_netconn_init(mo_l610_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = l610_netconn_create,
    .destroy              = l610_netconn_destroy,
    .gethostbyname        = l610_netconn_gethostbyname,
    .connect              = l610_netconn_connect,
    .send                 = l610_netconn_send,
    .get_info             = l610_netconn_get_info,
};
#endif /* L610_USING_NETCONN_OPS */

static void urc_ready_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("L610 AT command initialization is successful");
}

static void urc_sim_func(struct at_parser *parser, const char *data, os_size_t size)
{
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    INFO("Module %s SIM ready", module->name);
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "AT command", .suffix = "ready\r\n", .func = urc_ready_func},
    {.prefix = "+SIM",       .suffix = "READY\r\n", .func = urc_sim_func},
};

#ifdef L610_USING_GENERAL_OPS

static void l610_gernel_at_init(mo_object_t *self)
{
    char imsi[MO_IMSI_LEN + 1] = {0};

    os_uint8_t fun_lvl = 0;

    /*  Check the SIM card reday */
    if (l610_get_imsi(self, imsi, sizeof(imsi)) != OS_EOK)
    {
        WARN("Module %s gernel at init, get imsi failed, please check the SIM card.", self->name);
        return;
    }

    if (l610_get_cfun(self, &fun_lvl) != OS_EOK)
    {
        ERROR("Module %s gernel at init, get cfun status failed", self->name);
        return;
    }

    if (0 == fun_lvl && (l610_set_cfun(self, 1) != OS_EOK))
    {
        WARN("Module %s gernel at init set cfun 1 failed, attached network failed.", self->name);
        return;
    }

    INFO("Module %s gernel at init OK", self->name);

    return;
}

#endif /* L610_USING_GENERAL_OPS */

#ifdef L610_USING_NETSERV_OPS

static void l610_netserv_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    eps_reg_info_t reg_info = {0};
    for (int i = 0; i < L610_RETRY_TIMES; i++)
    {
        if (l610_get_reg(self, &reg_info) != OS_EOK)
        {
            return;
        }

        if (1 == reg_info.reg_stat || 5 == reg_info.reg_stat)
        {
            DEBUG("Module %s network registered OK", self->name);
            break;
        }

        os_task_msleep(1000);
    }

    if (at_parser_exec_cmd(parser, &resp, "AT+MIPCALL?") != OS_EOK)
    {
        WARN("Module %s check tcp/ip protocol stack failed", self->name);
        return;
    }

    os_int32_t stat = 0;
    if (at_resp_get_data_by_kw(&resp, "+MIPCALL:", "+MIPCALL: %d", &stat) <= 0)
    {
        WARN("Module %s check tcp/ip protocol stack, resp parse failed", self->name);
        return;
    }

    if (0 == stat)
    {
        if (l610_netserv_open(self) != OS_EOK)
        {
            WARN("Module %s netserv open failed", self->name);
        }
    }

    return;
}

#endif /* L610_USING_NETSERV_OPS */

static os_err_t l610_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    os_err_t result = at_parser_connect(parser, L610_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &resp, "ATE0");
    if (result != OS_EOK)
    {
        return result;
    }

    /* Try to initialize the module network */
#ifdef L610_USING_GENERAL_OPS
    l610_gernel_at_init(self);
#endif /* L610_USING_GENERAL_OPS */

#ifdef L610_USING_NETSERV_OPS
    l610_netserv_at_init(self);
#endif /* L610_USING_NETSERV_OPS */

    return result;
}

static void l610_ops_table_init(mo_object_t *module)
{
#ifdef L610_USING_GENERAL_OPS
    module->ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif

#ifdef L610_USING_NETSERV_OPS
    module->ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* L610_USING_NETSERV_OPS */

#ifdef L610_USING_PING_OPS
    module->ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* L610_USING_PING_OPS */

#ifdef L610_USING_IFCONFIG_OPS
    module->ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* L610_USING_IFCONFIG_OPS */

#ifdef L610_USING_NETCONN_OPS
    module->ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
#endif /* L610_USING_NETCONN_OPS */

    return;
}

mo_object_t *module_l610_create(const char *name, void *parser_config)
{
    mo_l610_t *module = (mo_l610_t *)os_malloc(sizeof(mo_l610_t));
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

    result = l610_at_init(&(module->parent));
    if (result != OS_EOK)
    {
        goto __exit;
    }

    l610_ops_table_init(&module->parent);

#ifdef L610_USING_NETCONN_OPS
    l610_netconn_init(module);

    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
#endif /* L610_USING_NETCONN_OPS */

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

os_err_t module_l610_destroy(mo_object_t *self)
{
    mo_l610_t *module = os_container_of(self, mo_l610_t, parent);

    mo_object_deinit(self);

#ifdef L610_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* L610_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef L610_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int l610_auto_create(void)
{
    os_device_t *device = os_device_find(L610_DEVICE_NAME);
    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find L610 interface device %s!", L610_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = L610_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = L610_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = L610_RECV_BUFF_LEN};

    mo_object_t *module = module_l610_create(L610_NAME, &parser_config);
    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", L610_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", L610_NAME);

    return OS_EOK;
}

OS_CMPOENT_INIT(l610_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* L610_AUTO_CREATE */
#endif /* MOLINK_USING_L610 */

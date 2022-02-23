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
 * @file        n21.c
 *
 * @brief       n21.c module api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "n21.h"

#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "n21"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"


#ifdef MOLINK_USING_N21

#define N21_RETRY_TIMES (10)

#ifdef N21_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = n21_at_test,
    .get_imei             = n21_get_imei,
    .get_imsi             = n21_get_imsi,
    .get_iccid            = n21_get_iccid,
    .get_cfun             = n21_get_cfun,
    .set_cfun             = n21_set_cfun,
    .get_firmware_version = n21_get_firmware_version,
};
#endif /* N21_USING_GENERAL_OPS */

#ifdef N21_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = n21_set_attach,
    .get_attach           = n21_get_attach,
    .set_reg              = n21_set_reg,
    .get_reg              = n21_get_reg,
    .get_csq              = n21_get_csq,
};
#endif /* N21_USING_NETSERV_OPS */

#ifdef N21_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = n21_ping,
};
#endif /* N21_USING_PING_OPS */

#ifdef N21_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = n21_ifconfig,
    .get_ipaddr           = n21_get_ipaddr,
};
#endif /* N21_USING_IFCONFIG_OPS */

#ifdef N21_USING_NETCONN_OPS
extern void n21_netconn_init(mo_n21_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = n21_netconn_create,
    .destroy              = n21_netconn_destroy,
    .gethostbyname        = n21_netconn_gethostbyname,
    .connect              = n21_netconn_connect,
    .send                 = n21_netconn_send,
    .get_info             = n21_netconn_get_info,
};
#endif /* N21_USING_NETCONN_OPS */

static void urc_ready_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("N21 module initialization is successful");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+PBREADY", .suffix = "\r\n", .func = urc_ready_func},
};

static void n21_gernel_at_init(mo_object_t *self)
{
    char imsi[MO_IMSI_LEN + 1] = {0};
    if (n21_get_imsi(self, imsi, sizeof(imsi)) != OS_EOK)
    {
        WARN("Get module %s imsi failed, please check the SIM card.", self->name);
        return;
    }

    os_uint8_t fun_lvl = 0;
    if (n21_get_cfun(self, &fun_lvl) != OS_EOK)
    {
        return;
    }

    if (0 == fun_lvl && (n21_set_cfun(self, 1) != OS_EOK))
    {
        return;
    }
}

static void n21_netserv_open(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff), 
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+XIIC=1") != OS_EOK)
    {
        ERROR("The N21 module establishment of the PPP link failed");
    }
}

static void n21_netserv_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = OS_TICK_PER_SECOND};

    eps_reg_info_t reg_info = {0};

    for (int i = 0; i < N21_RETRY_TIMES; i++)
    {

        if (n21_get_reg(self, &reg_info) != OS_EOK)
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
        n21_netserv_open(self);
    }
}

static os_err_t n21_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    os_err_t result = at_parser_connect(parser, N21_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout = AT_RESP_TIMEOUT_DEF};

    result = at_parser_exec_cmd(parser, &resp, "ATE0");
    if (result != OS_EOK)
    {
        return result;
    }
    
    /* Try to initialize the module */
#ifdef N21_USING_GENERAL_OPS
    n21_gernel_at_init(self);
#endif /* N21_USING_GENERAL_OPS */

#ifdef N21_USING_NETSERV_OPS
    n21_netserv_at_init(self);
#endif /* N21_USING_NETSERV_OPS */

    return result;
}

static void n21_ops_table_init(mo_object_t *module)
{
#ifdef N21_USING_GENERAL_OPS
    module->ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif

#ifdef N21_USING_NETSERV_OPS
    module->ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* N21_USING_NETSERV_OPS */

#ifdef N21_USING_PING_OPS
    module->ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* N21_USING_PING_OPS */

#ifdef N21_USING_IFCONFIG_OPS
    module->ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* N21_USING_IFCONFIG_OPS */

#ifdef N21_USING_NETCONN_OPS
    module->ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
#endif /* N21_USING_NETCONN_OPS */
}

mo_object_t *module_n21_create(const char *name, void *parser_config)
{
    mo_n21_t *module = (mo_n21_t *)os_malloc(sizeof(mo_n21_t));
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

    result = n21_at_init(&(module->parent));
    if (result != OS_EOK)
    {
        goto __exit;
    }

    n21_ops_table_init(&module->parent);

#ifdef N21_USING_NETCONN_OPS
    n21_netconn_init(module);

    os_event_init(&module->netconn_evt, name);

    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

    module->curr_connect = -1;
#endif /* N21_USING_NETCONN_OPS */

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

os_err_t module_n21_destroy(mo_object_t *self)
{
    mo_n21_t *module = os_container_of(self, mo_n21_t, parent);

    mo_object_deinit(self);

#ifdef N21_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
#endif /* N21_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef N21_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int n21_auto_create(void)
{
    os_device_t *device = os_device_find(N21_DEVICE_NAME);
    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find N21 interface device %s!", N21_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = N21_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = N21_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = N21_RECV_BUFF_LEN};

    mo_object_t *module = module_n21_create(N21_NAME, &parser_config);
    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", N21_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", N21_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(n21_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* N21_AUTO_CREATE */
#endif /* MOLINK_USING_N21 */

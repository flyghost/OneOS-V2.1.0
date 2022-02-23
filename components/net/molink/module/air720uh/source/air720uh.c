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
 * @file        air720uh.c
 *
 * @brief       air720uh module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "air720uh.h"
#include "drv_gpio.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "air720uh"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_AIR720UH

#define AIR720UH_RETRY_TIMES (10)

#ifdef AIR720UH_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = air720uh_at_test,
    .get_imei             = air720uh_get_imei,
    .get_imsi             = air720uh_get_imsi,
    .get_iccid            = air720uh_get_iccid,
    .get_cfun             = air720uh_get_cfun,
    .set_cfun             = air720uh_set_cfun,
    .get_firmware_version = air720uh_get_firmware_version,
};
#endif /* AIR720UH_USING_GENERAL_OPS */

#ifdef AIR720UH_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = air720uh_set_attach,
    .get_attach           = air720uh_get_attach,
    .set_reg              = air720uh_set_reg,
    .get_reg              = air720uh_get_reg,
    .set_cgact            = air720uh_set_cgact,
    .get_cgact            = air720uh_get_cgact,
    .get_csq              = air720uh_get_csq,
};
#endif /* AIR720UH_USING_NETSERV_OPS */

#ifdef AIR720UH_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = air720uh_ping,
};
#endif /* AIR720UH_USING_PING_OPS */

#ifdef AIR720UH_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig            = air720uh_ifconfig,
    .get_ipaddr          = air720uh_get_ipaddr,
};
#endif /* AIR720UH_USING_IFCONFIG_OPS */

#ifdef AIR720UH_USING_NETCONN_OPS
extern void air720uh_netconn_init(mo_air720uh_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create              = air720uh_netconn_create,
    .destroy             = air720uh_netconn_destroy,
    .gethostbyname       = air720uh_netconn_gethostbyname,
    .connect             = air720uh_netconn_connect,
    .send                = air720uh_netconn_send,
    .get_info            = air720uh_netconn_get_info,
};
#endif /* AIR720UH_USING_NETCONN_OPS */

void air720uh_poweron_sequence(void)
{
    os_pin_mode(GET_PIN(A, 3), PIN_MODE_OUTPUT);
    os_pin_mode(GET_PIN(A, 4), PIN_MODE_OUTPUT);
    os_pin_write(GET_PIN(A, 4), PIN_HIGH);
    os_pin_write(GET_PIN(A, 3), PIN_HIGH);
    os_task_msleep( 500 );
    os_pin_write(GET_PIN(A, 3), PIN_LOW);

    INFO("%s Executed power on process.", __func__);
}

static void urc_ready_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("ME initialization is successful");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "RDY", .suffix = "\r\n", .func = urc_ready_func},
};

static os_err_t air720uh_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    os_err_t result = at_parser_connect(parser, AIR720UH_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout = AT_RESP_TIMEOUT_DEF};
    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_air720uh_create(const char *name, void *parser_config)
{
    mo_air720uh_t *module = (mo_air720uh_t *)os_malloc(sizeof(mo_air720uh_t));
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

    result = air720uh_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef AIR720UH_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* AIR720UH_USING_GENERAL_OPS */

#ifdef AIR720UH_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* AIR720UH_USING_NETSERV_OPS */

#ifdef AIR720UH_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* AIR720UH_USING_PING_OPS */

#ifdef AIR720UH_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* AIR720UH_USING_IFCONFIG_OPS */

#ifdef AIR720UH_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    air720uh_netconn_init(module);

    os_event_init(&module->netconn_evt, name);

    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

#endif /* AIR720UH_USING_NETCONN_OPS */

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

os_err_t module_air720uh_destroy(mo_object_t *self)
{
    mo_air720uh_t *module = os_container_of(self, mo_air720uh_t, parent);

    mo_object_deinit(self);

#ifdef AIR720UH_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
#endif /* AIR720UH_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef AIR720UH_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int air720uh_auto_create(void)
{
    os_device_t *device = os_device_find(AIR720UH_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find AIR720UH interface device %s!", AIR720UH_DEVICE_NAME);
        return OS_ERROR;
    }

	uart_config.baud_rate = AIR720UH_DEVICE_RATE;

    // air720uh_poweron_sequence();
    INFO("Auto create %s module object with [%s]:[%d]bps", AIR720UH_NAME, AIR720UH_DEVICE_NAME, AIR720UH_DEVICE_RATE);

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = AIR720UH_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = AIR720UH_RECV_BUFF_LEN};

    mo_object_t *module = module_air720uh_create(AIR720UH_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", AIR720UH_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success [%s]:[%d]bps!", AIR720UH_NAME, AIR720UH_DEVICE_NAME, AIR720UH_DEVICE_RATE);
    return OS_EOK;
}
OS_CMPOENT_INIT(air720uh_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* AIR720UH_AUTO_CREATE */

#endif /* MOLINK_USING_AIR720UH */

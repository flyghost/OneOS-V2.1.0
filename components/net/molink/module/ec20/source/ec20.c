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
 * @file        ec20.c
 *
 * @brief       ec20 module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec20.h"
#include "drv_gpio.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "ec20"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_EC20

#define EC20_RETRY_TIMES (10)

#ifdef EC20_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = ec20_at_test,
    .get_imei             = ec20_get_imei,
    .get_imsi             = ec20_get_imsi,
    .get_iccid            = ec20_get_iccid,
    .get_cfun             = ec20_get_cfun,
    .set_cfun             = ec20_set_cfun,
    .get_firmware_version = ec20_get_firmware_version,
};
#endif /* EC20_USING_GENERAL_OPS */

#ifdef EC20_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = ec20_set_attach,
    .get_attach           = ec20_get_attach,
    .set_reg              = ec20_set_reg,
    .get_reg              = ec20_get_reg,
    .set_cgact            = ec20_set_cgact,
    .get_cgact            = ec20_get_cgact,
    .get_csq              = ec20_get_csq,
};
#endif /* EC20_USING_NETSERV_OPS */

#ifdef EC20_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = ec20_ping,
};
#endif /* EC20_USING_PING_OPS */

#ifdef EC20_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig            = ec20_ifconfig,
    .get_ipaddr          = ec20_get_ipaddr,
    .set_dnsserver       = ec20_set_dnsserver,
    .get_dnsserver       = ec20_get_dnsserver,
};
#endif /* EC20_USING_IFCONFIG_OPS */

#ifdef EC20_USING_NETCONN_OPS
extern void ec20_netconn_init(mo_ec20_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create              = ec20_netconn_create,
    .destroy             = ec20_netconn_destroy,
    .gethostbyname       = ec20_netconn_gethostbyname,
    .connect             = ec20_netconn_connect,
    .send                = ec20_netconn_send,
    .get_info            = ec20_netconn_get_info,
};
#endif /* EC20_USING_NETCONN_OPS */

#ifdef EC20_USING_PPP_OPS
static const struct mo_ppp_ops gs_ppp_ops = {
    .ppp_init             = ec20_ppp_init,
    .ppp_dial             = ec20_ppp_dial,
    .ppp_exit             = ec20_ppp_exit,
};
#endif /* EC20_USING_PPP_OPS */

void ec20_poweron_sequence(void)
{
    os_pin_mode(GET_PIN(A, 3), PIN_MODE_OUTPUT);
    os_pin_mode(GET_PIN(A, 4), PIN_MODE_OUTPUT);
    os_pin_write(GET_PIN(A, 4), PIN_HIGH);
    os_pin_write(GET_PIN(A, 3), PIN_HIGH);
    os_task_msleep(500);
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

static os_err_t ec20_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    os_err_t result = at_parser_connect(parser, EC20_RETRY_TIMES);
    if (result != OS_EOK)
    {
       ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout = AT_RESP_TIMEOUT_DEF};
    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_ec20_create(const char *name, void *parser_config)
{
    mo_ec20_t *module = (mo_ec20_t *)os_calloc(1, sizeof(mo_ec20_t));
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

    result = ec20_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef EC20_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* EC20_USING_GENERAL_OPS */

#ifdef EC20_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* EC20_USING_NETSERV_OPS */

#ifdef EC20_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* EC20_USING_PING_OPS */

#ifdef EC20_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* EC20_USING_IFCONFIG_OPS */

#ifdef EC20_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    ec20_netconn_init(module);
    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
    module->curr_connect = -1;
#endif /* EC20_USING_NETCONN_OPS */

#ifdef EC20_USING_PPP_OPS
    module->parent.ops_table[MODULE_OPS_PPP] = &gs_ppp_ops;
#endif /* EC20_USING_IFCONFIG_OPS */
__exit:
    if (result != OS_EOK)
    {
#ifdef EC20_USING_NETCONN_OPS
        if (OS_NULL != module->parent.ops_table[MODULE_OPS_NETCONN])
        {
            os_event_deinit(&module->netconn_evt);
            os_mutex_deinit(&module->netconn_lock);
        }
#endif /* EC20_USING_NETCONN_OPS */

        if (mo_object_get_by_name(name) != OS_NULL)
        {
            mo_object_deinit(&module->parent);
        }

        os_free(module);

        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_ec20_destroy(mo_object_t *self)
{
    mo_ec20_t *module = os_container_of(self, mo_ec20_t, parent);

    mo_object_deinit(self);

#ifdef EC20_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* EC20_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef EC20_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int ec20_auto_create(void)
{
    os_device_t *device = os_device_find(EC20_DEVICE_NAME);

    if (OS_NULL == device)
    {
       ERROR("Auto create failed, Can not find EC20 interface device %s!", EC20_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = EC20_DEVICE_RATE;

    // ec20_poweron_sequence();
   INFO("Auto create %s module object with [%s]:[%d]bps", EC20_NAME, EC20_DEVICE_NAME, EC20_DEVICE_RATE);

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = EC20_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = EC20_RECV_BUFF_LEN};

    mo_object_t *module = module_ec20_create(EC20_NAME, &parser_config);

    if (OS_NULL == module)
    {
       ERROR("Auto create failed, Can not create %s module object!", EC20_NAME);
        return OS_ERROR;
    }

   INFO("Auto create %s module object success [%s]:[%d]bps!", EC20_NAME, EC20_DEVICE_NAME, EC20_DEVICE_RATE);
    return OS_EOK;
}
OS_CMPOENT_INIT(ec20_auto_create,OS_INIT_SUBLEVEL_MIDDLE);

#endif /* EC20_AUTO_CREATE */

#endif /* MOLINK_USING_EC20 */

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
 * @file        clm920rv3.c
 *
 * @brief       clm920rv3 module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "clm920rv3.h"
#include "drv_gpio.h"

#include <stdlib.h>
#include <string.h>

#ifdef MOLINK_USING_CLM920RV3

#define MO_LOG_TAG "clm920rv3"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define CLM920RV3_RETRY_TIMES (10)

#ifdef CLM920RV3_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = clm920rv3_at_test,
    .get_imei             = clm920rv3_get_imei,
    .get_imsi             = clm920rv3_get_imsi,
    .get_iccid            = clm920rv3_get_iccid,
    .get_cfun             = clm920rv3_get_cfun,
    .set_cfun             = clm920rv3_set_cfun,
    .get_firmware_version = clm920rv3_get_firmware_version,
};
#endif /* CLM920RV3_USING_GENERAL_OPS */

#ifdef CLM920RV3_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = clm920rv3_set_attach,
    .get_attach           = clm920rv3_get_attach,
    .set_reg              = clm920rv3_set_reg,
    .get_reg              = clm920rv3_get_reg,
    .get_cgact            = clm920rv3_get_cgact,
    .get_csq              = clm920rv3_get_csq,
};
#endif /* CLM920RV3_USING_NETSERV_OPS */

#ifdef CLM920RV3_USING_PING_OPS
/* CLM920RV3 ping function just for test use, not officially support for now. */
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = clm920rv3_ping,
};
#endif /* CLM920RV3_USING_PING_OPS */

#ifdef CLM920RV3_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig            = clm920rv3_ifconfig,
    .get_ipaddr          = clm920rv3_get_ipaddr,
    .set_dnsserver       = clm920rv3_set_dnsserver,
    .get_dnsserver       = clm920rv3_get_dnsserver,
};
#endif /* CLM920RV3_USING_IFCONFIG_OPS */

#ifdef CLM920RV3_USING_NETCONN_OPS
extern os_err_t clm920rv3_netconn_init(mo_clm920rv3_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create              = clm920rv3_netconn_create,
    .destroy             = clm920rv3_netconn_destroy,
    .gethostbyname       = clm920rv3_netconn_gethostbyname,
    .connect             = clm920rv3_netconn_connect,
    .send                = clm920rv3_netconn_send,
    .get_info            = clm920rv3_netconn_get_info,
};
#endif /* CLM920RV3_USING_NETCONN_OPS */

static void urc_info_func(struct at_parser *parser, const char *data, os_size_t size)
{
    /* handle the init process urc information */
    if (AT_RESP_BUFF_SIZE_DEF >= size)
        INFO("%s ME initialization info recv:[%s]", __func__, data);
}

static at_urc_t clm920rv3_urc_table[] = {
    {.prefix = "RDY",       .suffix = "\r\n", .func = urc_info_func},
    {.prefix = "*ATREADY:", .suffix = "\r\n", .func = urc_info_func},
    {.prefix = "+CPIN:",    .suffix = "\r\n", .func = urc_info_func},
    {.prefix = "^MODE:",    .suffix = "\r\n", .func = urc_info_func},
    {.prefix = "+CGEV:",    .suffix = "\r\n", .func = urc_info_func},
    {.prefix = "+NITZ:",    .suffix = "\r\n", .func = urc_info_func},
    {.prefix = "+CTZV:",    .suffix = "\r\n", .func = urc_info_func},
    {.prefix = "+CREG:",    .suffix = "\r\n", .func = urc_info_func},
};

static os_err_t clm920rv3_at_init(mo_object_t *self)
{
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_ERROR;
    at_resp_t      resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    /* set urc table first to handle module init-process unused urc  */
    at_parser_set_urc_table(parser, clm920rv3_urc_table, sizeof(clm920rv3_urc_table) / sizeof(at_urc_t));
    
    if (OS_EOK != at_parser_connect(parser, CLM920RV3_RETRY_TIMES))
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        goto __exit;
    }

    /* basic environment set */
    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "ATE0"))       goto __exit;
    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CREG=0"))  goto __exit;
    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+CEREG=0")) goto __exit;
    
    result = OS_EOK;
    INFO("%s init success", __func__);

__exit:
    return result;
}

mo_object_t *module_clm920rv3_create(const char *name, void *parser_config)
{
    mo_clm920rv3_t *module = (mo_clm920rv3_t *)os_calloc(1, sizeof(mo_clm920rv3_t));
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

    result = clm920rv3_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef CLM920RV3_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* CLM920RV3_USING_GENERAL_OPS */

#ifdef CLM920RV3_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* CLM920RV3_USING_NETSERV_OPS */

#ifdef CLM920RV3_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* CLM920RV3_USING_PING_OPS */

#ifdef CLM920RV3_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* CLM920RV3_USING_IFCONFIG_OPS */

#ifdef CLM920RV3_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

    result = clm920rv3_netconn_init(module);
    if (OS_EOK != result)
    {
        os_event_deinit(&module->netconn_evt);
        os_mutex_deinit(&module->netconn_lock);
    }
#endif /* CLM920RV3_USING_NETCONN_OPS */

__exit:
    if (OS_EOK != result)
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

os_err_t module_clm920rv3_destroy(mo_object_t *self)
{
    mo_clm920rv3_t *module = os_container_of(self, mo_clm920rv3_t, parent);

    mo_object_deinit(self);

#ifdef CLM920RV3_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
#endif /* CLM920RV3_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef CLM920RV3_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int clm920rv3_auto_create(void)
{
    os_device_t *device = os_device_find(CLM920RV3_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find CLM920RV3 interface device %s!", CLM920RV3_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = CLM920RV3_DEVICE_RATE;

    INFO("Auto create %s module object with [%s]:[%d]bps", CLM920RV3_NAME, CLM920RV3_DEVICE_NAME, CLM920RV3_DEVICE_RATE);

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = CLM920RV3_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = CLM920RV3_RECV_BUFF_LEN};

    mo_object_t *module = module_clm920rv3_create(CLM920RV3_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", CLM920RV3_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success [%s]:[%d]bps!", CLM920RV3_NAME, CLM920RV3_DEVICE_NAME, CLM920RV3_DEVICE_RATE);
    return OS_EOK;
}
OS_CMPOENT_INIT(clm920rv3_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* CLM920RV3_AUTO_CREATE */

#endif /* MOLINK_USING_CLM920RV3 */

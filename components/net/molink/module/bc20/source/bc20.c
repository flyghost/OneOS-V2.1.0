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
 * @file        bc20.c
 *
 * @brief       bc20 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc20.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "bc20"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_BC20

#define BC20_RETRY_TIMES (5)

#ifdef BC20_USING_GENERAL_OPS
extern void bc20_general_init(mo_bc20_t *module);

static const struct mo_general_ops gs_general_ops = {
    .at_test               = bc20_at_test,
    .get_imei              = bc20_get_imei,
    .get_imsi              = bc20_get_imsi,
    .get_iccid             = bc20_get_iccid,
    .get_cfun              = bc20_get_cfun,
    .set_cfun              = bc20_set_cfun,
};
#endif /* BC20_USING_GENERAL_OPS */

#ifdef BC20_USING_NETSERV_OPS
extern void bc20_netserv_init(mo_bc20_t *module);

static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach            = bc20_set_attach,
    .get_attach            = bc20_get_attach,
    .set_reg               = bc20_set_reg,
    .get_reg               = bc20_get_reg,
    .set_cgact             = bc20_set_cgact,
    .get_cgact             = bc20_get_cgact,
    .get_csq               = bc20_get_csq,
    .get_radio             = bc20_get_radio,
};
#endif /* BC20_USING_NETSERV_OPS */

#ifdef BC20_USING_PING_OPS
extern void bc20_ping_init(mo_bc20_t *module);
static const struct mo_ping_ops gs_ping_ops = {
    .ping                  = bc20_ping,
};
#endif /* BC20_USING_PING_OPS */

#ifdef BC20_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig              = bc20_ifconfig,
    .get_ipaddr            = bc20_get_ipaddr,
    .set_dnsserver         = bc20_set_dnsserver,
    .get_dnsserver         = bc20_get_dnsserver,
};
#endif /* BC20_USING_IFCONFIG_OPS */

#ifdef BC20_USING_NETCONN_OPS
extern void bc20_netconn_init(mo_bc20_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create                = bc20_netconn_create,
    .destroy               = bc20_netconn_destroy,
    .gethostbyname         = bc20_netconn_gethostbyname,
    .connect               = bc20_netconn_connect,
    .send                  = bc20_netconn_send,
    .get_info              = bc20_netconn_get_info,
};
#endif /* BC20_USING_NETCONN_OPS */

static os_err_t bc20_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = OS_TICK_PER_SECOND};

    result = at_parser_connect(parser, BC20_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, "ATE0");
    if (result != OS_EOK)
    {
        ERROR("%s failed set AT echo mode off", __func__);
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+QSCLK=0");
    if (result != OS_EOK)
    {
        ERROR("%s failed set sleep mode off", __func__);
        goto __exit;
    }

__exit:

    return result;
}

mo_object_t *module_bc20_create(const char *name, void *parser_config)
{
    os_err_t result = OS_ERROR;
    
    mo_bc20_t *module = (mo_bc20_t *)os_calloc(1, sizeof(mo_bc20_t));
    if (OS_NULL == module)
    {
        ERROR("Create BC20 failed, no enough memory.");
        return OS_NULL;
    }

    result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        os_free(module);
        return OS_NULL;
    }

    result = bc20_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef BC20_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* BC20_USING_GENERAL_OPS */

#ifdef BC20_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
    bc20_general_init(module);
#endif /* BC20_USING_NETSERV_OPS */

#ifdef BC20_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
    os_event_init(&module->ping_evt, "bc20_ping");
    bc20_ping_init(module);
#endif /* BC20_USING_PING_OPS */

#ifdef BC20_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* BC20_USING_IFCONFIG_OPS */

#ifdef BC20_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    bc20_netconn_init(module);
    os_event_init(&module->netconn_evt, "bc20_nc");
    os_mutex_init(&module->netconn_lock, "bc20_nc", OS_TRUE);
#endif /* BC20_USING_NETCONN_OPS */
	
__exit:
    if (OS_EOK != result)
    {
#ifdef BC20_USING_NETCONN_OPS
        if (OS_NULL != module->parent.ops_table[MODULE_OPS_NETCONN])
        {
            os_event_deinit(&module->netconn_evt);
            os_mutex_deinit(&module->netconn_lock);
        }
#endif /* BC20_USING_NETCONN_OPS */
        
#ifdef BC20_USING_PING_OPS
        if (OS_NULL != module->parent.ops_table[MODULE_OPS_PING])
        {
            os_event_deinit(&module->ping_evt);
        }
#endif /* BC20_USING_NETCONN_OPS */

        if (mo_object_get_by_name(name) != OS_NULL)
        {
            mo_object_deinit(&module->parent);
        }

        os_free(module);

        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_bc20_destroy(mo_object_t *self)
{
    mo_bc20_t *module = os_container_of(self, mo_bc20_t, parent);

#ifdef BC20_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* BC20_USING_NETCONN_OPS */

#ifdef BC20_USING_PING_OPS
    os_event_deinit(&module->ping_evt);
#endif /* BC20_USING_PING_OPS */

    mo_object_deinit(self);

    os_free(module);

    return OS_EOK;
}

#ifdef BC20_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int bc20_auto_create(void)
{
    os_device_t *device = os_device_find(BC20_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find BC20 interface device %s!", BC20_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = BC20_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = BC20_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = BC20_RECV_BUFF_LEN};

    mo_object_t *module = module_bc20_create(BC20_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", BC20_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", BC20_NAME);
    return OS_EOK;
}

OS_CMPOENT_INIT(bc20_auto_create, OS_INIT_SUBLEVEL_MIDDLE);
#endif /* BC20_AUTO_CREATE */

#endif /* MOLINK_USING_BC20 */

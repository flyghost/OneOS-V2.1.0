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
 * @file        bc95.c
 *
 * @brief       bc95 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc95.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <os_memory.h>

#define MO_LOG_TAG "bc95"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_BC95

#define BC95_RETRY_TIMES (5)

#ifdef BC95_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test               = bc95_at_test,
    .get_imei              = bc95_get_imei,
    .get_imsi              = bc95_get_imsi,
    .get_iccid             = bc95_get_iccid,
    .get_cfun              = bc95_get_cfun,
    .set_cfun              = bc95_set_cfun,
};
#endif /* BC95_USING_GENERAL_OPS */

#ifdef BC95_USING_NETSERV_OPS
extern void bc95_netserv_init(mo_bc95_t *module);

static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach            = bc95_set_attach,
    .get_attach            = bc95_get_attach,
    .set_reg               = bc95_set_reg,
    .get_reg               = bc95_get_reg,
    .set_cgact             = bc95_set_cgact,
    .get_cgact             = bc95_get_cgact,
    .get_csq               = bc95_get_csq,
    .get_radio             = bc95_get_radio,
    .set_psm               = bc95_set_psm,
    .get_psm               = bc95_get_psm,
    .set_edrx_cfg          = bc95_set_edrx_cfg,
    .get_edrx_cfg          = bc95_get_edrx_cfg,
    .get_edrx_dynamic      = bc95_get_edrx_dynamic,
};
#endif /* BC95_USING_NETSERV_OPS */

#ifdef BC95_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                  = bc95_ping,
};
#endif /* BC95_USING_PING_OPS */

#ifdef BC95_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig              = bc95_ifconfig,
    .get_ipaddr            = bc95_get_ipaddr,
    .set_dnsserver         = bc95_set_dnsserver,
    .get_dnsserver         = bc95_get_dnsserver,
};
#endif /* BC95_USING_IFCONFIG_OPS */

#ifdef BC95_USING_NETCONN_OPS
extern void bc95_netconn_init(mo_bc95_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create                = bc95_netconn_create,
    .destroy               = bc95_netconn_destroy,
    .gethostbyname         = bc95_netconn_gethostbyname,
    .connect               = bc95_netconn_connect,
    .send                  = bc95_netconn_send,
    .get_info              = bc95_netconn_get_info,
};
#endif /* BC95_USING_NETCONN_OPS */

#ifdef BC95_USING_ONENET_NB_OPS
extern os_err_t bc95_onenetnb_init(mo_bc95_t *module);
extern void     bc95_onenetnb_deinit(mo_bc95_t *module);

static const mo_onenet_ops_t gs_onenet_ops = {
    .onenetnb_get_config   = bc95_onenetnb_get_config,
    .onenetnb_set_config   = bc95_onenetnb_set_config,
    .onenetnb_create       = bc95_onenetnb_create,
    .onenetnb_delete       = bc95_onenetnb_delete,
    .onenetnb_addobj       = bc95_onenetnb_addobj,
    .onenetnb_delobj       = bc95_onenetnb_delobj,
    .onenetnb_open         = bc95_onenetnb_open,
    .onenetnb_close        = bc95_onenetnb_close,
    .onenetnb_discoverrsp  = bc95_onenetnb_discoverrsp,
    .onenetnb_observersp   = bc95_onenetnb_observersp,
    .onenetnb_readrsp      = bc95_onenetnb_readrsp,
    .onenetnb_writersp     = bc95_onenetnb_writersp,
    .onenetnb_executersp   = bc95_onenetnb_executersp,
    .onenetnb_parameterrsp = bc95_onenetnb_parameterrsp,
    .onenetnb_notify       = bc95_onenetnb_notify,
    .onenetnb_update       = bc95_onenetnb_update,
    .onenetnb_cb_register  = bc95_onenetnb_cb_register,
};
#endif /* BC95_USING_ONENET_OPS */

static os_err_t bc95_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, BC95_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
    }

#ifdef BC95_USING_IFCONFIG_OPS
    dns_server_t dns = {"114.114.114.114", "8.8.8.8"};

    bc95_set_dnsserver(self, dns);
#endif /* BC95_USING_IFCONFIG_OPS */

    return result;
}

mo_object_t *module_bc95_create(const char *name, void *parser_config)
{
    os_err_t result = OS_ERROR;
    
    mo_bc95_t *module = (mo_bc95_t *)os_calloc(1, sizeof(mo_bc95_t));
    if (OS_NULL == module)
    {
        ERROR("Create BC95 failed, no enough memory.");
        return OS_NULL;
    }

    result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        os_free(module);
        return OS_NULL;
    }

    result = bc95_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef BC95_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* BC95_USING_GENERAL_OPS */

#ifdef BC95_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
    bc95_netserv_init(module);
#endif /* BC95_USING_NETSERV_OPS */

#ifdef BC95_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* BC95_USING_PING_OPS */

#ifdef BC95_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* BC95_USING_IFCONFIG_OPS */

#ifdef BC95_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    bc95_netconn_init(module);
    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
#endif /* BC95_USING_NETCONN_OPS */

#ifdef BC95_USING_ONENET_NB_OPS
    result = bc95_onenetnb_init(module);
    if (OS_EOK != result)
    {
        goto __exit;
    }
    module->parent.ops_table[MODULE_OPS_ONENET_NB] = &gs_onenet_ops;
#endif /* BC95_USING_ONENET_NB_OPS */
	
__exit:
    if (OS_EOK != result)
    {
#ifdef BC95_USING_NETCONN_OPS
        if (OS_NULL != module->parent.ops_table[MODULE_OPS_NETCONN])
        {
            os_mutex_deinit(&module->netconn_lock);
        }
#endif /* BC95_USING_NETCONN_OPS */

#ifdef BC95_USING_ONENET_NB_OPS
        if (OS_NULL != module->regist_cb)
        {
            bc95_onenetnb_deinit(module);
        }
#endif /* BC95_USING_ONENET_NB_OPS */

        if (mo_object_get_by_name(name) != OS_NULL)
        {
            mo_object_deinit(&module->parent);
        }

        os_free(module);
        
        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_bc95_destroy(mo_object_t *self)
{
    mo_bc95_t *module = os_container_of(self, mo_bc95_t, parent);

#ifdef BC95_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* BC95_USING_NETCONN_OPS */

#ifdef BC95_USING_ONENET_NB_OPS
    bc95_onenetnb_deinit(module);
#endif /* BC95_USING_ONENET_NB_OPS */

    mo_object_deinit(self);

    os_free(module);

    return OS_EOK;
}

#ifdef BC95_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int bc95_auto_create(void)
{
    os_device_t *device = os_device_find(BC95_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find BC95 interface device %s!", BC95_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = BC95_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = BC95_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = BC95_RECV_BUFF_LEN};

    mo_object_t *module = module_bc95_create(BC95_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", BC95_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", BC95_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(bc95_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* BC95_AUTO_CREATE */

#endif /* MOLINK_USING_BC95 */

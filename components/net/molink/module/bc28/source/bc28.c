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
 * @file        bc28.c
 *
 * @brief       bc28 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc28.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <os_memory.h>

#define MO_LOG_TAG "bc28"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_BC28

#define BC28_RETRY_TIMES (5)

#ifdef BC28_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test               = bc28_at_test,
    .get_imei              = bc28_get_imei,
    .get_imsi              = bc28_get_imsi,
    .get_iccid             = bc28_get_iccid,
    .get_cfun              = bc28_get_cfun,
    .set_cfun              = bc28_set_cfun,
};
#endif /* BC28_USING_GENERAL_OPS */

#ifdef BC28_USING_NETSERV_OPS
extern void bc28_netserv_init(mo_bc28_t *module);

static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach            = bc28_set_attach,
    .get_attach            = bc28_get_attach,
    .set_reg               = bc28_set_reg,
    .get_reg               = bc28_get_reg,
    .set_cgact             = bc28_set_cgact,
    .get_cgact             = bc28_get_cgact,
    .get_csq               = bc28_get_csq,
    .get_radio             = bc28_get_radio,
    .set_psm               = bc28_set_psm,
    .get_psm               = bc28_get_psm,
    .set_edrx_cfg          = bc28_set_edrx_cfg,
    .get_edrx_cfg          = bc28_get_edrx_cfg,
    .get_edrx_dynamic      = bc28_get_edrx_dynamic,
};
#endif /* BC28_USING_NETSERV_OPS */

#ifdef BC28_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                  = bc28_ping,
};
#endif /* BC28_USING_PING_OPS */

#ifdef BC28_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig              = bc28_ifconfig,
    .get_ipaddr            = bc28_get_ipaddr,
    .set_dnsserver         = bc28_set_dnsserver,
    .get_dnsserver         = bc28_get_dnsserver,
};
#endif /* BC28_USING_IFCONFIG_OPS */

#ifdef BC28_USING_NETCONN_OPS
extern void bc28_netconn_init(mo_bc28_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create                = bc28_netconn_create,
    .destroy               = bc28_netconn_destroy,
    .gethostbyname         = bc28_netconn_gethostbyname,
    .connect               = bc28_netconn_connect,
    .send                  = bc28_netconn_send,
    .get_info              = bc28_netconn_get_info,
};
#endif /* BC28_USING_NETCONN_OPS */

#ifdef BC28_USING_ONENET_NB_OPS
extern os_err_t bc28_onenetnb_init(mo_bc28_t *module);
extern void     bc28_onenetnb_deinit(mo_bc28_t *module);

static const mo_onenet_ops_t gs_onenet_ops = {
    .onenetnb_get_config   = bc28_onenetnb_get_config,
    .onenetnb_set_config   = bc28_onenetnb_set_config,
    .onenetnb_create       = bc28_onenetnb_create,
    .onenetnb_delete       = bc28_onenetnb_delete,
    .onenetnb_addobj       = bc28_onenetnb_addobj,
    .onenetnb_delobj       = bc28_onenetnb_delobj,
    .onenetnb_open         = bc28_onenetnb_open,
    .onenetnb_close        = bc28_onenetnb_close,
    .onenetnb_discoverrsp  = bc28_onenetnb_discoverrsp,
    .onenetnb_observersp   = bc28_onenetnb_observersp,
    .onenetnb_readrsp      = bc28_onenetnb_readrsp,
    .onenetnb_writersp     = bc28_onenetnb_writersp,
    .onenetnb_executersp   = bc28_onenetnb_executersp,
    .onenetnb_parameterrsp = bc28_onenetnb_parameterrsp,
    .onenetnb_notify       = bc28_onenetnb_notify,
    .onenetnb_update       = bc28_onenetnb_update,
    .onenetnb_cb_register  = bc28_onenetnb_cb_register,
};
#endif /* BC28_USING_ONENET_OPS */

static os_err_t bc28_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, BC28_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
    }

#ifdef BC28_USING_IFCONFIG_OPS
    dns_server_t dns = {"114.114.114.114", "8.8.8.8"};

    bc28_set_dnsserver(self, dns);
#endif /* BC28_USING_IFCONFIG_OPS */

    return result;
}

mo_object_t *module_bc28_create(const char *name, void *parser_config)
{
    os_err_t result = OS_ERROR;
    
    mo_bc28_t *module = (mo_bc28_t *)os_calloc(1, sizeof(mo_bc28_t));
    if (OS_NULL == module)
    {
        ERROR("Create BC28 failed, no enough memory.");
        return OS_NULL;
    }

    result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        os_free(module);
        return OS_NULL;
    }

    result = bc28_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef BC28_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* BC28_USING_GENERAL_OPS */

#ifdef BC28_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
    bc28_netserv_init(module);
#endif /* BC28_USING_NETSERV_OPS */

#ifdef BC28_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* BC28_USING_PING_OPS */

#ifdef BC28_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* BC28_USING_IFCONFIG_OPS */

#ifdef BC28_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    bc28_netconn_init(module);
    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
#endif /* BC28_USING_NETCONN_OPS */

#ifdef BC28_USING_ONENET_NB_OPS
    result = bc28_onenetnb_init(module);
    if (OS_EOK != result)
    {
        goto __exit;
    }
    module->parent.ops_table[MODULE_OPS_ONENET_NB] = &gs_onenet_ops;
#endif /* BC28_USING_ONENET_NB_OPS */
	
__exit:
    if (OS_EOK != result)
    {
#ifdef BC28_USING_NETCONN_OPS
        if (OS_NULL != module->parent.ops_table[MODULE_OPS_NETCONN])
        {
            os_mutex_deinit(&module->netconn_lock);
        }
#endif /* BC28_USING_NETCONN_OPS */

#ifdef BC28_USING_ONENET_NB_OPS
        if (OS_NULL != module->regist_cb)
        {
            bc28_onenetnb_deinit(module);
        }
#endif /* BC28_USING_ONENET_NB_OPS */

        if (mo_object_get_by_name(name) != OS_NULL)
        {
            mo_object_deinit(&module->parent);
        }

        os_free(module);
        
        return OS_NULL;
    }

    return &(module->parent);
}

os_err_t module_bc28_destroy(mo_object_t *self)
{
    mo_bc28_t *module = os_container_of(self, mo_bc28_t, parent);

#ifdef BC28_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* BC28_USING_NETCONN_OPS */

#ifdef BC28_USING_ONENET_NB_OPS
    bc28_onenetnb_deinit(module);
#endif /* BC28_USING_ONENET_NB_OPS */

    mo_object_deinit(self);

    os_free(module);

    return OS_EOK;
}

#ifdef BC28_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int bc28_auto_create(void)
{
    os_device_t *device = os_device_find(BC28_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find BC28 interface device %s!", BC28_DEVICE_NAME);
        return OS_ERROR;
    }
	
	uart_config.baud_rate = BC28_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = BC28_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = BC28_RECV_BUFF_LEN};

    mo_object_t *module = module_bc28_create(BC28_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", BC28_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", BC28_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(bc28_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* BC28_AUTO_CREATE */

#endif /* MOLINK_USING_BC28 */

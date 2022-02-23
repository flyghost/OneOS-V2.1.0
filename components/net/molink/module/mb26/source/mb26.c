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
 * @file        mb26.c
 *
 * @brief       mb26 factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mb26.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "mb26"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#ifdef MOLINK_USING_MB26

#define MB26_RETRY_TIMES (5)

#ifdef MB26_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = mb26_at_test,
    .get_imei             = mb26_get_imei,
    .get_imsi             = mb26_get_imsi,
    .get_iccid            = mb26_get_iccid,
    .get_cfun             = mb26_get_cfun,
    .set_cfun             = mb26_set_cfun,
    .get_firmware_version = mb26_get_firmware_version,
};
#endif /* MB26_USING_GENERAL_OPS */

#ifdef MB26_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = mb26_set_attach,
    .get_attach           = mb26_get_attach,
    .set_reg              = mb26_set_reg,
    .get_reg              = mb26_get_reg,
    .set_cgact            = mb26_set_cgact,
    .get_cgact            = mb26_get_cgact,
    .get_csq              = mb26_get_csq,
    .get_radio            = mb26_get_radio,
    .set_psm              = mb26_set_psm,
    .get_psm              = mb26_get_psm,
};
#endif /* MB26_USING_NETSERV_OPS */

#ifdef MB26_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = mb26_ping,
};
#endif /* MB26_USING_PING_OPS */

#ifdef MB26_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = mb26_ifconfig,
    .get_ipaddr           = mb26_get_ipaddr,
};
#endif /* MB26_USING_IFCONFIG_OPS */

#ifdef MB26_USING_NETCONN_OPS
extern void mb26_netconn_init(mo_mb26_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = mb26_netconn_create,
    .destroy              = mb26_netconn_destroy,
    .gethostbyname        = mb26_netconn_gethostbyname,
    .connect              = mb26_netconn_connect,
    .send                 = mb26_netconn_send,
    .get_info             = mb26_netconn_get_info,
};
#endif /* MB26_USING_NETCONN_OPS */

#ifdef MB26_USING_CTM2M_OPS
static const struct mo_ctm2m_ops gs_ctm2m_ops = {
    .create               = mb26_ctm2m_create,
    .destroy              = mb26_ctm2m_destroy,
    .set_ue_cfg           = mb26_ctm2m_set_ue_cfg,
    .get_ue_cfg           = mb26_ctm2m_get_ue_cfg,
    .registering          = mb26_ctm2m_register,
    .deregistering        = mb26_ctm2m_deregister,
    .send                 = mb26_ctm2m_send,
    .resp                 = mb26_ctm2m_resp,
    .update               = mb26_ctm2m_update,
};
#endif /* MB26_USING_CTM2M_OPS */

static os_err_t mb26_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, MB26_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 3 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &resp, "ATE0");
    if (result != OS_EOK)
    {
        ERROR("%s module failed to close echo", self->name);
        return OS_ERROR;
    }

    return result;
}

mo_object_t *module_mb26_create(const char *name, void *parser_config)
{
    mo_mb26_t *module = (mo_mb26_t *)os_calloc(1, sizeof(mo_mb26_t));
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

    result = mb26_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef MB26_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* MB26_USING_GENERAL_OPS */

#ifdef MB26_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* MB26_USING_NETSERV_OPS */

#ifdef MB26_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* MB26_USING_PING_OPS */

#ifdef MB26_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* MB26_USING_IFCONFIG_OPS */

#ifdef MB26_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    mb26_netconn_init(module);

    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
#endif /* MB26_USING_NETCONN_OPS */

#ifdef MB26_USING_CTM2M_OPS
    module->parent.ops_table[MODULE_OPS_CTM2M] = &gs_ctm2m_ops;
#endif /* MB26_USING_CTM2M_OPS */

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

    os_kprintf("%s module instance created successfully\r\n", name);

    return &(module->parent);
}

os_err_t module_mb26_destroy(mo_object_t *self)
{
    mo_mb26_t *module = os_container_of(self, mo_mb26_t, parent);

#ifdef MB26_USING_NETCONN_OPS
    os_mutex_deinit(&module->netconn_lock);
#endif /* MB26_USING_NETCONN_OPS */

    mo_object_deinit(self);

    os_free(module);

    return OS_EOK;
}

#ifdef MB26_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int mb26_auto_create(void)
{
    os_device_t *device = os_device_find(MB26_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find MB26 interface device %s!", MB26_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = MB26_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = MB26_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = MB26_RECV_BUFF_LEN};

    mo_object_t *module = module_mb26_create(MB26_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", MB26_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", MB26_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(mb26_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* MB26_AUTO_CREATE */

#endif /* MOLINK_USING_MB26 */

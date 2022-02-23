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
 * @file        ml302.c
 *
 * @brief       ml302.c module api
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ml302.h"
#include <stdlib.h>
#include <os_task.h>

#define MO_LOG_TAG "ml302"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_ML302

#define ML302_RETRY_TIMES (5)

#ifdef ML302_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = ml302_at_test,
    .get_imei             = ml302_get_imei,
    .get_imsi             = ml302_get_imsi,
    .get_iccid            = ml302_get_iccid,
    .get_cfun             = ml302_get_cfun,
    .set_cfun             = ml302_set_cfun,
    .get_firmware_version = ml302_get_firmware_version,
};
#endif /* ML302_USING_GENERAL_OPS */

#ifdef ML302_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach           = ml302_set_attach,
    .get_attach           = ml302_get_attach,
    .set_reg              = ml302_set_reg,
    .get_reg              = ml302_get_reg,
    .set_cgact            = ml302_set_cgact,
    .get_cgact            = ml302_get_cgact,
    .get_csq              = ml302_get_csq,
    .get_cell_info        = ml302_get_cell_info,
};
#endif /* ML302_USING_NETSERV_OPS */

#ifdef ML302_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = ml302_ping,
};
#endif /* ML302_USING_PING_OPS */

#ifdef ML302_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = ml302_ifconfig,
    .get_ipaddr           = ml302_get_ipaddr,
};
#endif /* ML302_USING_IFCONFIG_OPS */

#ifdef ML302_USING_NETCONN_OPS
extern void ml302_netconn_init(mo_ml302_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = ml302_netconn_create,
    .destroy              = ml302_netconn_destroy,
    .gethostbyname        = ml302_netconn_gethostbyname,
    .connect              = ml302_netconn_connect,
    .send                 = ml302_netconn_send,
    .get_info             = ml302_netconn_get_info,
};
#endif /* ML302_USING_NETCONN_OPS */

#ifdef ML302_USING_MQTTC_OPS
extern void ml302_mqttc_init(mo_ml302_t *module);

static const struct mo_mqttc_ops gs_mqttc_ops = {
    .create               = ml302_mqttc_create,
    .connect              = ml302_mqttc_connect,
    .publish              = ml302_mqttc_publish,
    .subscribe            = ml302_mqttc_subscribe,
    .unsubscribe          = ml302_mqttc_unsubscribe,
    .disconnect           = ml302_mqttc_disconnect,
    .destroy              = ml302_mqttc_destroy,
};
#endif /* ML302_USING_MQTTC_OPS */

#ifdef ML302_USING_GENERAL_OPS

static void ml302_gernel_at_init(mo_object_t *self)
{
    char imsi[MO_IMSI_LEN + 1] = {0};
    if (ml302_get_imsi(self, imsi, sizeof(imsi)) != OS_EOK)
    {
        WARN("Get module %s imsi failed, please check the SIM card.", self->name);
        return;
    }

    os_uint8_t fun_lvl = 0;
    if (ml302_get_cfun(self, &fun_lvl) != OS_EOK)
    {
        return;
    }

    if (0 == fun_lvl && (ml302_set_cfun(self, 1) != OS_EOK))
    {
        ERROR("Set %s module level of functionality failed.", self->name);
        return;
    }
}

#endif /* ML302_USING_GENERAL_OPS */

#ifdef ML302_USING_NETSERV_OPS

static void ml302_netserv_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff, 
                      .buff_size = sizeof(resp_buff), 
                      .timeout   = 5 * OS_TICK_PER_SECOND};

    os_int32_t verctrl_data1 = -1;
    os_int32_t verctrl_data2 = -1;

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+VERCTRL?");
    if (result != OS_EOK)
    {
        return;
    }

    if (at_resp_get_data_by_kw(&resp, "+VERCTRL:", "+VERCTRL: %d, %d", &verctrl_data1, &verctrl_data2) < 0)
    {
        WARN("Get module %s verctrl data failed!", self->name);
        return;
    }

    if (0 == verctrl_data2)
    {
        result = ml302_set_attach(self, 1);
        if (result != OS_EOK)
        {
            WARN("Set module %s attach failed!", self->name);
            return;
        }

        result = ml302_set_cgact(self, 1, 1);
        if (result != OS_EOK)
        {
            WARN("Set module %s PDP context activate failed!", self->name);
        }
    }

    eps_reg_info_t reg_info = {0};
    result = ml302_get_reg(self, &reg_info);
    if (result != OS_EOK)
    {
        return;
    }

    if (1 != reg_info.reg_stat && 5 != reg_info.reg_stat)
    {
        WARN("Module %s network is not registered", self->name);
    }
    else
    {
        INFO("Module %s network is registered", self->name);
    }
}

#endif /* ML302_USING_NETSERV_OPS */

static os_err_t ml302_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, ML302_RETRY_TIMES);
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
#ifdef ML302_USING_GENERAL_OPS
    ml302_gernel_at_init(self);
#endif /* ML302_USING_GENERAL_OPS */

#ifdef ML302_USING_NETSERV_OPS
    ml302_netserv_at_init(self);
#endif /* ML302_USING_NETSERV_OPS */

    return result;
}

static void ml302_ops_table_init(mo_object_t *module)
{
#ifdef ML302_USING_GENERAL_OPS
    module->ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif

#ifdef ML302_USING_NETSERV_OPS
    module->ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* ML302_USING_NETSERV_OPS */

#ifdef ML302_USING_PING_OPS
    module->ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* ML302_USING_PING_OPS */

#ifdef ML302_USING_IFCONFIG_OPS
    module->ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* ML302_USING_IFCONFIG_OPS */

#ifdef ML302_USING_NETCONN_OPS
    module->ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
#endif /* ML302_USING_NETCONN_OPS */

#ifdef ML302_USING_MQTTC_OPS
    module->ops_table[MODULE_OPS_MQTTC] = &gs_mqttc_ops;
#endif /* ML302_USING_MQTTC_OPS */
}

mo_object_t *module_ml302_create(const char *name, void *parser_config)
{
    mo_ml302_t *module = (mo_ml302_t *)os_malloc(sizeof(mo_ml302_t));
    if (OS_NULL == module)
    {
        ERROR("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }
    
    os_task_msleep(5000);
    /* make sure ml302 power on and be ready */
    os_err_t result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        os_free(module);

        return OS_NULL;
    }

    result = ml302_at_init(&(module->parent));
    if (result != OS_EOK)
    {
        goto __exit;
    }

    ml302_ops_table_init(&module->parent);

#ifdef ML302_USING_NETCONN_OPS
    ml302_netconn_init(module);

    os_event_init(&module->netconn_evt, name);

    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

    module->curr_connect = -1;
#endif /* ML302_USING_NETCONN_OPS */

#ifdef ML302_USING_MQTTC_OPS
    ml302_mqttc_init(module);
#endif /* ML302_USING_MQTTC_OPS */

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

os_err_t module_ml302_destroy(mo_object_t *self)
{
    mo_ml302_t *module = os_container_of(self, mo_ml302_t, parent);

    mo_object_deinit(self);

#ifdef ML302_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
#endif /* ML302_USING_NETCONN_OPS */

#ifdef ML302_USING_MQTTC_OPS
    os_mutex_deinit(&module->mqttc_lock);
#endif /* ML302_USING_MQTTC_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef ML302_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int ml302_auto_create(void)
{
    os_device_t *device = os_device_find(ML302_DEVICE_NAME);
    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find  ML302 interface device %s!", ML302_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = ML302_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = ML302_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = ML302_RECV_BUFF_LEN};

    mo_object_t *module = module_ml302_create(ML302_NAME, &parser_config);
    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", ML302_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", ML302_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(ml302_auto_create, OS_INIT_SUBLEVEL_MIDDLE);
#endif /* ML302_AUTO_CREATE */

#endif /* MOLINK_USING_ML302 */

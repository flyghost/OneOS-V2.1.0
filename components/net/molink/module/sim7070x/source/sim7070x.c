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
 * @file        sim7070x.c
 *
 * @brief       sim7070x factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "sim7070x.h"

#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "sim7070x"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_SIM7070X

#define SIM7070X_RETRY_TIMES (5)

enum sim7070x_pdp_status
{
    PDP_DEACTIVED = 0,
    PDP_ACTIVED   = 1,
};

#ifdef SIM7070X_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = sim7070x_at_test,
    .get_imei  = sim7070x_get_imei,
    .get_imsi  = sim7070x_get_imsi,
    .get_iccid = sim7070x_get_iccid,
    .get_cfun  = sim7070x_get_cfun,
    .set_cfun  = sim7070x_set_cfun,
    .get_firmware_version = sim7070x_get_firmware_version,
};
#endif /* SIM7070X_USING_GENERAL_OPS */

#ifdef SIM7070X_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach = sim7070x_set_attach,
    .get_attach = sim7070x_get_attach,
    .set_reg    = sim7070x_set_reg,
    .get_reg    = sim7070x_get_reg,
    .set_cgact  = sim7070x_set_cgact,
    .get_cgact  = sim7070x_get_cgact,
    .get_csq    = sim7070x_get_csq,
};
#endif /* SIM7070X_USING_NETSERV_OPS */

#ifdef SIM7070X_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping = sim7070x_ping,
};
#endif /* SIM7070X_USING_PING_OPS */

#ifdef SIM7070X_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig   = sim7070x_ifconfig,
    .get_ipaddr = sim7070x_get_ipaddr,
};
#endif /* SIM7070X_USING_IFCONFIG_OPS */

#ifdef SIM7070X_USING_NETCONN_OPS
extern void sim7070x_netconn_init(mo_sim7070x_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = sim7070x_netconn_create,
    .destroy       = sim7070x_netconn_destroy,
    .gethostbyname = sim7070x_netconn_gethostbyname,
    .connect       = sim7070x_netconn_connect,
    .send          = sim7070x_netconn_send,
    .get_info      = sim7070x_netconn_get_info,
};
#endif /* SIM7070X_USING_NETCONN_OPS */

static os_err_t sim7070x_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, SIM7070X_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[32] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 2 * OS_TICK_PER_SECOND};

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

static os_err_t sim7070x_set_app_network_pdpidx0(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_uint32_t cid = 0;
    os_uint32_t stat = 0;
    os_err_t   result = OS_EOK;

    char resp_buff[256] = {0};
    char temp[64]       = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &resp, "AT+CFUN=0");
    if (result != OS_EOK)
    {
        ERROR("Module %s set cfun to 0 failed", self->name);
        return OS_ERROR;
    }

    /* Define PDP context */
    result = at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IP\",\"ctnb\"");
    if (result != OS_EOK)
    {
        ERROR("Module %s APP network PDP index0 config to ctnb failed", self->name);
        return OS_ERROR;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CFUN=1");
    if (result != OS_EOK)
    {
        ERROR("Module %s set cfun to 1 failed", self->name);
        return OS_ERROR;
    }

    os_task_msleep(5000);

        /* PDP index0 config to ctnb (APN delivered by the CAT-M or NB-IOT network) */
    result = at_parser_exec_cmd(parser, &resp, "AT+CNCFG=0,1,\"ctnb\"");
    if (result != OS_EOK)
    {
        ERROR("Module %s APP network PDP index0 config to ctnb failed", self->name);
        return OS_ERROR;
    }

    resp.line_num = 2;
    resp.timeout  = 30 * OS_TICK_PER_SECOND;
    /* APP network pdp index0 active */
    result = at_parser_exec_cmd(parser, &resp, "AT+CNACT=0,1");
    if (result != OS_EOK)
    {
        ERROR("Module %s APP network PDP index0 active failed", self->name);
        return OS_ERROR;
    }
    /* for ex: OK\r\n\r\n+APP PDP: 0,ACTIVE\r\n */
    if ((at_resp_get_data_by_kw(&resp, "+APP PDP: 0", "+APP PDP: 0,%s", temp) <= 0) && (strcmp(temp, "ACTIVE") != 0))
    {
        ERROR("Module %s APP network PDP index0 active failed, parse resp failed", self->name);
        return OS_ERROR;
    }

    resp.line_num = 0;
    resp.timeout  = 3 * OS_TICK_PER_SECOND;
    /* PDP context activate? */
    for (int i = 0; i < SIM7070X_RETRY_TIMES; i++)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CGACT?");
        if (result != OS_EOK)
        {
            return OS_ERROR;
        }

        if (at_resp_get_data_by_line(&resp, 1, "+CGACT: %u,%u", &cid, &stat) <= 0)
        {
            ERROR("Get %s module cgact state failed", self->name);
        }

        if (stat == PDP_ACTIVED)
        {
            break;
        }

        os_task_msleep(5000);
    }

    if (stat == PDP_DEACTIVED)
    {
        ERROR("Module %s PDP context is deactivate, please activate first", self->name);
        return OS_ERROR;
    }

    return result;
}

static os_err_t sim7070x_get_app_network_pdpidx0_status(mo_object_t *self, os_uint32_t *stat)
{
    at_parser_t *parser = &self->parser;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[128]                 = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 3 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CNACT?");
    if (result != OS_EOK)
    {
        ERROR("Module %s get APP network PDP index0 status failed", self->name);
        return OS_ERROR;
    }

    /* for ex: +CNACT: 0,1,"100.122.236.108"\r\n +CNACT: 1,0,"0.0.0.0"\r\n */
    if (at_resp_get_data_by_line(&resp, 1, "+CNACT: 0,%u,\"%[^\"]", stat, ipaddr) <= 0)
    {
        ERROR("Module %s get APP network PDP index0 status failed, parse resp failed ", self->name);
        return OS_ERROR;
    }

    DEBUG("Module %s APP network PDP index0 IP addr: %s", self->name, ipaddr);

    return result;
}

os_err_t module_sim7070x_app_network_pdpidx0_init(mo_object_t *self)
{
    os_uint32_t stat = 0;

    os_err_t result = sim7070x_get_app_network_pdpidx0_status(self, &stat);
    if (result != OS_EOK)
    {
        return OS_ERROR;
    }

    if (stat == PDP_DEACTIVED)
    {
        result = sim7070x_set_app_network_pdpidx0(self);
        if (result != OS_EOK)
        {
            return OS_ERROR;
        }
    }

    return result;
}

mo_object_t *module_sim7070x_create(const char *name, void *parser_config)
{
    mo_sim7070x_t *module = (mo_sim7070x_t *)os_malloc(sizeof(mo_sim7070x_t));
    if (OS_NULL == module)
    {
        ERROR("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }

    os_err_t result = mo_object_init(&module->parent, name, parser_config);
    if (result != OS_EOK)
    {
        os_free(module);

        return OS_NULL;
    }

    result = sim7070x_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = module_sim7070x_app_network_pdpidx0_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef SIM7070X_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* SIM7070X_USING_GENERAL_OPS */

#ifdef SIM7070X_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* SIM7070X_USING_NETSERV_OPS */

#ifdef SIM7070X_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* SIM7070X_USING_PING_OPS */

#ifdef SIM7070X_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* SIM7070X_USING_IFCONFIG_OPS */

#ifdef SIM7070X_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;
    sim7070x_netconn_init(module);

    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);
#endif /* SIM7070X_USING_NETCONN_OPS */

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

os_err_t module_sim7070x_destroy(mo_object_t *self)
{
    mo_sim7070x_t *module = os_container_of(self, mo_sim7070x_t, parent);

#ifdef SIM7070X_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* SIM7070X_USING_NETCONN_OPS */

    mo_object_deinit(self);

    os_free(module);

    return OS_EOK;
}

#ifdef SIM7070X_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int sim7070x_auto_create(void)
{
    os_device_t *device = os_device_find(SIM7070X_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find %s interface device!", SIM7070X_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = SIM7070X_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = SIM7070X_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = SIM7070X_RECV_BUFF_LEN};

    mo_object_t *module = module_sim7070x_create(SIM7070X_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", SIM7070X_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", SIM7070X_NAME);

    return OS_EOK;
}
OS_CMPOENT_INIT(sim7070x_auto_create,OS_INIT_SUBLEVEL_MIDDLE);

#endif /* SIM7070X_AUTO_CREATE */

#endif /* MOLINK_USING_SIM7070X */

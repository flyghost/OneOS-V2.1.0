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
 * @file        gm510.c
 *
 * @brief       gm510.c module api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "gm510.h"
#include <stdlib.h>

#define MO_LOG_TAG "gm510"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_GM510

#define GM510_RETRY_TIMES (5)

#ifdef GM510_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test   = gm510_at_test,
    .get_imei  = gm510_get_imei,
    .get_imsi  = gm510_get_imsi,
    .get_iccid = gm510_get_iccid,
    .get_cfun  = gm510_get_cfun,
    .set_cfun  = gm510_set_cfun,
    .get_firmware_version = gm510_get_firmware_version,
};
#endif /* GM510_USING_GENERAL_OPS */

#ifdef GM510_USING_NETSERV_OPS
static const struct mo_netserv_ops gs_netserv_ops = {
    .set_attach    = gm510_set_attach,
    .get_attach    = gm510_get_attach,
    .set_reg       = gm510_set_reg,
    .get_reg       = gm510_get_reg,
    .set_cgact     = gm510_set_cgact,
    .get_cgact     = gm510_get_cgact,
    .get_csq       = gm510_get_csq,
};
#endif /* GM510_USING_NETSERV_OPS */

#ifdef GM510_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = gm510_ifconfig,
    .get_ipaddr           = gm510_get_ipaddr,
};
#endif /* GM510_USING_IFCONFIG_OPS */

#ifdef GM510_USING_NETCONN_OPS
extern void gm510_netconn_init(mo_gm510_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create        = gm510_netconn_create,
    .destroy       = gm510_netconn_destroy,
    .get_info      = gm510_netconn_get_info,
    .connect       = gm510_netconn_connect,
    .send          = gm510_netconn_send,
    .gethostbyname = gm510_netconn_gethostbyname,
};
#endif /* GM510_USING_NETCONN_OPS */



static os_err_t gm510_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    os_err_t result = at_parser_connect(parser, GM510_RETRY_TIMES);
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

mo_object_t *module_gm510_create(const char *name, void *parser_config)
{
    mo_gm510_t *module = (mo_gm510_t *)os_malloc(sizeof(mo_gm510_t));
    if (OS_NULL == module)
    {
        ERROR("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }
    
    os_task_msleep(5000);
    /* make sure gm510 power on and be ready */
    os_err_t result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        os_free(module);
        return OS_NULL;
    }

    result = gm510_at_init(&(module->parent));
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef GM510_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif

#ifdef GM510_USING_NETSERV_OPS
    module->parent.ops_table[MODULE_OPS_NETSERV] = &gs_netserv_ops;
#endif /* GM510_USING_NETSERV_OPS */
        
#ifdef GM510_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* GM510_USING_IFCONFIG_OPS */
        
#ifdef GM510_USING_NETCONN_OPS
    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    gm510_netconn_init(module);

    os_event_init(&module->netconn_evt, name);

    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

    module->curr_connect = -1;

#endif /* GM510_USING_NETCONN_OPS */

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

os_err_t module_gm510_destroy(mo_object_t *self)
{
    mo_gm510_t *module = os_container_of(self, mo_gm510_t, parent);

    mo_object_deinit(self);
    
#ifdef GM510_USING_NETCONN_OPS

    os_event_deinit(&module->netconn_evt);

    os_mutex_deinit(&module->netconn_lock);
    
#endif /* GM510_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef GM510_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int gm510_auto_create(void)
{
    os_device_t *device = os_device_find(GM510_DEVICE_NAME);
    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find M5311 interface device %s!", GM510_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = GM510_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = GM510_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = GM510_RECV_BUFF_LEN};

    mo_object_t *module = module_gm510_create(GM510_NAME, &parser_config);
    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", GM510_NAME);
        return OS_ERROR;
    }

    INFO("Auto create %s module object success!", GM510_NAME);
    return OS_EOK;
}
OS_CMPOENT_INIT(gm510_auto_create,OS_INIT_SUBLEVEL_MIDDLE);

#endif /* GM510_AUTO_CREATE */
#endif /* MOLINK_USING_GM510 */


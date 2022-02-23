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
 * @file        esp32.c
 *
 * @brief       esp32 module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp32.h"

#include <stdlib.h>
#include <string.h>
#include <os_task.h>

#define MO_LOG_TAG "esp32"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef MOLINK_USING_ESP32

#define ESP32_RETRY_TIMES (5)

#ifdef ESP32_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = esp32_at_test,
    .get_firmware_version = esp32_get_firmware_version,
};
#endif /* ESP32_USING_GENERAL_OPS */

#ifdef ESP32_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = esp32_ping,
};
#endif /* ESP32_USING_PING_OPS */

#ifdef ESP32_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = esp32_ifconfig,
    .get_ipaddr           = esp32_get_ipaddr,
    .set_dnsserver        = esp32_set_dnsserver,
    .get_dnsserver        = esp32_get_dnsserver,
};
#endif /* ESP32_USING_IFCONFIG_OPS */

#ifdef ESP32_USING_NETCONN_OPS
extern os_err_t esp32_netconn_init(mo_esp32_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = esp32_netconn_create,
    .destroy              = esp32_netconn_destroy,
    .gethostbyname        = esp32_netconn_gethostbyname,
    .connect              = esp32_netconn_connect,
    .send                 = esp32_netconn_send,
    .get_info             = esp32_netconn_get_info,
};
#endif /* ESP32_USING_NETCONN_OPS */

#ifdef ESP32_USING_WIFI_OPS
extern os_err_t esp32_wifi_init(mo_object_t *module);

static const struct mo_wifi_ops gs_wifi_ops = {
    .set_mode             = esp32_wifi_set_mode,
    .get_mode             = esp32_wifi_get_mode,
    .get_stat             = esp32_wifi_get_stat,
    .connect_ap           = esp32_wifi_connect_ap,
    .scan_info            = esp32_wifi_scan_info,
    .disconnect_ap        = esp32_wifi_disconnect_ap,
};
#endif /* ESP32_USING_WIFI_OPS */

static void urc_ready_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("AT firmware started successfully");
}

static void urc_busy_p_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("system is processing a commands and it cannot respond to the current commands.");
}

static void urc_busy_s_func(struct at_parser *parser, const char *data, os_size_t size)
{
    DEBUG("system is sending data and it cannot respond to the current commands.");
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "ready",  .suffix = "\r\n", .func = urc_ready_func},
    {.prefix = "busy p", .suffix = "\r\n", .func = urc_busy_p_func},
    {.prefix = "busy s", .suffix = "\r\n", .func = urc_busy_s_func},
};

static os_err_t esp32_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    os_err_t result = at_parser_connect(parser, ESP32_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_esp32_create(const char *name, void *parser_config)
{
    mo_esp32_t *module = (mo_esp32_t *)os_malloc(sizeof(mo_esp32_t));
    if (module == OS_NULL)
    {
        ERROR("Create %s module instance failed, no enough memory.", name);
        return OS_NULL;
    }

    os_err_t result = mo_object_init(&(module->parent), name, parser_config);
    if (result != OS_EOK)
    {
        ERROR("Create %s module instance failed, obj init failed.", name);
        os_free(module);
        return OS_NULL;
    }

    result = esp32_at_init(&module->parent);
    if (result != OS_EOK)
    {
        ERROR("Create %s module instance failed, at parser init failed.", name);
        goto __exit;
    }

#ifdef ESP32_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* ESP32_USING_GENERAL_OPS */

#ifdef ESP32_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* ESP32_USING_PING_OPS */

#ifdef ESP32_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* ESP32_USING_IFCONFIG_OPS */

#ifdef ESP32_USING_WIFI_OPS
    result = esp32_wifi_init(&module->parent);
    if (result != OS_EOK)
    {
        ERROR("Create %s module instance failed, wifi init failed.", name);
        goto __exit;
    }

    result = esp32_wifi_set_mode(&module->parent, MO_WIFI_MODE_STA);
    if (result != OS_EOK)
    {
        ERROR("Create %s module instance failed, set wifi mode failed.", name);
        goto __exit;
    }

    module->parent.ops_table[MODULE_OPS_WIFI] = &gs_wifi_ops;
    module->wifi_stat = MO_WIFI_STAT_INIT;
#endif /* ESP32_USING_WIFI_OPS */

#ifdef ESP32_USING_NETCONN_OPS
    result = esp32_netconn_init(module);
    if (result != OS_EOK)
    {
        ERROR("Create %s module instance failed, netconn init failed.", name);
        goto __exit;
    }

    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

    module->curr_connect = -1;
#endif /* ESP32_USING_NETCONN_OPS */

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

os_err_t module_esp32_destroy(mo_object_t *self)
{
    mo_esp32_t *module = os_container_of(self, mo_esp32_t, parent);

    mo_object_deinit(self);

#ifdef ESP32_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* ESP32_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef ESP32_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int esp32_auto_create(void)
{
    os_device_t *device = os_device_find(ESP32_DEVICE_NAME);

    if (device == OS_NULL)
    {
        ERROR("Auto create failed, Can not find ESP32 interface device %s!", ESP32_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = ESP32_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

    mo_parser_config_t parser_config = {.parser_name   = ESP32_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = ESP32_RECV_BUFF_LEN};

    mo_object_t *module = module_esp32_create(ESP32_NAME, &parser_config);

    if (module == OS_NULL)
    {
        ERROR("Auto create failed, Can not create %s module object!", ESP32_NAME);
        return OS_ERROR;
    }

#ifdef ESP32_AUTO_CONNECT_AP
    os_err_t result = mo_wifi_connect_ap(module, ESP32_CONNECT_SSID, ESP32_CONNECT_PASSWORD);
#endif /* ESP32_AUTO_CONNECT_AP */

    INFO("Auto create %s module object success!", ESP32_NAME);

    return OS_EOK;
}

OS_CMPOENT_INIT(esp32_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* ESP32_AUTO_CREATE */

#endif /* MOLINK_USING_ESP32 */

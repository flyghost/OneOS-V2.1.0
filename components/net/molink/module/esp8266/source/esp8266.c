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
 * @file        esp8266.c
 *
 * @brief       esp8266 module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp8266.h"

#include <stdlib.h>
#include <string.h>
#include <os_task.h>

#ifdef ESP8266_USING_HW_CONTROL
#include <drv_gpio.h>
#endif

#define MO_LOG_TAG "esp8266"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef MOLINK_USING_ESP8266

#define ESP8266_RETRY_TIMES (5)

#ifndef ESP8266_RST_PIN_NUM
#define ESP8266_RST_PIN_NUM (-1)
#endif

#ifdef ESP8266_USING_GENERAL_OPS
static const struct mo_general_ops gs_general_ops = {
    .at_test              = esp8266_at_test,
    .get_firmware_version = esp8266_get_firmware_version,
};
#endif /* ESP8266_USING_GENERAL_OPS */

#ifdef ESP8266_USING_PING_OPS
static const struct mo_ping_ops gs_ping_ops = {
    .ping                 = esp8266_ping,
};
#endif /* ESP8266_USING_PING_OPS */

#ifdef ESP8266_USING_IFCONFIG_OPS
static const struct mo_ifconfig_ops gs_ifconfig_ops = {
    .ifconfig             = esp8266_ifconfig,
    .get_ipaddr           = esp8266_get_ipaddr,
    .set_dnsserver        = esp8266_set_dnsserver,
    .get_dnsserver        = esp8266_get_dnsserver,
};
#endif /* ESP8266_USING_IFCONFIG_OPS */

#ifdef ESP8266_USING_NETCONN_OPS
extern os_err_t esp8266_netconn_init(mo_esp8266_t *module);

static const struct mo_netconn_ops gs_netconn_ops = {
    .create               = esp8266_netconn_create,
    .destroy              = esp8266_netconn_destroy,
    .gethostbyname        = esp8266_netconn_gethostbyname,
    .bind                 = esp8266_netconn_bind,
    .connect              = esp8266_netconn_connect,
    .sendto               = esp8266_netconn_sendto,
    .send                 = esp8266_netconn_send,
    .get_info             = esp8266_netconn_get_info,
};
#endif /* ESP8266_USING_NETCONN_OPS */

#ifdef ESP8266_USING_WIFI_OPS
extern os_err_t esp8266_wifi_init(mo_object_t *module);

static const struct mo_wifi_ops gs_wifi_ops = {
    .set_mode             = esp8266_wifi_set_mode,
    .get_mode             = esp8266_wifi_get_mode,
    .get_stat             = esp8266_wifi_get_stat,
    .get_sta_cip          = esp8266_wifi_get_sta_cip,
    .set_ap_cip           = esp8266_wifi_set_ap_cip,
    .get_ap_cip           = esp8266_wifi_get_ap_cip,
    .get_sta_mac          = esp8266_wifi_get_sta_mac,
    .get_ap_mac           = esp8266_wifi_get_ap_mac,
    .connect_ap           = esp8266_wifi_connect_ap,
    .scan_info            = esp8266_wifi_scan_info,
    .disconnect_ap        = esp8266_wifi_disconnect_ap,
    .start_ap             = esp8266_wifi_start_ap,
    .stop_ap              = esp8266_wifi_stop_ap,
};
#endif /* ESP8266_USING_WIFI_OPS */

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


#ifdef ESP8266_USING_HW_CONTROL

void esp8266_hw_rst(os_base_t rst_pin)
{
    if (-1 == rst_pin)
    {
        return;
    }

    os_pin_mode(rst_pin, PIN_MODE_OUTPUT);

    os_pin_write(rst_pin, PIN_LOW);

    os_task_msleep(10);

    os_pin_write(rst_pin, PIN_HIGH);

    os_task_msleep(200);

    INFO("The ESP8266 hardware reset is complete.");
}

#endif

static os_err_t esp8266_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    os_err_t result = at_parser_connect(parser, ESP8266_RETRY_TIMES);
    if (result != OS_EOK)
    {
        ERROR("Connect to %s module failed, please check whether the module connection is correct", self->name);
        return result;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_esp8266_create(const char *name, void *parser_config)
{
    mo_esp8266_t *module = (mo_esp8266_t *)os_malloc(sizeof(mo_esp8266_t));
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

    result = esp8266_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef ESP8266_USING_GENERAL_OPS
    module->parent.ops_table[MODULE_OPS_GENERAL] = &gs_general_ops;
#endif /* ESP8266_USING_GENERAL_OPS */

#ifdef ESP8266_USING_PING_OPS
    module->parent.ops_table[MODULE_OPS_PING] = &gs_ping_ops;
#endif /* ESP8266_USING_PING_OPS */

#ifdef ESP8266_USING_IFCONFIG_OPS
    module->parent.ops_table[MODULE_OPS_IFCONFIG] = &gs_ifconfig_ops;
#endif /* ESP8266_USING_IFCONFIG_OPS */

#ifdef ESP8266_USING_GENERAL_OPS
    mo_firmware_version_t version;

    version.line_counts = 0;
    version.ver_info = OS_NULL;

    result = mo_get_firmware_version(&module->parent, &version);
    if (result == OS_EOK)
    {
        for (int i = 0; i < version.line_counts; i++)
        {
            INFO("%s", version.ver_info[i]);
        }

        mo_get_firmware_version_free(&version);
    }
#endif /* ESP8266_USING_GENERAL_OPS */
    
#ifdef ESP8266_USING_WIFI_OPS

    //os_task_msleep(2000);
    result = esp8266_wifi_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    //os_task_msleep(1000);
    result = esp8266_wifi_set_mode(&module->parent, MO_WIFI_MODE_AP_STA);
    if (result != OS_EOK)
    {
        goto __exit;
    }
		
    /* not all esp8266 soft version support IPV6 */
#ifdef ESP8266_USING_IPV6
    result = esp8266_wifi_set_ipv6(&module->parent, MO_WIFI_CIPV6_ENABLE);
    if (result != OS_EOK)
    {
        INFO("module no support ipv6");
    }
#endif   
		
    result = esp8266_wifi_set_ipdinofo(&module->parent, MO_WIFI_CIPINFO_SHOW_IP_PORT);
    if (result != OS_EOK)
    {
        goto __exit;
    }
    module->parent.ops_table[MODULE_OPS_WIFI] = &gs_wifi_ops;

    module->wifi_stat = MO_WIFI_STAT_INIT;
#endif /* ESP8266_USING_WIFI_OPS */

#ifdef ESP8266_USING_NETCONN_OPS
    result = esp8266_netconn_init(module);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    module->parent.ops_table[MODULE_OPS_NETCONN] = &gs_netconn_ops;

    os_event_init(&module->netconn_evt, name);
    os_mutex_init(&module->netconn_lock, name, OS_TRUE);

    module->curr_connect = -1;
#endif /* ESP8266_USING_NETCONN_OPS */

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

os_err_t module_esp8266_destroy(mo_object_t *self)
{
    mo_esp8266_t *module = os_container_of(self, mo_esp8266_t, parent);

    mo_object_deinit(self);

#ifdef ESP8266_USING_NETCONN_OPS
    os_event_deinit(&module->netconn_evt);
    os_mutex_deinit(&module->netconn_lock);
#endif /* ESP8266_USING_NETCONN_OPS */

    os_free(module);

    return OS_EOK;
}

#ifdef ESP8266_AUTO_CREATE
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

int esp8266_auto_create(void)
{
    os_device_t *device = os_device_find(ESP8266_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find ESP8266 interface device %s!", ESP8266_DEVICE_NAME);
        return OS_ERROR;
    }

    uart_config.baud_rate = ESP8266_DEVICE_RATE;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);

#ifdef ESP8266_USING_HW_CONTROL
    esp8266_hw_rst(ESP8266_RST_PIN_NUM);
#endif

    mo_parser_config_t parser_config = {.parser_name   = ESP8266_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = ESP8266_RECV_BUFF_LEN};

    mo_object_t *module = module_esp8266_create(ESP8266_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", ESP8266_NAME);
        return OS_ERROR;
    }

#ifdef ESP8266_AUTO_CONNECT_AP
    os_err_t result = mo_wifi_connect_ap(module, ESP8266_CONNECT_SSID, ESP8266_CONNECT_PASSWORD);
#endif /* ESP8266_AUTO_CONNECT_AP */

    INFO("Auto create %s module object success!", ESP8266_NAME);

    return OS_EOK;
}
OS_CMPOENT_INIT(esp8266_auto_create, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* ESP8266_AUTO_CREATE */

#endif /* MOLINK_USING_ESP8266 */

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
 * @file        rg500q.c
 *
 * @brief       rg500q module link kit factory api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "rg500q.h"

#include <stdlib.h>
#include <string.h>
#include <device.h>
#include <mo_common.h>
#include <os_task.h>

#define MO_LOG_TAG "rg500q"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#ifdef MOLINK_USING_RG500Q

#define RG500Q_RETRY_TIMES (10)

extern void rg500q_urc_register(mo_rg500q_t *module);

static void urc_info_func(struct at_parser *parser, const char *data, os_size_t size)
{
   DEBUG("%.*s", size, data);
}

static at_urc_t gs_urc_table[] = {
    { .prefix = "RDY",     .suffix = "\r\n", .func = urc_info_func },
    { .prefix = "+CPIN:",  .suffix = "\r\n", .func = urc_info_func },
    { .prefix = "+QIND:",  .suffix = "\r\n", .func = urc_info_func },
    { .prefix = "+QUSIM:", .suffix = "\r\n", .func = urc_info_func },
    { .prefix = "+CMTI:",  .suffix = "\r\n", .func = urc_info_func },
};

static os_err_t rg500q_at_init(mo_object_t *self)
{
    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};
    
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    /* Make sure module in data mode. */
    at_parser_exec_lock(parser);
    os_task_msleep(RG500Q_PPP_EXIT_DELAY_MS);
    at_parser_send(parser, "+++", 3);
    os_task_msleep(RG500Q_PPP_EXIT_DELAY_MS);
    at_parser_exec_unlock(parser);

    /* Test connection. */
    result = at_parser_connect(parser, RG500Q_RETRY_TIMES);
    if (result != OS_EOK)
    {
       ERROR("Connect to %s module failed", self->name);
        return result;
    }

    return at_parser_exec_cmd(parser, &resp, "ATE0");
}

mo_object_t *module_rg500q_create(const char *name, void *parser_config)
{
    mo_rg500q_t *module = (mo_rg500q_t *)os_calloc(1, sizeof(mo_rg500q_t));
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

    result = rg500q_at_init(&module->parent);
    if (result != OS_EOK)
    {
        goto __exit;
    }

#ifdef RG500Q_USING_PPP_OPS
    rg500q_urc_register(module);
    result = rg500q_ppp_startup(&module->parent);
#endif /* RG500Q_USING_IFCONFIG_OPS */

__exit:
    if (result != OS_EOK)
    {
        if (OS_NULL != mo_object_get_by_name(name))
        {
            mo_object_deinit(&module->parent);
        }

        os_free(module);

        ERROR("%s module create failed.", name);

        return OS_NULL;
    }

    return &(module->parent);
}

#ifdef RG500Q_SUPPORT_DESTROY
os_err_t module_rg500q_destroy(mo_object_t *self)
{
    OS_ASSERT(OS_NULL != self);
    
    mo_rg500q_t *module = os_container_of(self, mo_rg500q_t, parent);
    
    // rg500q_ppp_shutdown(self);

    mo_object_deinit(self);

    os_free(module);

    return OS_EOK;
}
#endif /* RG500Q_SUPPORT_DESTROY */

#ifdef RG500Q_AUTO_CREATE
void rg500q_auto_create(void *parameter)
{
    os_device_t *device = os_device_find(RG500Q_DEVICE_NAME);

    if (OS_NULL == device)
    {
        ERROR("Auto create failed, Can not find RG500Q interface device %s!", RG500Q_DEVICE_NAME);
        return;
    }

    INFO("Auto create %s module object with usb[%s]", RG500Q_NAME, RG500Q_DEVICE_NAME);

    mo_parser_config_t parser_config = {.parser_name   = RG500Q_NAME,
                                        .parser_device = device,
                                        .recv_buff_len = RG500Q_RECV_BUFF_LEN};

    mo_object_t *module = module_rg500q_create(RG500Q_NAME, &parser_config);

    if (OS_NULL == module)
    {
        ERROR("Auto create failed, Can not create %s module object!", RG500Q_NAME);
        return;
    }

    INFO("Auto create %s module object success usb dev[%s]!", RG500Q_NAME, RG500Q_DEVICE_NAME);
    return;
}

os_err_t rg500q_device_notify_callback(os_device_t *dev, os_ubase_t event, os_ubase_t args)
{
    os_task_t   *task   = OS_NULL;

#ifdef RG500Q_SUPPORT_DESTROY
    mo_object_t *module = OS_NULL;
#endif /* RG500Q_SUPPORT_DESTROY */

    if(strcmp(dev->name, RG500Q_DEVICE_NAME))
    {
        return OS_EOK;
    }

    switch(event)
    {
        case ION_GENERIC_NONE:
            INFO("%s none: %d", dev->name, args);
            break;
        case ION_GENERIC_REGISTER:
            INFO("%s register: %d", dev->name, args);
            task = os_task_create(RG500Q_NAME, rg500q_auto_create, OS_NULL, 2048, 16);
            if (OS_NULL != task)
            {
                os_task_startup(task);
                INFO("RG500Q start auto create task.");
            }
            else
            {
                ERROR("RG500Q start auto create task failed.");
                OS_ASSERT(OS_NULL != task);
            }
            break;
        case ION_GENERIC_UNREGISTER:
            INFO("%s unregister: %d", dev->name, args);
#ifdef RG500Q_SUPPORT_DESTROY
            module = mo_get_by_name(RG500Q_NAME);
            if (OS_NULL != module)
            {
                module_rg500q_destroy(module);
                INFO("RG500Q module destroyed success.");
            }
            else
            {
                ERROR("RG500Q module destroyed failed.");
                OS_ASSERT(OS_NULL != module);
            }
#endif /* RG500Q_SUPPORT_DESTROY */
            break;
        default:
            break;
    }
   return OS_EOK;
}

os_err_t rg500q_device_notify_register(void)
{
    os_device_notify_register(OS_NULL, rg500q_device_notify_callback, OS_NULL);
    return OS_EOK;
}
OS_CMPOENT_INIT(rg500q_device_notify_register, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* RG500Q_AUTO_CREATE */

#endif /* MOLINK_USING_RG500Q */

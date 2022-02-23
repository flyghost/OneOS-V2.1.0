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
 * @file        mo_object.c
 *
 * @brief       module link kit object api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>

#define MO_LOG_TAG "molink.core"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef NET_USING_MOLINK

static os_slist_node_t gs_mo_object_list    = {0};
static mo_object_t    *gs_mo_object_default = OS_NULL;

static void mo_object_list_add(mo_object_t *self)
{
    os_schedule_lock();

    os_slist_init(&(self->list));

    if (OS_NULL == gs_mo_object_default)
    {
        gs_mo_object_default = self;
    }

    /* tail insertion */
    os_slist_add_tail(&(gs_mo_object_list), &(self->list));

    os_schedule_unlock();
}

static void mo_object_list_del(mo_object_t *self)
{
    OS_ASSERT(self != OS_NULL);

    os_slist_node_t *node  = OS_NULL;
    mo_object_t     *entry = OS_NULL;

    os_schedule_lock();

    for (node = &gs_mo_object_list; node; node = os_slist_next(node))
    {
        entry = os_slist_entry(node, mo_object_t, list);
        if (entry == self)
        {
            os_slist_del(&(gs_mo_object_list), &(self->list));

            if (gs_mo_object_default == self)
            {
                gs_mo_object_default = OS_NULL;
            }
            break;
        }
    }

    os_schedule_unlock();
}

mo_object_t *mo_object_get_by_name(const char *name)
{
    OS_ASSERT(name != OS_NULL);

    os_slist_node_t *node  = OS_NULL;
    mo_object_t     *entry = OS_NULL;

    if (OS_NULL == gs_mo_object_list.next)
    {
        return OS_NULL;
    }

    os_schedule_lock();

    for (node = &gs_mo_object_list; node; node = os_slist_next(node))
    {
        entry = os_slist_entry(node, mo_object_t, list);
        if (entry && (strncmp(entry->name, name, OS_NAME_MAX) == 0))
        {
            os_schedule_unlock();
            return entry;
        }
    }

    os_schedule_unlock();

    return OS_NULL;
}

mo_object_t *mo_object_get_default(void)
{
    if (OS_NULL == gs_mo_object_default)
    {
        ERROR("There are no default module in the system now");
    }

    return gs_mo_object_default;
}

void mo_object_set_default(mo_object_t *self)
{
    OS_ASSERT(self != OS_NULL);

    gs_mo_object_default = self;
}

#ifdef MOLINK_PLATFORM_MCU

mo_object_t *module_object_get_by_device(os_device_t *device)
{
    OS_ASSERT(device != OS_NULL);

    os_slist_node_t *node  = OS_NULL;
    mo_object_t     *entry = OS_NULL;

    if (OS_NULL == gs_mo_object_list.next)
    {
        return OS_NULL;
    }

    os_schedule_lock();

    for (node = &gs_mo_object_list; node; node = os_slist_next(node))
    {
        entry = os_slist_entry(node, mo_object_t, list);
        if (entry && entry->parser.device == device)
        {
            os_schedule_unlock();
            return entry;
        }
    }

    os_schedule_unlock();

    return OS_NULL;
}

static os_err_t mo_object_init_with_mcu(mo_object_t *self, const char *name, mo_parser_config_t *config)
{
    mo_object_t *temp = module_object_get_by_device(config->parser_device);
    if (temp != OS_NULL)
    {
        ERROR("Failed init module object, device %s has occupied by the module %s",
              config->parser_device->name,
              temp->name);
        return OS_ERROR;
    }

    os_err_t result = at_parser_init(&self->parser, config->parser_name, config->parser_device, config->recv_buff_len);
    if (result != OS_EOK)
    {
        ERROR("Module object create parser failed!");
        return result;
    }

    self->platform = MO_PLATFORM_MCU;

    mo_object_list_add(self);

    at_parser_startup(&self->parser);

    return OS_EOK;
}

#endif /* MOLINK_PLATFORM_MCU */

#ifdef MOLINK_PLATFORM_OPENCPU
static os_err_t mo_object_init_with_opencpu(mo_object_t *self, const char *name)
{
    self->platform = MO_PLATFORM_OPENCPU;

    mo_object_list_add(self);

    return OS_EOK;
}
#endif /* MOLINK_PLATFORM_OPENCPU */

/**
 ***********************************************************************************************************************
 * @brief           Init an instance of a molink module object
 *
 * @param[in]       self            The pointer to molink module instance
 * @param[in]       name            The molink module instance name
 * @param[in]       parser_config   Parameters to the AT parser, for the MCU platform module must not be OS_NULL,
 *                                  OpenCPU platform module must set to OS_NULL
 *
 * @return          On success, return OS_EOK; on error, return a error code.
 ***********************************************************************************************************************
 */
os_err_t mo_object_init(mo_object_t *self, const char *name, void *parser_config)
{
    OS_ASSERT(name != OS_NULL);
    OS_ASSERT(self != OS_NULL);

    if (strlen(name) == 0 || mo_object_get_by_name(name) != OS_NULL)
    {
        ERROR("Failed init module object, module name error");
        return OS_ERROR;
    }

    memset(self, 0, sizeof(mo_object_t));

    strncpy(self->name, name, OS_NAME_MAX);

#ifdef MOLINK_PLATFORM_MCU
    if (OS_NULL != parser_config)
    {
        return mo_object_init_with_mcu(self, name, (mo_parser_config_t *)parser_config);
    }
#endif /* MOLINK_PLATFORM_MCU */

#ifdef MOLINK_PLATFORM_OPENCPU
    if (OS_NULL == parser_config)
    {
        return mo_object_init_with_opencpu(self, name);
    }
#endif /* MOLINK_PLATFORM_OPENCPU */

    ERROR("Failed init module object, please check the parameter");

    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           Init an instance of a molink module object
 *
 * @param[in]       self            The pointer to molink module instance
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t mo_object_deinit(mo_object_t *self)
{
    OS_ASSERT(self != OS_NULL);

#ifdef MOLINK_PLATFORM_MCU
    if (MO_PLATFORM_MCU == self->platform)
    {
        at_parser_deinit(&self->parser);
    }
#endif /* MOLINK_PLATFORM_MCU */

    mo_object_list_del(self);

    return OS_EOK;
}

#endif /* NET_USING_MOLINK */

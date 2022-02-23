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
 * @file        os_event.h
 *
 * @brief       Header file for event interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __OS_EVENT_H__
#define __OS_EVENT_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_EVENT

#define OS_EVENT_WAKE_TYPE_PRIO     0x55
#define OS_EVENT_WAKE_TYPE_FIFO     0xAA

#define OS_EVENT_OPTION_AND         0x00000001
#define OS_EVENT_OPTION_OR          0x00000002
#define OS_EVENT_OPTION_CLEAR       0x00000004
#define OS_EVENT_OPTION_MASK        0x00000007

struct os_event
{
    os_list_node_t  task_list_head;         /* Block task list head */
    os_list_node_t  resource_node;          /* Node in resource list */

    os_uint32_t     set;                    /* Event set. */
    os_uint8_t      object_inited;          /* If mutex object is inited, value is OS_KOBJ_INITED */
    os_uint8_t      object_alloc_type;      /* Indicates whether memory is allocated dynamically or statically,
                                               value is OS_KOBJ_ALLOC_TYPE_STATIC or OS_KOBJ_ALLOC_TYPE_DYNAMIC */
    os_uint8_t      wake_type;              /* The type to wake up blocking tasks, value is OS_EVENT_WAKE_TYPE_PRIO
                                               or OS_EVENT_WAKE_TYPE_FIFO */
    char            name[OS_NAME_MAX + 1];  /* Mutex name */
};
typedef struct os_event os_event_t;

extern os_err_t    os_event_init(os_event_t *event, const char *name);
extern os_err_t    os_event_deinit(os_event_t *event);

#ifdef OS_USING_SYS_HEAP
extern os_event_t *os_event_create(const char *name);
extern os_err_t    os_event_destroy(os_event_t *event);
#endif

extern os_err_t    os_event_send(os_event_t *event, os_uint32_t set);

extern os_err_t    os_event_recv(os_event_t     *event,
                                 os_uint32_t     interested_set,
                                 os_uint32_t     option,
                                 os_tick_t       timeout,
                                 os_uint32_t    *recved_set);

extern os_err_t   os_event_clear(os_event_t *event, os_uint32_t interested_clear);
extern os_int32_t os_event_get(os_event_t *event);

extern os_err_t   os_event_set_wake_type(os_event_t *event, os_uint8_t wake_type);

extern os_err_t os_event_sync(os_event_t  *event,
                              os_uint32_t  set,
                              os_uint32_t  interested_set,
                              os_tick_t    timeout,
                              os_uint32_t *recved_set);

#endif /* OS_USING_EVENT */

#ifdef __cplusplus
}
#endif

#endif /* __OS_EVENT_H__ */


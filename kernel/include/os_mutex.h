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
 * @file        os_mutes.h
 *
 * @brief       Header file for mutex interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __OS_MUTEX_H__
#define __OS_MUTEX_H__

#include <oneos_config.h>
#include <os_errno.h>
#include <os_types.h>
#include <os_list.h>
#include <os_stddef.h>
#include <os_task.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_MUTEX

#define OS_MUTEX_WAKE_TYPE_PRIO             0x55
#define OS_MUTEX_WAKE_TYPE_FIFO             0xAA

struct os_mutex
{
    os_list_node_t  task_list_head;         /* Block task list head */
    os_list_node_t  resource_node;          /* Node in resource list */
    os_list_node_t  hold_node;
    
    os_task_t      *owner;                  /* Mutex owner */

    os_uint32_t     lock_count;             /* Current lock count */
    os_bool_t       is_recursive;           /* Support recursive call. */

    os_uint8_t      object_inited;          /* If mutex object is inited, value is OS_KOBJ_INITED */
    os_uint8_t      object_alloc_type;      /* Indicates whether memory is allocated dynamically or statically,
                                               value is OS_KOBJ_ALLOC_TYPE_STATIC or OS_KOBJ_ALLOC_TYPE_DYNAMIC */
    os_uint8_t      wake_type;              /* The type to wake up blocking tasks, value is OS_MUTEX_WAKE_TYPE_PRIO
                                               or OS_MUTEX_WAKE_TYPE_FIFO */
    os_uint8_t      original_priority;
    
    char            name[OS_NAME_MAX + 1]; /* Mutex name */
};
typedef struct os_mutex os_mutex_t;

extern os_err_t    os_mutex_init(os_mutex_t *mutex, const char *name, os_bool_t recursive);
extern os_err_t    os_mutex_deinit(os_mutex_t *mutex);

#ifdef OS_USING_SYS_HEAP
extern os_mutex_t *os_mutex_create(const char *name, os_bool_t recursive);
extern os_err_t    os_mutex_destroy(os_mutex_t *mutex);
#endif

extern os_err_t    os_mutex_lock(os_mutex_t *mutex, os_tick_t timeout);
extern os_err_t    os_mutex_unlock(os_mutex_t *mutex);
extern os_err_t    os_mutex_recursive_lock(os_mutex_t *mutex, os_tick_t timeout);
extern os_err_t    os_mutex_recursive_unlock(os_mutex_t *mutex);

extern os_err_t    os_mutex_set_wake_type(os_mutex_t *mutex, os_uint8_t wake_type);
extern os_task_t  *os_mutex_get_owner(os_mutex_t *mutex);

#endif /* OS_USING_MUTEX */

#ifdef __cplusplus
}
#endif

#endif /* __OS_MUTEX_H__ */


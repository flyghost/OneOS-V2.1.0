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
 * @file        os_sem.h
 *
 * @brief       Header file for semaphore interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __OS_SEM_H__
#define __OS_SEM_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_SEMAPHORE

#define OS_SEM_WAKE_TYPE_PRIO               0x55
#define OS_SEM_WAKE_TYPE_FIFO               0xAA

#define OS_SEM_MAX_VALUE    OS_UINT32_MAX

struct os_semaphore
{
    os_list_node_t task_list_head;         /* Block task list head  */
    os_list_node_t resource_node;          /* Node in resource list */

    os_uint32_t    count;                  /* Current count */
    os_uint32_t    max_count;              /* Semaphore support maximum value */
    os_uint8_t     object_inited;          /* If semaphore object is inited, value is OS_KOBJ_INITED */
    os_uint8_t     object_alloc_type;      /* Indicates whether memory is allocated dynamically or statically,
                                              value is OS_KOBJ_ALLOC_TYPE_STATIC or OS_KOBJ_ALLOC_TYPE_DYNAMIC */
    os_uint8_t     wake_type;              /* The type to wake up blocking tasks, value is OS_SEM_WAKE_TYPE_PRIO
                                              or OS_SEM_WAKE_TYPE_FIFO */

    char           name[OS_NAME_MAX + 1];  /* Semaphore name */
};
typedef struct os_semaphore os_sem_t;

extern os_err_t    os_sem_init(os_sem_t *sem, const char *name, os_uint32_t value, os_uint32_t max_value);
extern os_err_t    os_sem_deinit(os_sem_t *sem);

#ifdef OS_USING_SYS_HEAP
extern os_sem_t   *os_sem_create(const char *name, os_uint32_t value, os_uint32_t max_value);
extern os_err_t    os_sem_destroy(os_sem_t *sem);
#endif

extern os_err_t    os_sem_wait(os_sem_t *sem, os_tick_t timeout);
extern os_err_t    os_sem_post(os_sem_t *sem);

extern os_err_t    os_sem_set_wake_type(os_sem_t *sem, os_uint8_t wake_type);
extern os_uint32_t os_sem_get_count(os_sem_t *sem);
extern os_uint32_t os_sem_get_max_count(os_sem_t *sem);

#endif /* OS_USING_SEMAPHORE */

#ifdef __cplusplus
}
#endif

#endif /* __OS_SEM_H__ */


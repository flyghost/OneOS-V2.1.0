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
 * @file        os_task.h
 *
 * @brief       Header file for task interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-18   OneOS Team      First Version.
 * 2020-11-10   OneOS Team      Refactor header file for task interface.
 ***********************************************************************************************************************
 */

#ifndef __OS_TASK_H__
#define __OS_TASK_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_list.h>
#include <arch_interrupt.h>
#include <arch_task.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_TASK_STATE_EMPTY                 0x0000U
#define OS_TASK_STATE_INIT                  0x0001U
#define OS_TASK_STATE_READY                 0x0002U
#define OS_TASK_STATE_RUNNING               0x0004U
#define OS_TASK_STATE_SLEEP                 0x0008U
#define OS_TASK_STATE_BLOCK                 0x0010U
#define OS_TASK_STATE_SUSPEND               0x0020U
#define OS_TASK_STATE_CLOSE                 0x8000U
#define OS_TASK_STATE_MASK                  0xFFFFU

#define OS_IDLE_TASK_NAME                   "idle"
#define OS_RECYCLE_TASK_NAME                "recycle"

struct os_task
{
    /* begin: The order, position and content cannot be changed */
    void           *stack_top;                  /* Point to SP */
    void           *stack_begin;                /* The begin address of task stack */
    void           *stack_end;                  /* The end address of task stack */

    os_uint16_t     state;                      /* Task state, refer to OS_TASK_STATE_INIT, OS_TASK_STATE_READY ... */
    /* end:   The order, position and content cannot be changed */

    os_uint8_t      current_priority;           /* Current priority. */
    os_uint8_t      backup_priority;            /* Backup priority. */

    os_err_t        err_code;                   /* Error code. */
    os_err_t        switch_retval;              /* Task switch return value, defined in os_errno.h*/

    os_uint8_t      object_inited;              /* If task object is inited, value is OS_KOBJ_INITED */ 
    os_uint8_t      object_alloc_type;          /* Indicates whether memory is allocated dynamically or statically, 
                                                   value is OS_KOBJ_ALLOC_TYPE_STATIC or OS_KOBJ_ALLOC_TYPE_DYNAMIC */
    os_uint8_t      pad[2];

    os_list_node_t  resource_node;              /* Node in resource list */
    os_list_node_t	task_node;                  /* Node in ready queue or blocking queue */

    os_list_node_t  tick_node;                  /* Node in tick queue */	
    os_tick_t       tick_timeout;               /* Timeout */
    os_tick_t       tick_absolute;              /* Absolute time of timeout */

    os_tick_t       time_slice;                 /* Task's slice time (unit: tick). */
    os_tick_t       remaining_time_slice;       /* Task's remaining slice time (unit: tick). */

    os_list_node_t *block_list_head;            /* Where the task is blocked at */
    os_bool_t       is_wake_prio;               /* The wake type to task at block_list_head, according to priority or not */

#if defined(OS_USING_EVENT)
    os_uint32_t     event_set;
    os_uint32_t     event_option;
#endif

#if defined(OS_USING_MUTEX)
    os_list_node_t  hold_mutex_list_head;
#endif

    os_ubase_t      swap_data;

    void          (*cleanup)(void *user_data);  /* The cleanup function is provided by the user */
    void           *user_data;                  /* Private user data beyond this task. */

    char            name[OS_NAME_MAX + 1];      /* Task name */
};
typedef struct os_task os_task_t;


#ifdef OS_USING_SYS_HEAP
extern os_task_t   *os_task_create(const char   *name, 
                                   void        (*entry)(void *arg),
                                   void         *arg,
                                   os_uint32_t   stack_size,
                                   os_uint8_t    priority);

extern os_err_t     os_task_destroy(os_task_t *task);
#endif /* OS_USING_SYS_HEAP */

#define OS_TASK_STACK_DEFINE(name, size)    \
    OS_ALIGN(OS_ARCH_STACK_ALIGN_SIZE) os_uint8_t name[OS_ALIGN_UP(size, OS_ARCH_STACK_ALIGN_SIZE)]
#define OS_TASK_STACK_BEGIN_ADDR(name)      (void *)&name[0]
#define OS_TASK_STACK_SIZE(name)            sizeof(name)


extern os_err_t     os_task_init(os_task_t    *task,
                                 const char   *name, 
                                 void        (*entry)(void *arg),
                                 void         *arg,
                                 void         *stack_begin,
                                 os_uint32_t   stack_size,
                                 os_uint8_t    priority);

extern os_err_t     os_task_deinit(os_task_t *task);

extern void         os_task_set_cleanup_callback(os_task_t *task, void (*cleanup)(void *user_data), void *user_data);
extern os_err_t     os_task_startup(os_task_t *task);
extern os_err_t     os_task_suspend(os_task_t *task);
extern os_err_t     os_task_resume(os_task_t *task);
extern os_err_t     os_task_yield(void);

extern os_err_t     os_task_set_time_slice(os_task_t *task, os_tick_t new_time_slice);
extern os_tick_t    os_task_get_time_slice(os_task_t *task);
extern os_err_t     os_task_set_priority(os_task_t *task, os_uint8_t new_priority);
extern os_uint8_t   os_task_get_priority(os_task_t *task);

extern os_task_t   *os_task_self(void);
extern os_task_t   *os_task_find(const char *name);
extern os_bool_t    os_task_check_exist(os_task_t *task);
extern const char  *os_task_name(os_task_t *task);
extern os_uint16_t  os_task_get_state(os_task_t *task);
extern os_uint32_t  os_task_get_total_count(void);

extern os_err_t     os_task_tsleep(os_tick_t tick);
extern os_err_t     os_task_msleep(os_uint32_t ms);

extern void         os_schedule_lock(void);
extern void         os_schedule_unlock(void);
extern os_bool_t    os_is_schedule_locked(void);

extern void         os_set_errno(os_err_t err_code);
extern os_err_t     os_get_errno(void);
extern os_err_t    *os_errno(void);

#ifdef OS_TASK_SWITCH_NOTIFY
extern void         os_task_switch_notify(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __OS_TASK_H__ */


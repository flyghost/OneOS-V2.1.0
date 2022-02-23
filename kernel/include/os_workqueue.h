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
 * @file        os_workqueue.h
 *
 * @brief       Header file for workqueue interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __OS_WORKQUEUE_H__
#define __OS_WORKQUEUE_H__

#include <oneos_config.h>
#include <os_stddef.h>
#include <os_list.h>
#include <os_timer.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_WORKQUEUE

#define OS_WORK_STAGE_IDLE           0x0
#define OS_WORK_STAGE_DELAY          0x1
#define OS_WORK_STAGE_PENDING        0x2

struct os_workqueue;
struct os_work
{
    os_list_node_t   work_node;
    
    void           (*func)(void *data);         /* Callback function for work. */
    void            *data;                      /* Private data for callback function. */
    os_timer_t       timer;                     /* Using for delayed work. */

    struct os_workqueue * volatile workqueue;   /* Workqueue used to execute work. */

    os_uint8_t       flag;   
    os_uint8_t       object_inited;             /* If os_work is inited, value is OS_KOBJ_INITED */
};
typedef struct os_work os_work_t;

struct os_workqueue
{
    os_list_node_t  work_list_head;    
    os_work_t      *work_current;               /* Work in progress on workqueue. */
    os_task_t       worker_task;                /* Task on the workqueue to execute work. */

    os_spinlock_t   lock;                       /* Spin lock. */    
    os_sem_t        sem;                        /* Semaphore for synchronization. */    

    os_uint8_t      object_inited;              /* If os_workqueue is inited, value is OS_KOBJ_INITED */
};

typedef struct os_workqueue os_workqueue_t;

#ifdef OS_USING_SYS_HEAP
extern os_workqueue_t *os_workqueue_create(const char  *name,
                                           os_uint32_t  stack_size,
                                           os_uint8_t   priority,
                                           os_int32_t   cpu_index);
#endif

os_err_t os_workqueue_init(os_workqueue_t *queue,
                           const char     *name, 
                           void           *stack_begin,
                           os_uint32_t     stack_size,
                           os_uint8_t      priority,
                           os_int32_t      cpu_index);

extern os_err_t        os_submit_work_to_queue(os_workqueue_t *queue ,os_work_t *work, os_tick_t delay_time);

#ifdef OS_USING_SYSTEM_WORKQUEUE
extern os_err_t        os_submit_work(os_work_t *work, os_tick_t delay_time);
#endif /* OS_USING_SYSTEM_WORKQUEUE */

extern os_err_t        os_cancel_work(os_work_t *work);
extern os_err_t        os_cancel_work_sync(os_work_t *work);

extern void            os_work_init(os_work_t *work, void (*func)(void *data), void *data);

#endif /* OS_USING_WORKQUEUE */

#ifdef __cplusplus
}
#endif

#endif /* __OS_WORKQUEUE_H__ */


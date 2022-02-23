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
 * @file        ones_cmsis.h
 *
 * @brief       head file for CMSIS APIs adapter
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-26   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __ONEOS_CMSIS_H__
#define __ONEOS_CMSIS_H__

#include <oneos_config.h>
#include <cmsis_os2.h>
#include <os_memory.h>
#include <os_timer.h>
#include <os_mutex.h>
#include <os_event.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_mq.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @struct      thread_control_block
 *
 * @brief       Adapter CMSIS API thread function control block
 ***********************************************************************************************************************
 */
struct thread_control_block
{
    os_sem_t                   *joinable_sem;               /* Semaphore for joinable thread */
    os_task_t                  task;                       /* OneOS task control block */
    void                      *stack_start;
    uint32_t                   flags;                      /* Thread flag set attribution */
    os_uint8_t                 prio;                       /* CMSIS thread priority */
    os_list_node_t resource_node;
    uint8_t flags_options;
    uint32_t wait_flags;
    uint32_t thread_flags;
    os_timer_t timer;
    os_err_t    error;
    char      id;
};
typedef struct thread_control_block         thread_cb_t;

/**
 ***********************************************************************************************************************
 * @struct      timer_control_block
 *
 * @brief       Adapter CMSIS API timer function control block
 ***********************************************************************************************************************
 */
struct thread_timer_block
{
    os_timer_t                 timer;                      /* OneOS timer control block */
    uint32_t                      flags;                      /* CMSIS attribution value */
    char               name[OS_NAME_MAX];
    char      id;
};
typedef struct thread_timer_block           timer_cb_t;


#ifdef OS_USING_EVENT
/**
 ***********************************************************************************************************************
 * @struct      event_control_block
 *
 * @brief       Adapter CMSIS API event function control block
 ***********************************************************************************************************************
 */
struct event_control_block
{
    os_event_t                 event;                      /* OneOS event control block */
    uint32_t                      flags;                      /* CMSIS attribution value */
    char               name[OS_NAME_MAX];
    char      id;
};
typedef struct event_control_block          event_cb_t;

#endif

#ifdef OS_USING_MUTEX
/**
 ***********************************************************************************************************************
 * @struct      mutex_control_block
 *
 * @brief       Adapter CMSIS API mutex function control block
 ***********************************************************************************************************************
 */
struct mutex_control_block
{
    os_mutex_t                 mutex;                      /* OneOS event control block */
    uint32_t                      flags;                      /* CMSIS attribution value */
    char               name[OS_NAME_MAX];
    char      id;
};
typedef struct mutex_control_block          mutex_cb_t;

#endif

#ifdef OS_USING_SEMAPHORE
/**
 ***********************************************************************************************************************
 * @struct      semaphore_control_block
 *
 * @brief       Adapter CMSIS API semaphore function control block
 ***********************************************************************************************************************
 */
struct semaphore_control_block
{
    os_sem_t             sem;                        /* OneOS event control block */
    uint32_t                      flags;                      /* CMSIS attribution value */
    char               name[OS_NAME_MAX];
    char      id;
};
typedef struct semaphore_control_block      sem_cb_t;

#endif

#ifdef OS_USING_MEM_POOL
/**
 ***********************************************************************************************************************
 * @struct      mempool_control_block
 *
 * @brief       Adapter CMSIS API mempool function control block
 ***********************************************************************************************************************
 */
struct mempool_control_block
{
    os_mp_t               mp;                         /* OneOS event control block */
    uint32_t                      flags;                      /* CMSIS attribution value */
    char               name[OS_NAME_MAX];
    void *start_addr;
    char      id;
};
typedef struct mempool_control_block        mempool_cb_t;

#endif

#ifdef OS_USING_MESSAGEQUEUE
/**
 ***********************************************************************************************************************
 * @struct      messagequeue_control_block
 *
 * @brief       Adapter CMSIS API messagequeue function control block
 ***********************************************************************************************************************
 */
struct messagequeue_control_block
{
    void                           *init_msg_addr;              /* CMSIS init message address */
    os_uint32_t                     init_msg_size;              /* CMSIS init message size */
    os_mq_t                 mq;                         /* OneOS event control block */
    uint32_t                      flags;                      /* CMSIS attribution value */
    char               name[OS_NAME_MAX];
    char      id;
};
typedef struct messagequeue_control_block   mq_cb_t;

#endif


#endif

#ifdef __cplusplus
}
#endif


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
 * @file        nimble_port_oneos.c
 *
 * @brief       The function to init Nimble stack and start the staks.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stddef.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_memory.h>
#include <os_task.h>
#include <oneos_config.h>
#include "syscfg/syscfg.h"
#include "nimble/nimble_port.h"

#ifndef OS_USING_SYS_HEAP
#error "NimBLE npl depend on 'OS_USING_SYS_HEAP'"
#endif

extern void ble_ll_task(void *arg);

void nimble_port_oneos_init(void)
{
    nimble_port_init();

#if NIMBLE_CFG_CONTROLLER
    /*
     * Create task where NimBLE LL will run. This one is required as LL has its
     * own event queue and should have highest priority. The task function is
     * provided by NimBLE and in case of FreeRTOS it does not need to be wrapped
     * since it has compatible prototype.
     */
    os_task_t *task;

    task = os_task_create("ble_ll",
                          ble_ll_task,
                          OS_NULL,
                          MYNEWT_VAL(BLE_CTLR_THREAD_STACK_SIZE),
                          MYNEWT_VAL(BLE_CTLR_THREAD_PRIORITY));
    OS_ASSERT(OS_NULL != task);
    os_task_startup(task);
#endif

    return;
}

#if NIMBLE_CFG_HOST
void ble_hs_task(void *parameter)
{
    nimble_port_run();
}

void ble_hs_task_startup(void)
{
    os_task_t *task;

    task = os_task_create("ble_hs",
                          ble_hs_task,
                          OS_NULL,
                          MYNEWT_VAL(BLE_HOST_THREAD_STACK_SIZE),
                          MYNEWT_VAL(BLE_HOST_THREAD_PRIORITY));
    OS_ASSERT(OS_NULL != task);
    os_task_startup(task);

    return;
}
#endif

/**
 * Initialize a task.
 *
 * This function initializes the task structure pointed to by t,
 * clearing and setting it's stack pointer, provides sane defaults
 * and sets the task as ready to run, and inserts it into the operating
 * system scheduler.
 *
 * @param t The task to initialize
 * @param name The name of the task to initialize
 * @param func The task function to call
 * @param arg The argument to pass to this task function
 * @param prio The priority at which to run this task
 * @param sanity_itvl The time at which this task should check in with the
 *                    sanity task.  OS_WAIT_FOREVER means never check in
 *                    here.
 * @param stack_bottom A pointer to the bottom of a task's stack
 * @param stack_size The overall size of the task's stack.
 *
 * @return 0 on success, non-zero on failure.
 */
void nimble_task_init(struct ble_npl_task *ble_npl_task,
                      const char *name,
                      void (*entry)(void *parameter),
                      void *parameter,
                      uint8_t priority_invaild,
                      uint32_t sanity_itvl_invaild,
                      void *stack_start,
                      uint32_t stack_size)
{
    os_err_t ret;
    os_task_t *task;

    task = (os_task_t *)os_malloc(sizeof(os_task_t));
    OS_ASSERT(OS_NULL != task);

    memset(task, 0, sizeof(os_task_t));

    ret = os_task_init(task,
                       name,
                       entry,
                       parameter,
                       stack_start,
                       stack_size * 4,
                       6);
    OS_ASSERT(OS_EOK == ret);
    os_task_startup(task);

    ble_npl_task->t = task;
    return;
}

void ble_npl_eventq_run(void *evq)
{
    struct ble_npl_event *ev;

    ev = ble_npl_eventq_get((struct ble_npl_eventq *)evq, BLE_NPL_TIME_FOREVER);
    ble_npl_event_run(ev);
    return;
}
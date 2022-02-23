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
 * @file        os_sem.c
 *
 * @brief       This file implements the semaphore functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-11   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <os_sem.h>
#include <os_errno.h>
#include <arch_interrupt.h>
#include <string.h>
#include <os_spinlock.h>

#include "os_kernel_internal.h"

#ifdef OS_USING_SEMAPHORE

#define SEM_TAG             "SEM"

static os_list_node_t gs_os_sem_resource_list_head = OS_LIST_INIT(gs_os_sem_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_sem_resource_list_lock);

OS_INLINE void _k_sem_init(os_sem_t *sem, os_uint32_t value, os_uint32_t max_value, const char *name)
{
    os_list_init(&sem->task_list_head);

    sem->count     = value;
    sem->max_count = max_value;
    sem->wake_type = OS_SEM_WAKE_TYPE_PRIO;

    if (OS_NULL != name)
    {
        (void)strncpy(&sem->name[0], name, OS_NAME_MAX);
        sem->name[OS_NAME_MAX] = '\0';
    }
    else
    {
        sem->name[0] = '\0';
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initlialize a semaphore object.
 *
 * @param[in]       sem             The semaphore to be initialized.
 * @param[in]       name            The name of semaphore.
 * @param[in]       value           The init value of semaphore.
 * @param[in]       max_value       The maximum value of semaphore.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_sem_init(os_sem_t *sem, const char *name, os_uint32_t value, os_uint32_t max_value)
{
    os_err_t        ret;
    os_sem_t       *iter_sem;
    os_list_node_t *pos;

    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT(max_value >= value);
    OS_ASSERT(max_value >  0);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    ret = OS_EOK;

    os_spin_lock(&gs_os_sem_resource_list_lock);
    os_list_for_each(pos, &gs_os_sem_resource_list_head)
    {
        iter_sem = os_list_entry(pos, os_sem_t, resource_node);
        if (iter_sem == sem)
        {
            OS_KERN_LOG(KERN_ERROR, SEM_TAG, "The sem(addr: %p, name: %s) has been exist", iter_sem, iter_sem->name);
            ret = OS_EINVAL;
            break;
        }
    }

    if (OS_EOK == ret)
    {
        os_list_add_tail(&gs_os_sem_resource_list_head, &sem->resource_node);
        os_spin_unlock(&gs_os_sem_resource_list_lock);

        _k_sem_init(sem, value, max_value, name);
        sem->object_alloc_type = OS_KOBJ_ALLOC_TYPE_STATIC;
        sem->object_inited     = OS_KOBJ_INITED;
    }
    else
    {
        os_spin_unlock(&gs_os_sem_resource_list_lock);
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will deinitialize the specific semaphore.
 *
 * @param[in]       sem             The semaphore to be deinitialized.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_sem_deinit(os_sem_t *sem)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT(OS_KOBJ_INITED == sem->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_STATIC == sem->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    OS_KERNEL_ENTER();

    sem->object_inited = OS_KOBJ_DEINITED;
    k_cancle_all_blocked_task(&sem->task_list_head);

    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_sem_resource_list_lock);
    os_list_del(&sem->resource_node);
    os_spin_unlock(&gs_os_sem_resource_list_lock);

    return OS_EOK;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function will create a semaphore object from heap.
 *
 * @param[in]       name            The name of semaphore.
 * @param[in]       value           The init value of semaphore.
 * @param[in]       max_value       The maximum value of semaphore.
 *
 * @return          The pointer to the created semaphore.
 * @retval          pointer         If operation successful.
 * @retval          OS_NULL         Error occurred.
 ***********************************************************************************************************************
 */
os_sem_t *os_sem_create(const char *name, os_uint32_t value, os_uint32_t max_value)
{
    os_sem_t *sem;

    OS_ASSERT(max_value >= value);
    OS_ASSERT(max_value > 0U);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    sem = (os_sem_t *)OS_KERNEL_MALLOC(sizeof(os_sem_t));
    if (OS_NULL == sem)
    {
        OS_KERN_LOG(KERN_ERROR, SEM_TAG, "Semaphore memory malloc fail");
    }
    else
    {
        os_spin_lock(&gs_os_sem_resource_list_lock);
        os_list_add_tail(&gs_os_sem_resource_list_head, &sem->resource_node);
        os_spin_unlock(&gs_os_sem_resource_list_lock);

        _k_sem_init(sem, value, max_value, name);
        sem->object_alloc_type = OS_KOBJ_ALLOC_TYPE_DYNAMIC;
        sem->object_inited = OS_KOBJ_INITED;
    }

    return sem;
}

/**
 ***********************************************************************************************************************
 * @brief           Destory the specific semaphore object created from heap.
 *
 * @param[in]       sem             The semaphore to destroy.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_sem_destroy(os_sem_t *sem)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT(OS_KOBJ_INITED == sem->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_DYNAMIC == sem->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    OS_KERNEL_ENTER();

    sem->object_inited = OS_KOBJ_DEINITED;
    k_cancle_all_blocked_task(&sem->task_list_head);

    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_sem_resource_list_lock);
    os_list_del(&sem->resource_node);
    os_spin_unlock(&gs_os_sem_resource_list_lock);

    OS_KERNEL_FREE(sem);
    sem = OS_NULL;

    return OS_EOK;
}
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           This function decrements the semaphore's count. If the semaphore's count greater than 0, the function
 *                  decrements semaphore's count and returns immediately. Otherwise, the calling task blocks until either
 *                  the semaphore's count greater than 0 or waiting time expires.
 *
 * @param[in]       sem             The pointer to a semaphore.
 * @param[in]       timeout         Wait time (in clock ticks).
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_sem_wait(os_sem_t *sem, os_tick_t timeout)
{
    os_err_t    ret;
    os_task_t  *task;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT(OS_KOBJ_INITED == sem->object_inited);
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_active()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_disabled()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_schedule_locked()));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    ret = OS_EOK;
    OS_KERNEL_ENTER();

    if (sem->count > 0)
    {
        sem->count--;
        OS_KERNEL_EXIT();
    }
    else
    {
        if (OS_NO_WAIT == timeout)
        {
            OS_KERNEL_EXIT();
            ret = OS_EBUSY;
        }
        else
        {
            task = k_task_self();

            if (OS_SEM_WAKE_TYPE_PRIO == sem->wake_type)
            {
                k_block_task(&sem->task_list_head, task, timeout, OS_TRUE);
            }
            else
            {
                k_block_task(&sem->task_list_head, task, timeout, OS_FALSE);
            }

            OS_KERNEL_EXIT_SCHED();

            ret = task->switch_retval;
        }
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function increments the semaphore's count. If the semaphore's count becomes greater than 0. A
 *                  task blocked on it will be woken up.
 *
 * @param[in]       sem             The pointer to a semaphore.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_sem_post(os_sem_t *sem)
{
    os_err_t    ret;
    os_task_t  *task;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT(OS_KOBJ_INITED == sem->object_inited);

    ret = OS_EOK;
    OS_KERNEL_ENTER();

    if (os_list_empty(&sem->task_list_head))
    {
        if (sem->count < sem->max_count)
        {
            sem->count++;
            OS_KERNEL_EXIT();
        }
        else
        {
            OS_KERNEL_EXIT();

            ret = OS_EFULL;
        }
    }
    else
    {
        task = os_list_first_entry(&sem->task_list_head, os_task_t, task_node);
        k_unblock_task(task);
        if (task->state & OS_TASK_STATE_READY)
        {
            OS_KERNEL_EXIT_SCHED();
        }
        else
        {
            OS_KERNEL_EXIT();
        }

    }

    return ret;
}

os_err_t os_sem_set_wake_type(os_sem_t *sem, os_uint8_t wake_type)
{
    os_err_t  ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT((OS_SEM_WAKE_TYPE_PRIO == wake_type) || (OS_SEM_WAKE_TYPE_FIFO == wake_type));

    ret = OS_EBUSY;

    OS_KERNEL_ENTER();

    if (os_list_empty(&sem->task_list_head))
    {
        sem->wake_type = wake_type;
        ret            = OS_EOK;
    }

    OS_KERNEL_EXIT();

    return ret;
}

os_uint32_t os_sem_get_count(os_sem_t *sem)
{
    os_uint32_t    count;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT(OS_KOBJ_INITED == sem->object_inited);

    OS_KERNEL_ENTER();
    count = sem->count;
    OS_KERNEL_EXIT();

    return count;
}

os_uint32_t os_sem_get_max_count(os_sem_t *sem)
{
    OS_ASSERT(OS_NULL != sem);
    OS_ASSERT(OS_KOBJ_INITED == sem->object_inited);

    return sem->max_count;
}

#if defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP)
#include <shell.h>

#define SH_SHOW_TASK_CNT_MAX    5

typedef struct
{
    os_sem_t    *sem;
    os_uint32_t  count;

    os_uint16_t  block_task_count;
    os_task_t   *block_task[SH_SHOW_TASK_CNT_MAX];
}sh_sem_info_t;


/**
 ***********************************************************************************************************************
 * @brief           This function prints information about all the semaphore and it's corresponding blokced tasks.
 *
 * @param[in]
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t sh_show_sem_info(os_int32_t argc, char **argv)
{
    os_sem_t      *iter_sem;
    sh_sem_info_t *sem_info;
    os_uint16_t    sem_count;
    os_uint16_t    sem_index;
    char           name[OS_NAME_MAX + 1];
    os_uint16_t    len;
    os_int32_t     ret;

    OS_KERNEL_INIT();

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s %-10s %-10s %-10s\r\n", OS_NAME_MAX, "Semaphore", "Count", "Max Count", "Block Task");

    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    os_kprintf(" ---------- ---------- ----------\r\n");

    os_spin_lock(&gs_os_sem_resource_list_lock);
    sem_count = os_list_len(&gs_os_sem_resource_list_head);
    os_spin_unlock(&gs_os_sem_resource_list_lock);

    sem_info = OS_NULL;
    ret      = OS_EOK;

    if (0U != sem_count)
    {
        sem_info = (sh_sem_info_t *)os_malloc(sizeof(sh_sem_info_t) * sem_count);
        if (OS_NULL == sem_info)
        {
            ret = OS_ENOMEM;
        }
    }

    if ((OS_EOK == ret) && (OS_NULL != sem_info))
    {
        memset((void *)sem_info, 0, sizeof(sh_sem_info_t) * sem_count);

        os_spin_lock(&gs_os_sem_resource_list_lock);

        sem_index = 0;
        os_list_for_each_entry(iter_sem, &gs_os_sem_resource_list_head, os_sem_t, resource_node)
        {
            OS_KERNEL_ENTER();

            sem_info[sem_index].sem   = iter_sem;
            sem_info[sem_index].count = iter_sem->count;

            sem_info[sem_index].block_task_count = os_list_len(&iter_sem->task_list_head);
            k_get_blocked_task(&iter_sem->task_list_head, sem_info[sem_index].block_task, SH_SHOW_TASK_CNT_MAX);

            OS_KERNEL_EXIT();

            sem_index++;
            if (sem_index == sem_count)
            {
                break;
            }
        }

        os_spin_unlock(&gs_os_sem_resource_list_lock);

        if (sem_index < sem_count)
        {
            sem_count = sem_index;
        }

        for (sem_index = 0U; sem_index < sem_count; sem_index++)
        {
            if (OS_KOBJ_INITED != sem_info[sem_index].sem->object_inited)
            {
                continue;
            }

            strncpy(name, sem_info[sem_index].sem->name, OS_NAME_MAX);
            name[OS_NAME_MAX] = '\0';

            os_kprintf("%-*s %-10u %-10u %-10u:",
                       OS_NAME_MAX,
                       (name[0] != '\0') ? name : "-",
                       sem_info[sem_index].count,
                       sem_info[sem_index].sem->max_count,
                       sem_info[sem_index].block_task_count);

            k_show_blocked_task(sem_info[sem_index].block_task,
                                SH_SHOW_TASK_CNT_MAX,
                                sem_info[sem_index].block_task_count);
            os_kprintf("\r\n");
        }

        os_free(sem_info);
        sem_info = OS_NULL;
    }

    return ret;
}
SH_CMD_EXPORT(show_sem, sh_show_sem_info, "show semaphore information");

#endif /* defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP) */

#endif /* OS_USING_SEMAPHORE */


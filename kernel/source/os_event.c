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
 * @file        os_event.c
 *
 * @brief       This file implements the event functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-24   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_errno.h>
#include <os_event.h>
#include <arch_interrupt.h>
#include <string.h>
#include <os_spinlock.h>

#include "os_kernel_internal.h"

#ifdef OS_USING_EVENT

#define EVENT_TAG       "EVENT"

static os_list_node_t gs_os_event_resource_list_head = OS_LIST_INIT(gs_os_event_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_event_resource_list_lock);

OS_INLINE os_uint32_t _k_event_flag_check(os_event_t  *event,
                                          os_uint32_t  interested_set,
                                          os_uint32_t  option)
{
    os_uint32_t recved_set;

    recved_set = 0;

    if (option & OS_EVENT_OPTION_AND)
    {
        if ((event->set & interested_set) == interested_set)
        {
            recved_set = interested_set;
        }
    }
    else if (option & OS_EVENT_OPTION_OR)
    {
        if (event->set & interested_set)
        {
            recved_set = interested_set & event->set;
        }
    }
    else
    {
        OS_ASSERT_EX(0,"Check event option parameter error [%s]", k_task_self()->name);
    }

    if (0U != recved_set)
    {
        /* Received event. */
        if (option & OS_EVENT_OPTION_CLEAR)
        {
            event->set &= ~recved_set;
        }
    }

    return recved_set;
}

OS_INLINE void _k_event_init(os_event_t *event, const char *name, os_uint16_t object_alloc_type)
{
    os_list_init(&event->task_list_head);

    if (OS_NULL != name)
    {
        (void)strncpy(&event->name[0], name, OS_NAME_MAX);
        event->name[OS_NAME_MAX] = '\0';
    }
    else
    {
        event->name[0] = '\0';
    }

    event->set               = 0U;
    event->wake_type         = OS_EVENT_WAKE_TYPE_PRIO;
    event->object_alloc_type = object_alloc_type;
    event->object_inited     = OS_KOBJ_INITED;

    return;
}


/**
 ***********************************************************************************************************************
 * @brief           This function initlializes a event.
 *
 * @param[in]       event           The pointer to event.
 * @param[in]       name            The name of mutex.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_event_init(os_event_t *event, const char *name)
{
    os_event_t     *iter_event;
    os_list_node_t *pos;
    os_err_t        ret;

    OS_ASSERT(OS_NULL  != event);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    ret = OS_EOK;

    os_spin_lock(&gs_os_event_resource_list_lock);
    os_list_for_each(pos, &gs_os_event_resource_list_head)
    {
        iter_event = os_list_entry(pos, os_event_t, resource_node);
        if (iter_event == event)
        {
            OS_KERN_LOG(KERN_ERROR, EVENT_TAG,"The event(addr: %p, name: %s) has been exist", iter_event, iter_event->name);
            ret = OS_EINVAL;
            break;
        }
    }

    if (OS_EOK == ret)
    {
        os_list_add_tail(&gs_os_event_resource_list_head, &event->resource_node);
        os_spin_unlock(&gs_os_event_resource_list_lock);

        _k_event_init(event, name, OS_KOBJ_ALLOC_TYPE_STATIC);
    }
    else
    {
        os_spin_unlock(&gs_os_event_resource_list_lock);
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function deinitializes an event.
 *
 * @param[in]       event           The pointer to event.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_event_deinit(os_event_t *event)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != event);
    OS_ASSERT(OS_KOBJ_INITED == event->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_STATIC == event->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    OS_KERNEL_ENTER();

    event->object_inited = OS_KOBJ_DEINITED;
    event->set           = 0U;

    /* Wakeup all suspend tasks */
    k_cancle_all_blocked_task(&event->task_list_head);

    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_event_resource_list_lock);
    os_list_del(&event->resource_node);
    os_spin_unlock(&gs_os_event_resource_list_lock);

    return OS_EOK;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function will create an event object from heap.
 *
 * @param[in]       name            The name of mutex.
 *
 * @return          The pointer to the created event.
 * @retval          pointer         If operation successful.
 * @retval          OS_NULL         Error occurred.
 ***********************************************************************************************************************
 */
os_event_t *os_event_create(const char *name)
{
    os_event_t *event;

    /* Check context. */
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    event = (os_event_t *)OS_KERNEL_MALLOC(sizeof(os_event_t));
    if (OS_NULL == event)
    {
        OS_KERN_LOG(KERN_ERROR, EVENT_TAG, "Malloc event memory failed");
    }
    else
    {
        os_spin_lock(&gs_os_event_resource_list_lock);
        os_list_add_tail(&gs_os_event_resource_list_head, &event->resource_node);
        os_spin_unlock(&gs_os_event_resource_list_lock);

        _k_event_init(event, name, OS_KOBJ_ALLOC_TYPE_DYNAMIC);
    }
    return event;
}

/**
 ***********************************************************************************************************************
 * @brief           Destory an event created from heap.
 *
 * @param[in]       event           The event to destroy.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_event_destroy(os_event_t *event)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != event);
    OS_ASSERT(OS_KOBJ_INITED == event->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_DYNAMIC == event->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    OS_KERNEL_ENTER();

    event->object_inited = OS_KOBJ_DEINITED;
    event->set           = 0U;

    /* Wakeup all suspend tasks */
    k_cancle_all_blocked_task(&event->task_list_head);

    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_event_resource_list_lock);
    os_list_del(&event->resource_node);
    os_spin_unlock(&gs_os_event_resource_list_lock);

    OS_KERNEL_FREE(event);
    event = OS_NULL;

    return OS_EOK;
}

#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           This function sends an event to event object. If there are tasks blocked on the event object, tasks
 *                  interested this specific event will be woken up.
 *
 * @param[in]       event           The pointer to a event object.
 * @param[in]       set             The specific event set.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_event_send(os_event_t *event, os_uint32_t set)
{
    os_bool_t       need_schedule;
    os_uint32_t     recved_set_check;
    os_task_t      *block_task;
    os_task_t      *block_task_next;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != event);
    OS_ASSERT(OS_KOBJ_INITED == event->object_inited);
    OS_ASSERT(0U != set);

    need_schedule = OS_FALSE;

    OS_KERNEL_ENTER();

    event->set |= set;

    /* Search block task list and unblock suitable tasks. */
    os_list_for_each_entry_safe(block_task, block_task_next, &event->task_list_head, os_task_t, task_node)
    {
        recved_set_check = _k_event_flag_check(event, block_task->event_set, block_task->event_option);
        if (0U != recved_set_check)
        {
            block_task->event_set = recved_set_check;
            k_unblock_task(block_task);

            if (block_task->state & OS_TASK_STATE_READY)
            {
                need_schedule = OS_TRUE;
            }
        }

        if (0U == event->set)
        {
            break;
        }
    }

    if(OS_TRUE == need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function recieves an event from event object. If there is no interested event has been sent, the
 *                  calling task will be blocked until either interested event is sent by other task or waiting time expires.
 *
 * @param[in]       event           The pointer to a event object.
 * @param[in]       interested_set  The intereseted event set.
 * @param[in]       option          The option of OS_EVENT_OPTION_AND, OS_EVENT_OPTION_OR and OS_EVENT_OPTION_CLEAR.
 * @param[in]       timeout         Waiting time (in clock ticks).
 * @param[out]      recved_set      The actual recieved event set.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_event_recv(os_event_t  *event,
                       os_uint32_t  interested_set,
                       os_uint32_t  option,
                       os_tick_t    timeout,
                       os_uint32_t *recved_set)
{
    os_task_t  *current_task;
    os_uint32_t recved_set_check;
    os_err_t    ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != event);
    OS_ASSERT(OS_KOBJ_INITED == event->object_inited);
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_active()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_disabled()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_schedule_locked()));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));
    OS_ASSERT(0U != interested_set);

    ret = OS_EOK;

    /*Check parameter*/
    if (((option & (OS_EVENT_OPTION_OR | OS_EVENT_OPTION_AND)) == (OS_EVENT_OPTION_OR | OS_EVENT_OPTION_AND))
        || ((option & (OS_EVENT_OPTION_OR | OS_EVENT_OPTION_AND)) == 0))
    {
        OS_KERN_LOG(KERN_ERROR, EVENT_TAG, "Check event option parameter error [%s]", k_task_self()->name);
        ret = OS_EINVAL;
    }

    if (OS_EOK == ret)
    {
        OS_KERNEL_ENTER();

        recved_set_check = _k_event_flag_check(event, interested_set, option);
        if (0U != recved_set_check)
        {
            OS_KERNEL_EXIT();

            if (recved_set)
            {
                *recved_set = recved_set_check;
            }
        }
        else
        {
            if (OS_NO_WAIT == timeout)
            {
                OS_KERNEL_EXIT();
                ret = OS_EEMPTY;
            }
            else
            {
                current_task = k_task_self();
                current_task->event_set    = interested_set;
                current_task->event_option = option;

                /* Block current task */

                if (OS_EVENT_WAKE_TYPE_PRIO == event->wake_type)
                {
                    k_block_task(&event->task_list_head, current_task, timeout, OS_TRUE);
                }
                else
                {
                    k_block_task(&event->task_list_head, current_task, timeout, OS_FALSE);
                }

                OS_KERNEL_EXIT_SCHED();

                ret = current_task->switch_retval;

                if (OS_EOK == ret)
                {
                    if (recved_set)
                    {
                        *recved_set = current_task->event_set;
                    }
                }
                else if (OS_ETIMEOUT == ret)
                {
                    OS_KERNEL_ENTER();

                    recved_set_check = _k_event_flag_check(event, interested_set, option);

                    if (0U != recved_set_check)
                    {
                        if (recved_set)
                        {
                            *recved_set = recved_set_check;
                        }
                        ret = OS_EOK;
                    }
                    else
                    {
                        if (recved_set)
                        {
                            *recved_set = interested_set & event->set;
                        }
                    }

                    OS_KERNEL_EXIT();
                }
                else if (OS_ERROR == ret)
                {
                    if (recved_set)
                    {
                        *recved_set = interested_set & event->set;
                    }
                }
            }
        }
    }

    return ret;
}

os_err_t os_event_sync(os_event_t  *event,
                       os_uint32_t  set,
                       os_uint32_t  interested_set,
                       os_tick_t    timeout,
                       os_uint32_t *recved_set)
{
    os_task_t      *current_task;
    os_err_t        ret;
    os_uint32_t     recved_set_check;
    os_uint32_t     original_set;
    os_uint32_t     clear_bit;
    os_bool_t       need_schedule;
    os_task_t      *block_task;
    os_task_t      *block_task_next;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != event);
    OS_ASSERT(OS_KOBJ_INITED == event->object_inited);
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_active()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_disabled()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_schedule_locked()));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));
    OS_ASSERT(0U != interested_set);

    ret = OS_EOK;
    recved_set_check = 0;
    clear_bit = 0;
    need_schedule = OS_FALSE;

    OS_KERNEL_ENTER();

    event->set |= set;
    original_set = event->set;

    /* Search block task list and unblock suitable tasks. */
    os_list_for_each_entry_safe(block_task, block_task_next, &event->task_list_head, os_task_t, task_node)
    {
        if (block_task->event_option & OS_EVENT_OPTION_AND)
        {
            if ((event->set & block_task->event_set) == block_task->event_set)
            {
                recved_set_check = block_task->event_set;
            }
        }
        else if (block_task->event_option & OS_EVENT_OPTION_OR)
        {
            if (event->set & block_task->event_set)
            {
                recved_set_check = block_task->event_set & event->set;
            }
        }
        else
        {
            OS_ASSERT_EX(0,"Check event option parameter error [%s]", block_task->name);
        }

        if (0U != recved_set_check)
        {
            /* Received event. */
            if (block_task->event_option & OS_EVENT_OPTION_CLEAR)
            {
                /* store clear bit in temporary varialble. */
                clear_bit |= block_task->event_set;
            }

            block_task->event_set = recved_set_check;
            k_unblock_task(block_task);

            if (block_task->state & OS_TASK_STATE_READY)
            {
                need_schedule = OS_TRUE;
            }
        }
    }

    /* Now clear event bit. */
    event->set &= ~clear_bit;

    /* check if all bit has be set */
    if ((original_set & interested_set) == interested_set)
    {
        /* all bit has be set , always clear event here*/
        event->set &= ~interested_set;

        if (recved_set)
        {
            *recved_set = interested_set;
        }

        /* all bit has be set, there must be an unlocked task , schedule */
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        if (OS_NO_WAIT == timeout)
        {
            /* not all bit has be set. */
            if (recved_set)
            {
                *recved_set = interested_set & event->set;
            }

            ret = OS_ETIMEOUT;

            /* check if need schedule. */
            if (OS_TRUE == need_schedule)
            {
                OS_KERNEL_EXIT_SCHED();
            }
            else
            {
                OS_KERNEL_EXIT();
            }
        }
        else
        {
            current_task = k_task_self();
            current_task->event_set = interested_set;
            current_task->event_option = OS_EVENT_OPTION_AND | OS_EVENT_OPTION_CLEAR;

            /* Block current task */
            if (OS_EVENT_WAKE_TYPE_PRIO == event->wake_type)
            {
                k_block_task(&event->task_list_head, current_task, timeout, OS_TRUE);
            }
            else
            {
                k_block_task(&event->task_list_head, current_task, timeout, OS_FALSE);
            }

            OS_KERNEL_EXIT_SCHED();

            ret = current_task->switch_retval;

            if (OS_EOK == ret)
            {
                if (recved_set)
                {
                    *recved_set = current_task->event_set;
                }
            }
            else if (OS_ETIMEOUT == ret)
            {
                OS_KERNEL_ENTER();

                if (recved_set)
                {
                    *recved_set = interested_set & event->set;
                }

                if ((event->set & interested_set) == interested_set)
                {
                    event->set &= ~interested_set;

                    ret = OS_EOK;
                }

                OS_KERNEL_EXIT();
            }
            else if (OS_ERROR == ret)
            {
                if (recved_set)
                {
                    *recved_set = interested_set & event->set;
                }
            }
        }
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function clear event's mask.
 *
 * @param[in]       event             The pointer to a event object.
 * @param[in]       interested_clear  Clear the bits.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_event_clear(os_event_t *event, os_uint32_t interested_clear)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != event);
    OS_ASSERT(OS_KOBJ_INITED == event->object_inited);

    OS_KERNEL_ENTER();
    event->set &= ~interested_clear;
    OS_KERNEL_EXIT();

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function get event's value.
 *
 * @param[in]       event             The pointer to a event object.
 *
 * @return          The operation result.
 * @retval          event_set         Get the event value.
 ***********************************************************************************************************************
 */
os_int32_t os_event_get(os_event_t *event)
{
    OS_ASSERT(OS_NULL != event);
    OS_ASSERT(OS_KOBJ_INITED == event->object_inited);

    return event->set;
}

os_err_t os_event_set_wake_type(os_event_t *event, os_uint8_t wake_type)
{
    os_err_t  ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != event);
    OS_ASSERT((OS_EVENT_WAKE_TYPE_PRIO == wake_type) || (OS_EVENT_WAKE_TYPE_FIFO == wake_type));

    ret = OS_EBUSY;

    OS_KERNEL_ENTER();

    if (os_list_empty(&event->task_list_head))
    {
        event->wake_type = wake_type;
        ret              = OS_EOK;
    }

    OS_KERNEL_EXIT();

    return ret;
}

#if defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP)
#include <shell.h>

#define SH_SHOW_TASK_CNT_MAX    5

typedef struct
{
    os_event_t  *event;
    os_uint32_t  set;

    os_uint16_t  block_task_count;
    os_task_t   *block_task[SH_SHOW_TASK_CNT_MAX];
}sh_event_info_t;

/**
 ***********************************************************************************************************************
 * @brief           This function prints information about all the event and it's corresponding blokced tasks.
 *
 * @param[in]
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t sh_show_event_info(os_int32_t argc, char **argv)
{
    os_event_t      *iter_event;
    sh_event_info_t *event_info;
    os_uint16_t      event_count;
    os_uint16_t      event_index;
    char             name[OS_NAME_MAX + 1];
    os_uint16_t      len;
    os_int32_t       ret;

    OS_KERNEL_INIT();

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s %-10s %-10s\r\n", OS_NAME_MAX, "Event", "Set", "Block Task");

    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    os_kprintf(" ---------- ----------\r\n");

    os_spin_lock(&gs_os_event_resource_list_lock);
    event_count = os_list_len(&gs_os_event_resource_list_head);
    os_spin_unlock(&gs_os_event_resource_list_lock);

    event_info = OS_NULL;
    ret        = OS_EOK;

    if (0U != event_count)
    {
        event_info = (sh_event_info_t *)os_malloc(sizeof(sh_event_info_t) * event_count);
        if (OS_NULL == event_info)
        {
            ret = OS_ENOMEM;
        }
    }

    if ((OS_EOK == ret) && (OS_NULL != event_info))
    {
        memset((void *)event_info, 0, sizeof(sh_event_info_t) * event_count);

        os_spin_lock(&gs_os_event_resource_list_lock);

        event_index = 0;
        os_list_for_each_entry(iter_event, &gs_os_event_resource_list_head, os_event_t, resource_node)
        {
            OS_KERNEL_ENTER();

            event_info[event_index].event = iter_event;
            event_info[event_index].set   = iter_event->set;

            event_info[event_index].block_task_count = os_list_len(&iter_event->task_list_head);
            k_get_blocked_task(&iter_event->task_list_head, event_info[event_index].block_task, SH_SHOW_TASK_CNT_MAX);

            OS_KERNEL_EXIT();

            event_index++;
            if (event_index == event_count)
            {
                break;
            }
        }

        os_spin_unlock(&gs_os_event_resource_list_lock);

        if (event_index < event_count)
        {
            event_count = event_index;
        }

        for (event_index = 0U; event_index < event_count; event_index++)
        {
            if (OS_KOBJ_INITED != event_info[event_index].event->object_inited)
            {
                continue;
            }

            strncpy(name, event_info[event_index].event->name, OS_NAME_MAX);
            name[OS_NAME_MAX] = '\0';

            os_kprintf("%-*s 0x%-10x %-10u:",
                       OS_NAME_MAX,
                       (name[0] != '\0') ? name : "-",
                       event_info[event_index].set,
                       event_info[event_index].block_task_count);

            k_show_blocked_task(event_info[event_index].block_task,
                                SH_SHOW_TASK_CNT_MAX,
                                event_info[event_index].block_task_count);
            os_kprintf("\r\n");
        }

        os_free(event_info);
        event_info = OS_NULL;
    }

    return ret;
}
SH_CMD_EXPORT(show_event, sh_show_event_info, "show event information");

#endif /* defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP) */

#endif /* OS_USING_EVENT */


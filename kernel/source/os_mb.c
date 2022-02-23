/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        os_mb.c
 *
 * @brief       This file implements the mailbox functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-08   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_clock.h>
#include <arch_interrupt.h>
#include <os_mb.h>
#include <string.h>
#include <os_spinlock.h>

#include "os_kernel_internal.h"

#ifdef OS_USING_MAILBOX

#define MB_TAG      "MB"

static os_list_node_t gs_os_mb_resource_list_head = OS_LIST_INIT(gs_os_mb_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_mb_resource_list_lock);

static void _k_mb_modify_read_index(os_mb_t *mb)
{
    mb->read_index++;
    if (mb->read_index >= mb->capacity)
    {
        mb->read_index = 0U;
    }
    
    mb->entry_count--;

    return;
}

static void _k_mb_modify_write_index(os_mb_t *mb)
{
    mb->write_index++;
    if (mb->write_index >= mb->capacity)
    {
        mb->write_index = 0U;
    }
    
    mb->entry_count++;

    return;
}

static os_err_t _k_mb_init(os_mb_t     *mb,
                           const char  *name,
                           void        *mail_pool,
                           os_size_t    mail_pool_size,
                           os_uint16_t  object_alloc_type)
{
    void     *pool_align_begin;
    void     *pool_end;
    os_err_t  ret;

    ret = OS_EOK;

    /* The address of mail pool need to be aligned. */
    pool_align_begin = (void *)OS_ALIGN_UP((os_ubase_t)mail_pool, sizeof(os_ubase_t));
    pool_end         = (void *)((os_uint8_t *)mail_pool + mail_pool_size);
    mb->capacity     = (os_uint16_t)(((os_ubase_t)pool_end - (os_ubase_t)pool_align_begin) / sizeof(os_ubase_t));

    if (mb->capacity > 0U)
    {
        mb->mail_pool         = mail_pool;
        mb->mail_pool_align   = pool_align_begin;
        mb->entry_count       = 0U;
        mb->read_index        = 0U;
        mb->write_index       = 0U;
        mb->object_alloc_type = object_alloc_type;
        mb->wake_type         = OS_MB_WAKE_TYPE_PRIO;

        if (OS_NULL != name)
        {
            (void)strncpy(&mb->name[0], name, OS_NAME_MAX);
            mb->name[OS_NAME_MAX] = '\0';
        }
        else
        {
            mb->name[0] = '\0';
        }

        os_list_init(&mb->send_task_list_head);
        os_list_init(&mb->recv_task_list_head);
    }
    else
    {
        OS_KERN_LOG(KERN_ERROR, MB_TAG, "The count of calculated mail entry is less than 1.");
        ret = OS_ENOMEM;
    }
    
    return ret;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function creates a mailbox with dynamic memory allocation.
 *
 * @details         Both control block and mail pool of the mailbox are allocated in memory heap.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       name            Mailbox name.
 * @param[in]       max_mails       The maximum number of mails in this mailbox.
 * 
 * @return          The address of mailbox control block.
 * @retval          OS_NULL         Failed to create a mailbox with dynamic memory allocation.
 * @retval          else            Return the address of mailbox control block.
 ***********************************************************************************************************************
 */
os_mb_t *os_mb_create(const char *name, os_size_t max_mails)
{
    os_mb_t   *mb;
    void      *mail_pool;
    os_size_t  mail_pool_size;
    os_err_t   ret;

    OS_ASSERT(max_mails > 0U);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    ret = OS_EOK;
    
    mail_pool_size = max_mails * sizeof(os_ubase_t); 
    mb             = (os_mb_t *)OS_KERNEL_MALLOC(sizeof(os_mb_t));
    mail_pool      = OS_KERNEL_MALLOC_ALIGN(sizeof(os_ubase_t), mail_pool_size);
    
    if ((OS_NULL == mb) || (OS_NULL == mail_pool))
    {
        OS_KERN_LOG(KERN_ERROR, MB_TAG, "Malloc failed, mb(%p), mail_pool(%p)", mb, mail_pool);

        if (OS_NULL != mb)
        {
            OS_KERNEL_FREE(mb);
            mb = OS_NULL;
        }
        
        if (OS_NULL != mail_pool)
        {
            OS_KERNEL_FREE(mail_pool);
            mail_pool = OS_NULL;
        }

        ret = OS_ENOMEM;        
    }

    if (OS_EOK == ret)
    {
        ret = _k_mb_init(mb, name, mail_pool, mail_pool_size, OS_KOBJ_ALLOC_TYPE_DYNAMIC);
        if (OS_EOK == ret)
        {
            os_spin_lock(&gs_os_mb_resource_list_lock);
            os_list_add_tail(&gs_os_mb_resource_list_head, &mb->resource_node);
            os_spin_unlock(&gs_os_mb_resource_list_lock);
            
            mb->object_inited = OS_KOBJ_INITED;  
        }
        else
        {
            OS_KERNEL_FREE(mb);
            OS_KERNEL_FREE(mail_pool);
            
            mb        = OS_NULL;
            mail_pool = OS_NULL;
        }
    }

    return mb;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy a mailbox.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       mb              Mailbox control block.
 *
 * @return          The result of destroying the mailbox.
 * @retval          OS_EOK          Destroy the mailbox successfully.
 * @retval          else            Destroy the mailbox failed.
 ***********************************************************************************************************************
 */
os_err_t os_mb_destroy(os_mb_t *mb)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_DYNAMIC == mb->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    OS_KERNEL_ENTER();
    
    mb->object_inited = OS_KOBJ_DEINITED;
    
    k_cancle_all_blocked_task(&mb->send_task_list_head);
    k_cancle_all_blocked_task(&mb->recv_task_list_head);

    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_mb_resource_list_lock);
    os_list_del(&mb->resource_node);
    os_spin_unlock(&gs_os_mb_resource_list_lock);

    OS_KERNEL_FREE(mb->mail_pool);
    mb->mail_pool = OS_NULL;
    
    OS_KERNEL_FREE(mb);
    mb = OS_NULL;
    
    return OS_EOK;
}
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           This function initialize a mailbox with static memory allocation.
 *
 * @attention       This interface is not allowed in interrupt context.
 *
 * @param[in]       mb              Mailbox control block.
 * @param[in]       name            Mailbox name.
 * @param[in]       mail_pool       The address of mail pool.
 * @param[in]       mail_pool_size  The size of mail pool in bytes.
 *
 * @return          The result of initializing the mailbox.
 * @retval          OS_EOK          Initialize the mailbox successfully. 
 * @retval          else            Initialize the mailbox failed..
 ***********************************************************************************************************************
 */
os_err_t os_mb_init(os_mb_t *mb, const char *name, void *mail_pool, os_size_t mail_pool_size)
{
    os_mb_t        *iter_mb;
    os_list_node_t *pos;
    os_bool_t       exist;
    os_err_t        ret;

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_NULL != mail_pool);
    OS_ASSERT(mail_pool_size > 0);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    exist = OS_FALSE;
    ret   = OS_EOK;

    os_spin_lock(&gs_os_mb_resource_list_lock);
    os_list_for_each(pos, &gs_os_mb_resource_list_head)
    {
        iter_mb = os_list_entry(pos, os_mb_t, resource_node);
        if (iter_mb == mb)
        {
            OS_KERN_LOG(KERN_ERROR, MB_TAG, "The mb(addr: %p, name: %s) has been exist", iter_mb, iter_mb->name);

            exist = OS_TRUE;
            ret   = OS_EINVAL;
            break;
        }
    }

    if (OS_FALSE == exist)
    {
        os_list_add_tail(&gs_os_mb_resource_list_head, &mb->resource_node);
        os_spin_unlock(&gs_os_mb_resource_list_lock);;

        ret = _k_mb_init(mb, name, mail_pool, mail_pool_size, OS_KOBJ_ALLOC_TYPE_STATIC);
        if (OS_EOK == ret)
        {
            mb->object_inited = OS_KOBJ_INITED;
        }
        else
        {
            os_spin_lock(&gs_os_mb_resource_list_lock);
            os_list_del(&mb->resource_node);
            os_spin_unlock(&gs_os_mb_resource_list_lock);
        }
    }
    else
    {
        os_spin_unlock(&gs_os_mb_resource_list_lock);    
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Deinitialize a mailbox.
 *
 * @attention       This interface is not allowed in interrupt context.
 *
 * @param[in]       mb              Mailbox control block.
 *
 * @return          The result of deinitializing the mailbox.
 * @retval          OS_EOK          Deinitialize the mailbox successfully.
 * @retval          else            Deinitialize the mailbox failed.
 ***********************************************************************************************************************
 */
os_err_t os_mb_deinit(os_mb_t *mb)
{
    OS_KERNEL_INIT();
    
    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_STATIC == mb->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    OS_KERNEL_ENTER();
    
    mb->object_inited = OS_KOBJ_DEINITED;
    
    k_cancle_all_blocked_task(&mb->send_task_list_head);
    k_cancle_all_blocked_task(&mb->recv_task_list_head);

    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_mb_resource_list_lock);
    os_list_del(&mb->resource_node);
    os_spin_unlock(&gs_os_mb_resource_list_lock);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will send a mail to mailbox.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context and timeout is not OS_NO_WAIT.
 *                      2. Interrupt is disabled and timeout is not OS_NO_WAIT.
 *                      3. Scheduler is locked and timeout is not OS_NO_WAIT.
 *
 * @param[in]       mb              Mailbox control block.
 * @param[in]       value           The mail to send. This parameter is used to transfer address of data memory block.
 * @param[in]       timeout         The timeout. This parameter can be the following value:
 *                                      1. OS_NO_WAIT, if the mailbox is full, return immediately.
 *                                      2. OS_WAIT_FOREVER, if the mailbox is full, wait until mailbox is not full.
 *                                      3. Else, if the mailbox is full, wait until mailbox is not full or timeout. 
 * 
 * @return          The result of sending a mail.
 * @retval          OS_EOK          Send a mail successfully.
 * @retval          else            Send a mail failed.
 ***********************************************************************************************************************
 */
os_err_t os_mb_send(os_mb_t *mb, os_ubase_t value, os_tick_t timeout)
{
    os_task_t *current_task;
    os_task_t *block_task;
    os_bool_t  need_schedule;
    os_err_t   ret;

    OS_KERNEL_INIT();
    
    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);
    OS_ASSERT((OS_FALSE == os_is_irq_active()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    ret           = OS_EOK;
    need_schedule = OS_FALSE;
    
    OS_KERNEL_ENTER();
    if (!os_list_empty(&mb->recv_task_list_head))
    {
        /* 
         * If the receiving task is blocked, use swap_data of blocked task to transfer mail.
         * So, when the receiving task is awakened, there is no need to get the mail from 
         * the mail pool of mailbox. 
         */
        block_task = os_list_first_entry(&mb->recv_task_list_head, os_task_t, task_node); 
        k_unblock_task(block_task);
        block_task->swap_data = value;

        if (block_task->state & OS_TASK_STATE_READY)
        {
            need_schedule = OS_TRUE;
        }
    }
    else
    {
        if (mb->entry_count == mb->capacity)
        {
            if (OS_NO_WAIT == timeout)
            {
                ret = OS_EFULL;
            }
            else
            {
                /* 
                 * If the mailbox is full, use swap_data of current task to store the mail.
                 * When the receiving task receive a mail, it will put this mail into mail
                 * pool of mailbox. 
                 */
                current_task = k_task_self();
                current_task->swap_data = value;

                if (OS_MB_WAKE_TYPE_PRIO == mb->wake_type)
                {
                    k_block_task(&mb->send_task_list_head, current_task, timeout, OS_TRUE);
                }
                else
                {
                    k_block_task(&mb->send_task_list_head, current_task, timeout, OS_FALSE);
                }
                
                OS_KERNEL_EXIT_SCHED();

                /* 
                 * If timeout, mailbox is destroyed or other abnormal wake-up, ret is not OS_EOK
                 * and send mail failed. Otherwise, the mail has been put into mail pool of mailbox
                 * by the receiving task.
                 */
                ret = current_task->switch_retval;

                OS_KERNEL_ENTER();
            }
        }
        else
        {
            /* 
             * The mailbox is not full and there is no blocked task, put the mail info the mail 
             * pool of mailbox
             */
            *((os_ubase_t *)mb->mail_pool_align + mb->write_index) = value;
             _k_mb_modify_write_index(mb);
        }
    }

    if (OS_TRUE == need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();   
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will receive a mail from mailbox.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context and timeout is not OS_NO_WAIT.
 *                      2. Interrupt is disabled and timeout is not OS_NO_WAIT.
 *                      3. Scheduler is locked and timeout is not OS_NO_WAIT.
 *
 * @param[in]       mb              Mailbox control block.
 * @param[in]       value           The mail to receive.
 * @param[in]       timeout         The timeout. This parameter can be the following value:
 *                                      1. OS_NO_WAIT, if the mailbox is empty, return immediately.
 *                                      2. OS_WAIT_FOREVER, if the mailbox is empty, wait until mailbox is not empty.
 *                                      3. Else, if the mailbox is empty, wait until mailbox is not empty or timeout. 
 * 
 * @return          The result of receiving a mail.
 * @retval          OS_EOK          Receive a mail successfully.
 * @retval          else            Receive a mail failed.
 ***********************************************************************************************************************
 */
os_err_t os_mb_recv(os_mb_t *mb, os_ubase_t *value, os_tick_t timeout)
{
    os_task_t *current_task;
    os_task_t *block_task;
    os_bool_t  need_schedule;
    os_bool_t  modify_read_index;
    os_err_t   ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_NULL != value);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);
    OS_ASSERT((OS_FALSE == os_is_irq_active()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));
    
    ret               = OS_EOK;
    need_schedule     = OS_FALSE;
    modify_read_index = OS_TRUE;
    
    OS_KERNEL_ENTER();

    if (0U != mb->entry_count)
    {
        *value = *((os_ubase_t *)mb->mail_pool_align + mb->read_index);    
    }
    else
    {
        if (OS_NO_WAIT == timeout)
        {
            ret = OS_EEMPTY;
        }
        else
        {
            current_task = k_task_self();
            if (OS_MB_WAKE_TYPE_PRIO == mb->wake_type)
            {
                k_block_task(&mb->recv_task_list_head, current_task, timeout, OS_TRUE);
            }
            else
            {
                k_block_task(&mb->recv_task_list_head, current_task, timeout, OS_FALSE);
            }
            
            OS_KERNEL_EXIT_SCHED();

            OS_KERNEL_ENTER();
            
            ret = current_task->switch_retval;
            if (OS_EOK == ret)
            {
                /* 
                 * When the sender wakes up this task, it puts the data on the swap_data,
                 * so there's no need to get data from the mail pool of mailbox.
                 */
                *value = current_task->swap_data;
                modify_read_index = OS_FALSE;
            }
        }
    }

    if ((OS_EOK == ret) && (OS_TRUE == modify_read_index))
    {
        _k_mb_modify_read_index(mb);
    
        if (!os_list_empty(&mb->send_task_list_head))
        {
            block_task = os_list_first_entry(&mb->send_task_list_head, os_task_t, task_node); 
            k_unblock_task(block_task);

            /* 
             * when the sender task was blocked, it put the data on the swap_data, so receiver get mail
             * directly from swap data.
             *
             * In addition, the receiver has obtained a mail at this time, so it is necessary to put the
             * mail info the mail pool of mailbox.
             */
            *((os_ubase_t *)mb->mail_pool_align + mb->write_index) = block_task->swap_data;
            _k_mb_modify_write_index(mb);

            if (block_task->state & OS_TASK_STATE_READY)
            {
                need_schedule = OS_TRUE;
            }
        }
    }

    if (OS_TRUE == need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();   
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return ret;
}

os_err_t os_mb_set_wake_type(os_mb_t *mb, os_uint8_t wake_type)
{
    os_err_t  ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT((OS_MB_WAKE_TYPE_PRIO == wake_type) || (OS_MB_WAKE_TYPE_FIFO == wake_type));

    ret = OS_EBUSY;

    OS_KERNEL_ENTER();

    if (os_list_empty(&mb->recv_task_list_head) && os_list_empty(&mb->send_task_list_head))
    {
        mb->wake_type = wake_type;
        ret           = OS_EOK;
    }

    OS_KERNEL_EXIT();

    return ret;
}

void os_mb_reset(os_mb_t *mb)
{
    os_bool_t need_schedule;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);

    need_schedule = OS_FALSE;

    OS_KERNEL_ENTER();

    mb->read_index  = 0U;
    mb->write_index = 0U;
    mb->entry_count = 0U;

    if (!os_list_empty(&mb->send_task_list_head))
    {
        k_cancle_all_blocked_task(&mb->send_task_list_head);
        need_schedule = OS_TRUE;
    }

    if (!os_list_empty(&mb->recv_task_list_head))
    {
        k_cancle_all_blocked_task(&mb->recv_task_list_head);
        need_schedule = OS_TRUE;
    }
    
    if (OS_TRUE == need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();   
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return;
}

os_bool_t os_mb_is_empty(os_mb_t *mb)
{
    os_bool_t is_empty;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);

    OS_KERNEL_ENTER();
    if (0U == mb->entry_count)
    {
        is_empty = OS_TRUE;
    }
    else
    {
        is_empty = OS_FALSE;
    }
    OS_KERNEL_EXIT();

    return is_empty;
}

os_bool_t os_mb_is_full(os_mb_t *mb)
{
    os_bool_t is_full;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);

    OS_KERNEL_ENTER();
    if (mb->entry_count == mb->capacity)
    {
        is_full = OS_TRUE;
    }
    else
    {
        is_full = OS_FALSE;
    }
    OS_KERNEL_EXIT();

    return is_full;
}

os_uint16_t os_mb_get_capacity(os_mb_t *mb)
{
    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);

    return mb->capacity;
}

os_uint16_t os_mb_get_used_entry_count(os_mb_t *mb)
{
    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);

    return mb->entry_count;
}

os_uint16_t os_mb_get_unused_entry_count(os_mb_t *mb)
{
    os_uint16_t unused_entry_cnt;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mb);
    OS_ASSERT(OS_KOBJ_INITED == mb->object_inited);

    OS_KERNEL_ENTER();
    unused_entry_cnt = mb->capacity - mb->entry_count;
    OS_KERNEL_EXIT();

    return unused_entry_cnt;
}

#if defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP)
#include <shell.h>

#define SH_SHOW_TASK_CNT_MAX    5

typedef struct
{
    os_mb_t     *mb;
    os_uint16_t  entry_count;

    os_uint16_t  recv_block_task_count;
    os_uint16_t  send_block_task_count;
    
    os_task_t   *recv_block_task[SH_SHOW_TASK_CNT_MAX];
    os_task_t   *send_block_task[SH_SHOW_TASK_CNT_MAX];  
}sh_mb_info_t;

static os_err_t sh_show_mb_info(os_int32_t argc, char **argv)
{
    os_mb_t      *iter_mb;
    sh_mb_info_t *mb_info;
    os_uint16_t   mb_count;
    os_uint16_t   mb_index;
    os_uint16_t   len;
    char          name[OS_NAME_MAX + 1];
    os_int32_t    ret;

    OS_KERNEL_INIT();

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s %-10s %-13s %-10s\r\n", OS_NAME_MAX, "Mailbox", "Mail Count", "Mailbox depth", "Block Task");

    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    
    os_kprintf(" ---------- ------------- ----------\r\n");


    os_spin_lock(&gs_os_mb_resource_list_lock);
    mb_count = os_list_len(&gs_os_mb_resource_list_head);
    os_spin_unlock(&gs_os_mb_resource_list_lock);

    mb_info = OS_NULL;
    ret     = OS_EOK;

    if (0U != mb_count)
    {
        mb_info = (sh_mb_info_t *)os_malloc(sizeof(sh_mb_info_t) * mb_count);
        if (OS_NULL == mb_info)
        {
            ret = OS_ENOMEM;
        }
    }

    if ((OS_EOK == ret) && (OS_NULL != mb_info))
    {
        memset((void *)mb_info, 0, sizeof(sh_mb_info_t) * mb_count);
    
        os_spin_lock(&gs_os_mb_resource_list_lock);

        mb_index = 0;
        os_list_for_each_entry(iter_mb, &gs_os_mb_resource_list_head, os_mb_t, resource_node)
        {
            OS_KERNEL_ENTER();
            
            mb_info[mb_index].mb          = iter_mb;
            mb_info[mb_index].entry_count = iter_mb->entry_count;

            mb_info[mb_index].recv_block_task_count = os_list_len(&iter_mb->recv_task_list_head);
            k_get_blocked_task(&iter_mb->recv_task_list_head, mb_info[mb_index].recv_block_task, SH_SHOW_TASK_CNT_MAX);

            mb_info[mb_index].send_block_task_count = os_list_len(&iter_mb->send_task_list_head);
            k_get_blocked_task(&iter_mb->send_task_list_head, mb_info[mb_index].send_block_task, SH_SHOW_TASK_CNT_MAX);

            OS_KERNEL_EXIT();

            mb_index++;
            if (mb_index == mb_count)
            {
                break;
            }
        }
        
        os_spin_unlock(&gs_os_mb_resource_list_lock);

        if (mb_index < mb_count)
        {
            mb_count = mb_index;
        }

        for (mb_index = 0U; mb_index < mb_count; mb_index++)
        {
            if (OS_KOBJ_INITED != mb_info[mb_index].mb->object_inited)
            {
                continue;
            }

            strncpy(name, mb_info[mb_index].mb->name, OS_NAME_MAX);
            name[OS_NAME_MAX] = '\0';

            os_kprintf("%-*s     %-12u %-11u %-6u(send):",
                       OS_NAME_MAX,
                       (name[0] != '\0') ? name : "-",
                       mb_info[mb_index].entry_count,
                       mb_info[mb_index].mb->capacity,
                       mb_info[mb_index].send_block_task_count);
            k_show_blocked_task(mb_info[mb_index].send_block_task,
                                SH_SHOW_TASK_CNT_MAX,
                                mb_info[mb_index].send_block_task_count);
            os_kprintf("\r\n");


            os_kprintf("%-*s     %-12s %-11s %-6u(recv):",
                       OS_NAME_MAX,
                       "",
                       "",
                       "",
                       mb_info[mb_index].recv_block_task_count);         
            k_show_blocked_task(mb_info[mb_index].recv_block_task,
                                SH_SHOW_TASK_CNT_MAX,
                                mb_info[mb_index].recv_block_task_count);
            os_kprintf("\r\n");
        }

        os_free(mb_info);
        mb_info = OS_NULL;
    }

    return ret;
}
SH_CMD_EXPORT(show_mb, sh_show_mb_info, "Show mailbox information");

#endif /* defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP) */

#endif /* end of OS_USING_MAILBOX */


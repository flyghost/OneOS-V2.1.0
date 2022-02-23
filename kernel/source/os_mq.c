/**
 ***********************************************************************************************************************
 * Copyright (c) 2020-2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        os_mq.c
 *
 * @brief       This file implements the message queue functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-14   OneOS Team      First Version
 * 2021-01-08   OneOS team      Refactor message queue implementation.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_clock.h>
#include <arch_interrupt.h>
#include <os_mq.h>
#include <os_spinlock.h>
#include <string.h>

#include "os_kernel_internal.h"

#ifdef OS_USING_MESSAGEQUEUE

#define MQ_TAG              "MQ"

static os_list_node_t gs_os_mq_resource_list_head = OS_LIST_INIT(gs_os_mq_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_mq_resource_list_lock);

OS_INLINE void _k_mq_put_msg_to_queue(os_mq_t *mq, os_mq_msg_t *msg, os_bool_t urgent)
{
    if (OS_FALSE == urgent)
    {
        msg->next = OS_NULL;

        if (mq->msg_queue_tail != OS_NULL)
        {
            mq->msg_queue_tail->next = msg;
        }

        /* Set new tail */
        mq->msg_queue_tail = msg;

        if (OS_NULL == mq->msg_queue_head)
        {
            mq->msg_queue_head = msg;
        }
    }
    else
    {
        msg->next = mq->msg_queue_head;
        mq->msg_queue_head = msg;

        if (OS_NULL == mq->msg_queue_tail)
        {
            mq->msg_queue_tail = msg;
        }
    }
    
    mq->entry_count++;

    return;
}

OS_INLINE os_mq_msg_t * _k_mq_get_msg_from_queue(os_mq_t *mq)
{
    os_mq_msg_t *msg;

    OS_ASSERT(OS_NULL != mq->msg_queue_head);

    msg                = mq->msg_queue_head;
    mq->msg_queue_head = msg->next;
    msg->next          = OS_NULL;

    if (mq->msg_queue_tail == msg)
    {
        mq->msg_queue_tail = OS_NULL;
    }

    mq->entry_count--;

    return msg;
}

OS_INLINE void _k_mq_release_free_msg(os_mq_t *mq, os_mq_msg_t *msg)
{
    msg->next          = mq->msg_queue_free;
    mq->msg_queue_free = msg;

    return;
}

OS_INLINE os_mq_msg_t * _k_mq_get_free_msg(os_mq_t *mq)
{
    os_mq_msg_t *msg;

    OS_ASSERT(OS_NULL != mq->msg_queue_free);

    msg                = mq->msg_queue_free;
    mq->msg_queue_free = msg->next;
    msg->next          = OS_NULL;

    return msg;
}

static os_err_t _k_mq_init(os_mq_t    *mq,
                           const char *name,
                           void       *msg_pool,
                           os_size_t   msg_pool_size,
                           os_size_t   msg_size,
                           os_uint16_t object_alloc_type)
{
    void        *pool_align_begin;
    void        *pool_end;
    os_size_t    pool_align_size;
    os_mq_msg_t *msg;
    os_size_t    msg_align_size;
    os_size_t    msg_res_size;
    os_uint16_t  index;
    os_err_t     ret;

    ret = OS_EOK;
    
    msg_align_size   = OS_ALIGN_UP(msg_size, OS_ALIGN_SIZE);
    msg_res_size     = msg_align_size + sizeof(os_mq_msg_hdr_t);

    pool_align_begin = (void *)OS_ALIGN_UP((os_ubase_t)msg_pool, OS_ALIGN_SIZE);
    pool_end         = (void *)((os_uint8_t *)msg_pool + msg_pool_size);
    pool_align_size  = (os_ubase_t)pool_end - (os_ubase_t)pool_align_begin;
    
    mq->queue_depth   = (os_uint16_t)(pool_align_size / msg_res_size);
                   
    if (mq->queue_depth > 0)
    {
        mq->msg_pool          = msg_pool;
        mq->max_msg_size      = msg_align_size;
        mq->entry_count       = 0U;
        mq->object_alloc_type = object_alloc_type;
        mq->wake_type         = OS_MQ_WAKE_TYPE_PRIO;

        if (OS_NULL != name)
        {
            (void)strncpy(&mq->name[0], name, OS_NAME_MAX);
            mq->name[OS_NAME_MAX] = '\0';
        }
        else
        {
            mq->name[0] = '\0';
        }

        os_list_init(&mq->send_task_list_head);
        os_list_init(&mq->recv_task_list_head);

        mq->msg_queue_head = OS_NULL;
        mq->msg_queue_tail = OS_NULL;
        mq->msg_queue_free = OS_NULL;

        for (index = 0U; index < mq->queue_depth; index++)
        {
            msg = (os_mq_msg_t *)((os_uint8_t *)pool_align_begin + index * msg_res_size);
            msg->msg_len       = 0U;
            msg->next          = mq->msg_queue_free;
            mq->msg_queue_free = msg;
        }
    }
    else
    {
        OS_KERN_LOG(KERN_ERROR, MQ_TAG, "The count of calculated message entry is less than 1.");
        ret = OS_ENOMEM;
    }
    
    return ret;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function creates a message queue with dynamic memory allocation.
 *
 * @details         Both control block and message pool of the message queue are allocated in memory heap.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       name            Message queue name.
 * @param[in]       msg_size        The size of a message.
 * @param[in]       max_msgs        The maximum number of messages in this message queue.
 * 
 * @return          The address of message queue's control block.
 * @retval          OS_NULL         Failed to create a message queue with dynamic memory allocation.
 * @retval          else            Return the address of message queue's control block.
 ***********************************************************************************************************************
 */
os_mq_t *os_mq_create(const char *name, os_size_t msg_size, os_size_t max_msgs)
{
    os_mq_t   *mq;
    void      *msg_pool;
    os_size_t  msg_pool_size;
    os_size_t  align_msg_size;
    os_err_t   ret;

    OS_ASSERT(msg_size > 0U);
    OS_ASSERT(max_msgs > 0U);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    ret = OS_EOK;

    align_msg_size = OS_ALIGN_UP(msg_size, OS_ALIGN_SIZE);
    msg_pool_size  = max_msgs * (align_msg_size + sizeof(os_mq_msg_hdr_t));
    mq       = (os_mq_t *)OS_KERNEL_MALLOC(sizeof(os_mq_t));
    msg_pool = OS_KERNEL_MALLOC_ALIGN(OS_ALIGN_SIZE, msg_pool_size);
    
    if ((OS_NULL == mq) || (OS_NULL == msg_pool))
    {
        OS_KERN_LOG(KERN_ERROR, MQ_TAG, "Malloc failed, mq(%p), msg_pool(%p)", mq, msg_pool);

        if (OS_NULL != mq)
        {
            OS_KERNEL_FREE(mq);
            mq = OS_NULL;
        }
        
        if (OS_NULL != msg_pool)
        {
            OS_KERNEL_FREE(msg_pool);
            msg_pool = OS_NULL;
        }

        ret = OS_ENOMEM;
    }

    if (OS_EOK == ret)
    {
        ret = _k_mq_init(mq, name, msg_pool, msg_pool_size, align_msg_size, OS_KOBJ_ALLOC_TYPE_DYNAMIC);
        if (OS_EOK == ret)
        {
            os_spin_lock(&gs_os_mq_resource_list_lock);
            os_list_add_tail(&gs_os_mq_resource_list_head, &mq->resource_node);
            os_spin_unlock(&gs_os_mq_resource_list_lock);

            mq->object_inited = OS_KOBJ_INITED;  
        }
        else
        {
            OS_KERNEL_FREE(mq);
            OS_KERNEL_FREE(msg_pool);
            
            mq       = OS_NULL;
            msg_pool = OS_NULL;
        }
    }

    return mq;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy a message queue.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       mq              Message queue's control block.
 *
 * @return          The result of destroying the message queue.
 * @retval          OS_EOK          Destroy the message queue successfully.
 * @retval          else            Destroy the message queue failed.
 ***********************************************************************************************************************
 */
os_err_t os_mq_destroy(os_mq_t *mq)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_DYNAMIC == mq->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());
    
    OS_KERNEL_ENTER();
    
    mq->object_inited = OS_KOBJ_DEINITED;
    k_cancle_all_blocked_task(&mq->send_task_list_head);
    k_cancle_all_blocked_task(&mq->recv_task_list_head);
    
    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_mq_resource_list_lock);
    os_list_del(&mq->resource_node);
    os_spin_unlock(&gs_os_mq_resource_list_lock);

    OS_KERNEL_FREE(mq->msg_pool);
    mq->msg_pool = OS_NULL;
    
    OS_KERNEL_FREE(mq);
    mq = OS_NULL;

    return OS_EOK;
}
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           This function initialize a message queue with static memory allocation.
 *
 * @attention       This interface is not allowed in interrupt context.
 *
 * @param[in]       mq              Message queue's control block.
 * @param[in]       name            Message queue name.
 * @param[in]       msg_pool        The address of message queue's pool.
 * @param[in]       msg_pool_size   The size of message queue's pool in bytes.
 * @param[in]       msg_size        The size of a message.
 *
 * @return          The result of initializing the message queue.
 * @retval          OS_EOK          Initialize the message queue successfully. 
 * @retval          else            Initialize the message queue failed.
 ***********************************************************************************************************************
 */
os_err_t os_mq_init(os_mq_t        *mq,
                    const char     *name,
                    void           *msg_pool,
                    os_size_t       msg_pool_size,
                    os_size_t       msg_size)
{
    os_mq_t        *iter_mq;
    os_list_node_t *pos;
    os_bool_t       exist;
    os_err_t        ret;

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_NULL != msg_pool);
    OS_ASSERT(msg_pool_size > 0U);
    OS_ASSERT(msg_size > 0U);
    OS_ASSERT(os_is_irq_active() == OS_FALSE);

    exist = OS_FALSE;
    ret   = OS_EOK;

    os_spin_lock(&gs_os_mq_resource_list_lock);
    os_list_for_each(pos, &gs_os_mq_resource_list_head)
    {
        iter_mq = os_list_entry(pos, os_mq_t, resource_node);
        if (iter_mq == mq)
        {
            OS_KERN_LOG(KERN_ERROR, MQ_TAG, "The mq(addr: %p, name: %s) has been exist", iter_mq, iter_mq->name);

            exist = OS_TRUE;
            ret   = OS_EINVAL;
            break;
        }
    }

    if (OS_FALSE == exist)
    {
        os_list_add_tail(&gs_os_mq_resource_list_head, &mq->resource_node);
        os_spin_unlock(&gs_os_mq_resource_list_lock);

        ret = _k_mq_init(mq, name, msg_pool, msg_pool_size, msg_size, OS_KOBJ_ALLOC_TYPE_STATIC);
        if (OS_EOK == ret)
        {
            mq->object_inited = OS_KOBJ_INITED;
        }
        else
        {
            os_spin_lock(&gs_os_mq_resource_list_lock);
            os_list_del(&mq->resource_node);
            os_spin_unlock(&gs_os_mq_resource_list_lock);  
        }
    }
    else
    {
        os_spin_unlock(&gs_os_mq_resource_list_lock);    
    }
      
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Deinitialize a message queue.
 *
 * @attention       This interface is not allowed in interrupt context.
 *
 * @param[in]       mq              Message queue's control block.
 *
 * @return          The result of deinitializing the message queue.
 * @retval          OS_EOK          Deinitialize the message queue successfully.
 * @retval          else            Deinitialize the message queue failed.
 ***********************************************************************************************************************
 */
os_err_t os_mq_deinit(os_mq_t *mq)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_STATIC == mq->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    OS_KERNEL_ENTER();
    
    mq->object_inited = OS_KOBJ_DEINITED;
    k_cancle_all_blocked_task(&mq->send_task_list_head);
    k_cancle_all_blocked_task(&mq->recv_task_list_head);
    
    OS_KERNEL_EXIT_SCHED();

    os_spin_lock(&gs_os_mq_resource_list_lock);
    os_list_del(&mq->resource_node);
    os_spin_unlock(&gs_os_mq_resource_list_lock);

    return OS_EOK;
}

static os_err_t _k_mq_send(os_mq_t *mq, void *buffer, os_size_t buff_size, os_tick_t timeout, os_bool_t urgent)
{
    os_task_t   *current_task;
    os_task_t   *block_task;
    os_mq_msg_t *msg;
    os_err_t     ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_NULL != buffer);
    OS_ASSERT((buff_size > 0U) && (buff_size <= mq->max_msg_size));
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);
    OS_ASSERT((OS_FALSE == os_is_irq_active()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    ret = OS_EOK;

    OS_KERNEL_ENTER();

    if (OS_NULL != mq->msg_queue_free)
    {
        msg = _k_mq_get_free_msg(mq);
        OS_KERNEL_EXIT();
        
        (void)memcpy((void*)((os_uint8_t *)msg + sizeof(os_mq_msg_hdr_t)), buffer, buff_size);
        msg->msg_len = buff_size;
    }
    else
    {
        if (OS_NO_WAIT == timeout)
        {
            OS_KERNEL_EXIT();
            ret = OS_EFULL;
        }
        else
        {
            current_task = k_task_self();

            if (OS_MQ_WAKE_TYPE_PRIO == mq->wake_type)
            { 
                k_block_task(&mq->send_task_list_head, current_task, timeout, OS_TRUE);
            }
            else
            {
                k_block_task(&mq->send_task_list_head, current_task, timeout, OS_FALSE);
            }
            
            OS_KERNEL_EXIT_SCHED();

            ret = current_task->switch_retval;
            if (OS_EOK == ret)
            {
                /* 
                 * swap_data field filled by the receiver is point to free message buffer,
                 * so put the sending message into this buffer.
                 */
                msg = (os_mq_msg_t *)current_task->swap_data;
                (void)memcpy((void*)((os_uint8_t *)msg + sizeof(os_mq_msg_hdr_t)), buffer, buff_size);
                msg->msg_len = buff_size; 
            }
        }
    }

    if (OS_EOK == ret)
    {
        OS_KERNEL_ENTER();
        
        if (!os_list_empty(&mq->recv_task_list_head))
        {
            /* 
             * When the receiver task has been blocked, put address of the message buffer filled data on swap_data,
             * and unblock the receiver task. The receiver task will get message from swap_data.
             */
            block_task = os_list_first_entry(&mq->recv_task_list_head, os_task_t, task_node);
            k_unblock_task(block_task);

            block_task->swap_data = (os_ubase_t)msg;

            if (block_task->state & OS_TASK_STATE_READY)
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
            _k_mq_put_msg_to_queue(mq, msg, urgent);
            OS_KERNEL_EXIT();    
        }
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will send a message to message queue.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context and timeout is not OS_NO_WAIT.
 *                      2. Interrupt is disabled and timeout is not OS_NO_WAIT.
 *                      3. Scheduler is locked and timeout is not OS_NO_WAIT.
 *
 * @param[in]       mq              Message queue's control block.
 * @param[in]       buffer          The message buffer to be send.
 * @param[in]       buff_size       The size of the message to be send.
 * @param[in]       timeout         The timeout. This parameter can be the following value:
 *                                      1. OS_NO_WAIT, if the message queue is full, return immediately.
 *                                      2. OS_WAIT_FOREVER, if the message queue is full, wait until mesage queue
 *                                         is not full.
 *                                      3. Else, if the message queue is full, wait until message queue is not
 *                                         full or timeout. 
 * 
 * @return          The result of sending a message.
 * @retval          OS_EOK          Send a message successfully.
 * @retval          else            Send a message failed.
 ***********************************************************************************************************************
 */
os_err_t os_mq_send(os_mq_t *mq, void *buffer, os_size_t buff_size, os_tick_t timeout)
{
    os_err_t ret;

    ret = _k_mq_send(mq, buffer, buff_size, timeout, OS_FALSE);

    return ret;
}

os_err_t os_mq_send_urgent(os_mq_t *mq, void *buffer, os_size_t buff_size, os_tick_t timeout)
{
    os_err_t ret;

    ret = _k_mq_send(mq, buffer, buff_size, timeout, OS_TRUE);

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will receive a message from message queue.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context and timeout is not OS_NO_WAIT.
 *                      2. Interrupt is disabled and timeout is not OS_NO_WAIT.
 *                      3. Scheduler is locked and timeout is not OS_NO_WAIT.
 *
 * @param[in]       mq              Message queue's control block.
 * @param[in]       buffer          The receive buffer is provided by the caller.
 * @param[in]       buff_size       The size of the receive buffer.
 * @param[in]       timeout         The timeout. This parameter can be the following value:
 *                                      1. OS_NO_WAIT, if the message queue is empty, return immediately.
 *                                      2. OS_WAIT_FOREVER, if the message queue is empty, wait until message queue is
 *                                         not empty.
 *                                      3. Else, if the message queue is empty, wait until message queue is not empty
 *                                         or timeout.
 * @param[in]       recv_msg_size   The size of the received message.
 * 
 * @return          The result of receiving a message.
 * @retval          OS_EOK          Receive a message successfully.
 * @retval          else            Receive a message failed.
 ***********************************************************************************************************************
 */
os_err_t os_mq_recv(os_mq_t    *mq,
                    void       *buffer,
                    os_size_t   buff_size,
                    os_tick_t   timeout,
                    os_size_t  *recv_msg_size)
{
    os_task_t   *current_task;
    os_task_t   *block_task;
    os_mq_msg_t *msg;
    os_err_t     ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_NULL != buffer);
    OS_ASSERT(buff_size > 0U);    
    OS_ASSERT(OS_NULL != recv_msg_size);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);
    OS_ASSERT((OS_FALSE == os_is_irq_active()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    ret = OS_EOK;

    OS_KERNEL_ENTER();

    current_task = k_task_self();

    if (OS_NULL == mq->msg_queue_head)
    {
        if (OS_NO_WAIT == timeout)
        {
            OS_KERNEL_EXIT();
            ret = OS_EEMPTY;
        }
        else
        {
            if (OS_MQ_WAKE_TYPE_PRIO == mq->wake_type)
            {
                k_block_task(&mq->recv_task_list_head, current_task, timeout, OS_TRUE);
            }
            else
            {
                k_block_task(&mq->recv_task_list_head, current_task, timeout, OS_FALSE);
            }
            
            OS_KERNEL_EXIT_SCHED();

            ret = current_task->switch_retval;
            if (OS_EOK == ret)
            {
                /* 
                 * When the sender wakes up this task, it puts address of the message buffer on the swap_data,
                 * so there's no need to get message from the queue.
                 */
                msg = (os_mq_msg_t *)current_task->swap_data;
            }
        }
    }
    else
    {
        msg = _k_mq_get_msg_from_queue(mq); 
        OS_KERNEL_EXIT();
    }

    if (OS_EOK == ret)
    {
        if (buff_size < msg->msg_len)
        {
            /* When receive buffer size is less than message size, discard this message. */
            OS_KERN_LOG(KERN_ERROR, MQ_TAG, "Recv buff size(%lu) of task(%s) is less than msg size(%lu)",
                    buff_size,
                    current_task->name,  
                    msg->msg_len);        
            ret = OS_ENOMEM;
        }
        else
        {
            (void)memcpy(buffer, (void*)((os_uint8_t *)msg + sizeof(os_mq_msg_hdr_t)), msg->msg_len);
            *recv_msg_size = msg->msg_len;     
        }

        OS_KERNEL_ENTER();
        if (!os_list_empty(&mq->send_task_list_head))
        {
            /* 
             * When the sender task was blocked, the message buffer that has just been processed will
             * not be released into free pool. Instead, this message buffer is assigned to the sender task. 
             */
            block_task = os_list_first_entry(&mq->send_task_list_head, os_task_t, task_node);
            k_unblock_task(block_task);
            block_task->swap_data = (os_ubase_t)msg;

            if (block_task->state & OS_TASK_STATE_READY)
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
            _k_mq_release_free_msg(mq, msg);
            OS_KERNEL_EXIT();
        }
    }
        
    return ret;
}

os_err_t os_mq_set_wake_type(os_mq_t *mq, os_uint8_t wake_type)
{
    os_err_t  ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT((OS_MQ_WAKE_TYPE_PRIO == wake_type) || (OS_MQ_WAKE_TYPE_FIFO == wake_type));

    ret = OS_EBUSY;

    OS_KERNEL_ENTER();

    if (os_list_empty(&mq->recv_task_list_head) && os_list_empty(&mq->send_task_list_head))
    {
        mq->wake_type = wake_type;
        ret           = OS_EOK;
    }

    OS_KERNEL_EXIT();

    return ret;
}

void os_mq_reset(os_mq_t *mq)
{
    os_mq_msg_t *msg;
    os_bool_t    need_schedule;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);

    need_schedule = OS_FALSE;

    OS_KERNEL_ENTER();

    while (1)
    {
        if (OS_NULL == mq->msg_queue_head)
        {
            break;
        }

        msg = _k_mq_get_msg_from_queue(mq);
        _k_mq_release_free_msg(mq, msg);
    }

    if (!os_list_empty(&mq->send_task_list_head))
    {
        k_cancle_all_blocked_task(&mq->send_task_list_head);
        need_schedule = OS_TRUE;
    }

    if (!os_list_empty(&mq->recv_task_list_head))
    {
        k_cancle_all_blocked_task(&mq->recv_task_list_head);
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

os_bool_t os_mq_is_empty(os_mq_t *mq)
{
    os_bool_t is_empty;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);

    OS_KERNEL_ENTER();
    if (OS_NULL == mq->msg_queue_head)
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

os_bool_t os_mq_is_full(os_mq_t *mq)
{
    os_bool_t is_full;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);

    OS_KERNEL_ENTER();
    if (OS_NULL == mq->msg_queue_free)
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

os_uint16_t os_mq_get_queue_depth(os_mq_t *mq)
{
    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);

    return mq->queue_depth;
}

os_uint16_t os_mq_get_used_entry_count(os_mq_t *mq)
{
    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);

    return mq->entry_count;
}

os_uint16_t os_mq_get_unused_entry_count(os_mq_t *mq)
{
    os_uint16_t unused_entry_cnt;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mq);
    OS_ASSERT(OS_KOBJ_INITED == mq->object_inited);

    OS_KERNEL_ENTER();
    unused_entry_cnt = mq->queue_depth - mq->entry_count;
    OS_KERNEL_EXIT();

    return unused_entry_cnt;
}

#if defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP)
#include <shell.h>

#define SH_SHOW_TASK_CNT_MAX    8

typedef struct
{
    os_mq_t     *mq;
    os_uint16_t  entry_count;

    os_uint16_t  recv_block_task_count;
    os_uint16_t  send_block_task_count;
    
    os_task_t   *recv_block_task[SH_SHOW_TASK_CNT_MAX];
    os_task_t   *send_block_task[SH_SHOW_TASK_CNT_MAX];  
}sh_mq_info_t;

static os_err_t sh_show_mq_info(os_int32_t argc, char **argv)
{
    os_mq_t      *iter_mq;
    sh_mq_info_t *mq_info;
    os_uint16_t   mq_count;
    os_uint16_t   mq_index;
    os_uint16_t   len;
    char          name[OS_NAME_MAX + 1];
    os_int32_t    ret;

    OS_KERNEL_INIT();

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s %-13s %-11s %-10s\r\n",
               OS_NAME_MAX,
               "Message queue",
               "Message Count",
               "Queue depth",
               "Block Task");

    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    os_kprintf(" ------------- ----------- ----------\r\n");

    os_spin_lock(&gs_os_mq_resource_list_lock);
    mq_count = os_list_len(&gs_os_mq_resource_list_head);
    os_spin_unlock(&gs_os_mq_resource_list_lock);

    mq_info = OS_NULL;
    ret     = OS_EOK;

    if (0U != mq_count)
    {
        mq_info = (sh_mq_info_t *)os_malloc(sizeof(sh_mq_info_t) * mq_count);
        if (OS_NULL == mq_info)
        {
            ret = OS_ENOMEM;
        }
    }

    if ((OS_EOK == ret) && (OS_NULL != mq_info))
    {
        memset((void *)mq_info, 0, sizeof(sh_mq_info_t) * mq_count);
    
        os_spin_lock(&gs_os_mq_resource_list_lock);

        mq_index = 0;
        os_list_for_each_entry(iter_mq, &gs_os_mq_resource_list_head, os_mq_t, resource_node)
        {
            OS_KERNEL_ENTER();
            
            mq_info[mq_index].mq          = iter_mq;
            mq_info[mq_index].entry_count = iter_mq->entry_count;

            mq_info[mq_index].recv_block_task_count = os_list_len(&iter_mq->recv_task_list_head);
            k_get_blocked_task(&iter_mq->recv_task_list_head, mq_info[mq_index].recv_block_task, SH_SHOW_TASK_CNT_MAX);

            mq_info[mq_index].send_block_task_count = os_list_len(&iter_mq->send_task_list_head);
            k_get_blocked_task(&iter_mq->send_task_list_head, mq_info[mq_index].send_block_task, SH_SHOW_TASK_CNT_MAX);

            OS_KERNEL_EXIT();

            mq_index++;
            if (mq_index == mq_count)
            {
                break;
            }
        }
        
        os_spin_unlock(&gs_os_mq_resource_list_lock);

        if (mq_index < mq_count)
        {
            mq_count = mq_index;
        }

        for (mq_index = 0U; mq_index < mq_count; mq_index++)
        {
            if (OS_KOBJ_INITED != mq_info[mq_index].mq->object_inited)
            {
                continue;
            }

            strncpy(name, mq_info[mq_index].mq->name, OS_NAME_MAX);
            name[OS_NAME_MAX] = '\0';

            os_kprintf("%-*s       %-12u %-10u %-6u(send):",
                       OS_NAME_MAX,
                       (name[0] != '\0') ? name : "-",
                       mq_info[mq_index].entry_count,
                       mq_info[mq_index].mq->queue_depth,
                       mq_info[mq_index].send_block_task_count);

            k_show_blocked_task(mq_info[mq_index].send_block_task,
                                SH_SHOW_TASK_CNT_MAX,
                                mq_info[mq_index].send_block_task_count);
            os_kprintf("\r\n");

            os_kprintf("%-*s       %-12s %-10s %-6u(recv):",
                       OS_NAME_MAX,
                       "",
                       "",
                       "",
                       mq_info[mq_index].recv_block_task_count);
                       
            k_show_blocked_task(mq_info[mq_index].recv_block_task,
                                SH_SHOW_TASK_CNT_MAX,
                                mq_info[mq_index].recv_block_task_count);
            os_kprintf("\r\n");
        }

        os_free(mq_info);
        mq_info = OS_NULL;
    }

    return ret;
}
SH_CMD_EXPORT(show_mq, sh_show_mq_info, "Show message queue information");

#endif /* defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP) */

#endif /* end of OS_USING_MESSAGEQUEUE */


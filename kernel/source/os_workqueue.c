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
 * @file        os_workqueue.c
 *
 * @brief       This function implements workqueue mechanism.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_stddef.h>
#include "os_kernel_internal.h"
#include <os_workqueue.h>

#ifdef OS_USING_WORKQUEUE

/* Workqueue depends on semaphore. */
#ifndef OS_USING_SEMAPHORE
#error "Make sure semaphore is enable"
#endif

/* Log tag of workqueue */
#define WORKQ_TAG       "WORKQ"

static void _k_workqueue_task_entry(void *parameter)
{    
    struct os_workqueue  *queue;
    struct os_work       *work;
    const os_list_node_t       *head;
    os_ubase_t            irq_key;

    queue = (os_workqueue_t  *)parameter;
    head = &queue->work_list_head;

    while (1)
    {
        (void)os_sem_wait(&queue->sem,OS_WAIT_FOREVER);
        
        while (1)
        {
            os_spin_lock_irqsave(&queue->lock, &irq_key);  
            if (!os_list_empty(head))
            {
                work = os_list_first_entry(head, os_work_t, work_node);
                os_list_del(&work->work_node);
                queue->work_current = work;
                work->flag = OS_WORK_STAGE_IDLE;
                os_spin_unlock_irqrestore(&queue->lock, irq_key); 
                /*done work*/
                work->func(work->data);
            }
            else
            {
                queue->work_current = OS_NULL;
                os_spin_unlock_irqrestore(&queue->lock, irq_key);
                break;
            }
           
        }

    }


}


OS_INLINE void _k_submit_work_to_queue_tail(os_workqueue_t *queue ,os_work_t *work)
{
    work->flag = OS_WORK_STAGE_PENDING;
    os_list_add_tail(&queue->work_list_head,&work->work_node);
    /*Optimization: reduce bin semaphore release*/
    if (OS_NULL == queue->work_current)
    {
        (void)os_sem_post(&queue->sem);
    }  
}

OS_INLINE void _k_submit_work_to_queue_head(os_workqueue_t *queue ,os_work_t *work)
{
    work->flag = OS_WORK_STAGE_PENDING;
    os_list_add(&queue->work_list_head,&work->work_node);

    /*Optimization: reduce bin semaphore release*/
    if (OS_NULL == queue->work_current)
    {
        (void)os_sem_post(&queue->sem);
    }  
}

static void _k_work_timeout(void *parameter)
{
    os_ubase_t      irq_key;
    struct os_work *work;
    os_workqueue_t *queue;
    
    work = (struct os_work *)parameter;
    queue = work->workqueue;

    os_spin_lock_irqsave(&queue->lock, &irq_key);
    /*If the timer can't cancel,can be cancelled by os_cancel_work used work->flag */
    if (OS_WORK_STAGE_DELAY == work->flag)
    {
        _k_submit_work_to_queue_tail(work->workqueue,work);
    }
    os_spin_unlock_irqrestore(&queue->lock, irq_key); 
}

/**
 ***********************************************************************************************************************
 * @brief           Initialize work.
 *
 * @attention       Before using work, must initialize work.
 *
 * @param[in]       work            The work.
 * @param[in]       func            Callback function for work.
 * @param[in]       data            Private data of work.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_work_init(os_work_t *work, void (*func)(void *data), void *data)
{
    OS_ASSERT(work);
    OS_ASSERT(func);

    work->func = func;
    work->data = data;
    work->workqueue = OS_NULL;
    work->flag = OS_WORK_STAGE_IDLE;
    work->object_inited = OS_KOBJ_INITED;
    
    os_list_init(&work->work_node);
    (void)os_timer_init(&work->timer,
                        OS_NULL,
                        _k_work_timeout,
                        work,
                        1,   
                        OS_TIMER_FLAG_ONE_SHOT);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will deinitialize the work.
 *
 * @attention       Before deinitialize work, must initialize work.
 *
 * @param[in]       work            The work.
 
 * @retval          None.
 ***********************************************************************************************************************
 */
void os_work_deinit(os_work_t *work)
{
    OS_ASSERT(OS_NULL != work);
    OS_ASSERT(OS_KOBJ_INITED == work->object_inited);

    work->object_inited = OS_KOBJ_DEINITED;
    
    os_list_del(&work->work_node);

    os_timer_deinit(&work->timer);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Submit a work to the system workqueue.
 *
 * @details         The work does not resubmit if it is in a queue(OS_WORK_STAGE_PENDING) or in a timeout commit
 *                  (OS_WORK_STAGE_DELAY).If you want to recommit work immediately, call first os_cancel_work() cancel the 
 *                  last submission.
 *
 * @param[in]       work            The work.
 * @param[in]       delay_time      Delay time. Commit immediately if equals 0.If it is greater than 0, the work will be
 *                                  delayed commit.
 *
 * @return          Submit work result.
 * @retval          OS_EOK          Submit work success.
 * @retval          OS_EBUSY        The work has been submitted.
 ***********************************************************************************************************************
 */
os_err_t os_submit_work_to_queue(os_workqueue_t *queue, os_work_t *work, os_tick_t delay_time)
{
    os_ubase_t irq_key;
    os_err_t   ret;

    OS_ASSERT(queue);
    OS_ASSERT(work);
    OS_ASSERT(OS_KOBJ_INITED == queue->object_inited);
    OS_ASSERT(OS_KOBJ_INITED == work->object_inited);
    OS_ASSERT(delay_time < (OS_TICK_MAX / 2));

    ret = OS_EOK;

    os_spin_lock_irqsave(&queue->lock, &irq_key); 
    /*Not allowing a work to commit to more than one queue.Limiting concurrent running*/
    if (work->workqueue && (queue != work->workqueue))
    {
        os_spin_unlock_irqrestore(&queue->lock, irq_key);
        
        OS_KERN_LOG(KERN_WARNING, WORKQ_TAG, "Submitting work to more than one workqueue is not allowed");
        ret = OS_ERROR;
    }
    else
    {
        if (work->flag == OS_WORK_STAGE_IDLE)
        {
            work->workqueue = queue;
            if (delay_time)
            {
                work->flag = OS_WORK_STAGE_DELAY;
                (void)os_timer_set_timeout_ticks(&work->timer, delay_time);
                (void)os_timer_start(&work->timer);
            }
            else
            {
                _k_submit_work_to_queue_tail(queue,work);
            }
        
            ret = OS_EOK;
        }
        else
        {
            OS_KERN_LOG(KERN_WARNING, WORKQ_TAG, "Work(%p) had been submitted to workqueue(%p)", work, queue);
            ret = OS_EBUSY;
        }  
        os_spin_unlock_irqrestore(&queue->lock, irq_key);

    }

    return ret;  
}

static void _k_wait_work(void *parameter)
{
    (void)os_sem_post((os_sem_t *)parameter);
}

static os_err_t _k_cancel_work(os_work_t *work, os_bool_t sync)
{
    os_ubase_t key;
    os_err_t ret;
    os_workqueue_t *queue;
    os_work_t wait_work;
    os_sem_t  wait_sem;
    os_int32_t work_runing;
    
    OS_ASSERT(work);
    OS_ASSERT(OS_KOBJ_INITED == work->object_inited);

    ret = OS_EOK;
    work_runing = 0;
    
    /*The work has not been committed since it was initialized.*/
    if(OS_NULL == work->workqueue)
    {
        ret = OS_ERROR;
    }
    else
    {
        queue = work->workqueue;
         if (sync)
         {
            (void)os_sem_init(&wait_sem, OS_NULL, 0, 1);
            os_work_init(&wait_work, _k_wait_work,&wait_sem);
         }
        
         os_spin_lock_irqsave(&queue->lock, &key);  
        
         if (OS_WORK_STAGE_DELAY == work->flag)
         {
             (void)os_timer_stop(&work->timer);
             work->flag = OS_WORK_STAGE_IDLE;
         }
         else if (OS_WORK_STAGE_PENDING == work->flag)
         {
             os_list_del(&work->work_node);
             work->flag = OS_WORK_STAGE_IDLE;
         }
         else
         { 
             /*Work is in progress*/
              if (queue->work_current == work)
              {
                  work_runing = 1;
                  if (sync)
                  {
                      _k_submit_work_to_queue_head(work->workqueue, &wait_work); 
                  }
                  else
                  {
                      OS_KERN_LOG(KERN_WARNING, WORKQ_TAG, "Work(%p) is running", work);
                      ret = OS_EBUSY;
                  }
              }
         }
                
         os_spin_unlock_irqrestore(&queue->lock, key); 
         
         if (sync)
         {    
            if (work_runing)
            {
                (void)os_sem_wait(&wait_sem, OS_WAIT_FOREVER);
            }
            (void)os_sem_deinit(&wait_sem);
            os_work_deinit(&wait_work);
         }
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Cancel the work that has been committed(in a queue (OS_WORK_STAGE_DELAY) or in a timeout commit 
 *                  (OS_WORK_STAGE_PENDING).
 *
 * @attention       Work cannot be cancelled while it is in progress, and return OS_EBUSY.
 *
 * @param[in]       work            The work.
 *
 * @return          Cancel work result.
 * @retval          OS_EOK          Cancel work success.
 * @retval          OS_EBUSY        The work is in progress.
 * @retval          OS_ERROR        The work has not been committed since it was initialized.
 ***********************************************************************************************************************
 */
os_err_t os_cancel_work(os_work_t *work)
{
    return _k_cancel_work(work, 0);
}

/**
 ***********************************************************************************************************************
 * @brief           Cancel the work that has been committed(in a queue (OS_WORK_STAGE_DELAY) or in a timeout commit 
 *                  (OS_WORK_STAGE_PENDING).
 *
 * @attention       If the work is being executed, the task block until the work is completed.
 *
 * @param[in]       work            The work.
 *
 * @return          Cancel work result.
 * @retval          OS_EOK          Cancel work success. 
 * @retval          OS_ERROR        The work has not been committed since it was initialized.
 ***********************************************************************************************************************
 */
os_err_t os_cancel_work_sync(os_work_t *work)
{
    return _k_cancel_work(work,1);
}

/**
 ***********************************************************************************************************************
 * @brief           This function initializes a workqueue.
 *
 * @param[in]       queue           The descriptor of wrokqueue
 * @param[in]       name            Pointer to task name string
 * @param[in]       stack_begin     Pointer to start of stack
 * @param[in]       stack_size      Stack size in bytes
 * @param[in]       priority        Priority of task
 * @param[in]       cpu_index       The workqueue task want bind to that CPU(CPU affinity).-1:no CPU affinity.
 *
 * @return          On success, return OS_EOK; on error, return a error code.
 * @retval          OS_EOK          Successfully initialized a workqueue.
 * @retval          OS_ERROR        Initialization a workqueue failure.
 ***********************************************************************************************************************
 */     
os_err_t os_workqueue_init(os_workqueue_t *queue,
                           const char     *name, 
                           void           *stack_begin,
                           os_uint32_t     stack_size,
                           os_uint8_t      priority,
                           os_int32_t      cpu_index)
{
    os_err_t ret;

    OS_ASSERT(queue != OS_NULL);
    OS_ASSERT(stack_begin != OS_NULL);
    OS_ASSERT(stack_size > 0);
    OS_ASSERT(priority < OS_TASK_PRIORITY_MAX);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    OS_UNREFERENCE(cpu_index);

    queue->work_current = OS_NULL;
    queue->object_inited = OS_KOBJ_INITED;
    os_list_init(&queue->work_list_head);
    os_spin_lock_init(&queue->lock);
    
    /*binary semaphore*/
    ret = os_sem_init(&queue->sem, OS_NULL, 0, 1);
    if (ret == OS_EOK)
    {
        ret = os_task_init(&queue->worker_task,
                   name,
                   _k_workqueue_task_entry,
                   queue,
                   stack_begin,
                   stack_size,
                   priority);
        
        if (ret == OS_EOK)
        {
            ret = os_task_startup(&queue->worker_task);
        }
    }

    return ret;
    
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function dynamic creates a workqueue.
 *
 * @param[in]       name            The name of task in workqueue.
 * @param[in]       stack_size      The stack size of task in workqueue.
 * @param[in]       priority        The priority of task in workqueue.
 * @param[in]       cpu_index       The workqueue task want bind to that CPU(CPU affinity).-1:no CPU affinity.
 *
 * @return          The created workqueue pointer.
 * @retval          OS_NULL         Create workqueue failed.
 * @retval          else            Create workqueue success.
 ***********************************************************************************************************************
 */                                
os_workqueue_t *os_workqueue_create(const char  *name,
                                    os_uint32_t  stack_size, 
                                    os_uint8_t   priority, 
                                    os_int32_t   cpu_index)
{
    os_workqueue_t *queue;
    void           *stack_begin;
    os_err_t        ret;
    
    OS_ASSERT(stack_size > 0);
    OS_ASSERT(priority < OS_TASK_PRIORITY_MAX);
    OS_ASSERT(cpu_index >= -1);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    ret = OS_EOK;
    
    queue = (os_workqueue_t *)OS_KERNEL_MALLOC(sizeof(os_workqueue_t));
    if (OS_NULL == queue)
    {
        OS_KERN_LOG(KERN_ERROR, WORKQ_TAG, "Malloc workqueue(%s) failed", name);
        ret = OS_ENOMEM;
    }
    else
    {
        
        stack_size = OS_ALIGN_UP(stack_size,OS_ARCH_STACK_ALIGN_SIZE);
        stack_begin = OS_KERNEL_MALLOC_ALIGN(OS_ARCH_STACK_ALIGN_SIZE,stack_size);
        
        if (OS_NULL == stack_begin)
        {
            OS_KERN_LOG(KERN_ERROR, WORKQ_TAG, "Malloc stack_begin(%s) failed", name);
            ret = OS_ENOMEM;
        }
        else
        {
            ret = os_workqueue_init(queue,
                                    name,
                                    stack_begin,
                                    stack_size,
                                    priority,
                                    cpu_index);
        }

    }

    if(OS_EOK == ret)
    {
        return queue;
    }
    else
    {
        if (queue != OS_NULL)
        {
            OS_KERNEL_FREE(queue);
        }
        if (stack_begin != OS_NULL)
        {
            OS_KERNEL_FREE(stack_begin);
        }

        return OS_NULL;
    }
                         
}
#endif /* OS_USING_SYS_HEAP */

#ifdef OS_USING_SYSTEM_WORKQUEUE

static OS_TASK_STACK_DEFINE(gs_sys_worker_task_stack, OS_SYSTEM_WORKQUEUE_STACK_SIZE);
static os_workqueue_t gs_sys_workq;

/**
 ***********************************************************************************************************************
 * @brief           Initialize system workqueue.
 *
 * @param           None.
 *
 * @return          Initialize system workqueue result.
 * @retval          OS_EOK          Initialize system workqueue success.
 * @retval          OS_ERROR        Initialize system workqueue failed.
 ***********************************************************************************************************************
 */
static os_err_t _k_work_sys_workqueue_init(void)
{
    os_err_t ret;
    
    ret = os_workqueue_init(&gs_sys_workq,
                            "sys_work",
                            OS_TASK_STACK_BEGIN_ADDR(gs_sys_worker_task_stack),
                            OS_TASK_STACK_SIZE(gs_sys_worker_task_stack),
                            OS_SYSTEM_WORKQUEUE_PRIORITY,
                            -1);


    if(ret != OS_EOK)
    {
        OS_KERN_LOG(KERN_ERROR, WORKQ_TAG, "Create system workqueue failed");
    } 
    
    return ret;  
}

/**
 ***********************************************************************************************************************
 * @brief           Submit a work to the system workqueue.
 *
 * @details         When the delay_time is greater than 0, the work will be delayed.
 *
 * @param[in]       work            The work.
 * @param[in]       delay_time      Delay time. If it is greater than 0, the work will be delayed.
 *
 * @return          Submit work result.
 * @retval          OS_EOK          Submit work success.
 * @retval          OS_EBUSY        The work has been submitted.
 ***********************************************************************************************************************
 */
os_err_t os_submit_work(os_work_t *work, os_tick_t delay_time)
{
    OS_ASSERT(OS_NULL != work);
    OS_ASSERT(delay_time < (OS_TICK_MAX / 2));
    
    return os_submit_work_to_queue(&gs_sys_workq, work, delay_time);
}
OS_PREV_INIT(_k_work_sys_workqueue_init, OS_INIT_SUBLEVEL_HIGH);

#endif /* OS_USING_SYSTEM_WORKQUEUE */

#endif /* OS_USING_WORKQUEUE */


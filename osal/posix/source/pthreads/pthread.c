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
 * @file        pthread.c
 *
 * @brief       This file provides posix pthread functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <arch_interrupt.h>
#include <pthread.h>
#include <sched.h>
#include "pthread_internal.h"

OS_DEFINE_SPINLOCK(pth_lock);

static _pthread_data_t *gs_pthread_table[PTHREAD_NUM_MAX] = {NULL};

static _pthread_data_t *_pthread_get_data(pthread_t thread)
{
    os_ubase_t irq_key;
    _pthread_data_t *ptd;

    if (thread >= PTHREAD_NUM_MAX)
    {
        return NULL;
    }

    os_spin_lock_irqsave(&pth_lock, &irq_key);
    ptd = gs_pthread_table[thread];
    os_spin_unlock_irqrestore(&pth_lock, irq_key);

    if (ptd && (PTHREAD_MAGIC == ptd->magic))
    {
        return ptd;
    }

    return NULL;
}

static pthread_t _pthread_data_get_pth(_pthread_data_t *ptd)
{
    int index;
    os_ubase_t irq_key;

    os_spin_lock_irqsave(&pth_lock, &irq_key);
    
    for (index = 0; index < PTHREAD_NUM_MAX; index++)
    {
        if (gs_pthread_table[index] == ptd)
        {
            break;
        }
    }
    
    os_spin_unlock_irqrestore(&pth_lock, irq_key);

    return index;
}

static pthread_t _pthread_data_create(void)
{
    int index;
    _pthread_data_t *ptd;
    os_ubase_t irq_key;

    ptd = (_pthread_data_t *)os_malloc(sizeof(_pthread_data_t));
    if (!ptd)
    {
        return PTHREAD_NUM_MAX;
    }

    memset(ptd, 0x0, sizeof(_pthread_data_t));
    ptd->canceled    = 0;
    ptd->cancelstate = PTHREAD_CANCEL_DISABLE;
    ptd->canceltype  = PTHREAD_CANCEL_DEFERRED;
    ptd->magic       = PTHREAD_MAGIC;

    os_spin_lock_irqsave(&pth_lock, &irq_key);
    
    for (index = 0; index < PTHREAD_NUM_MAX; index ++)
    {
        if (NULL == gs_pthread_table[index])
        {
            gs_pthread_table[index] = ptd;
            break;
        }
    }
    
    os_spin_unlock_irqrestore(&pth_lock, irq_key);

    /* Full of pthreads, clean magic and release ptd. */
    if (PTHREAD_NUM_MAX == index)
    {
        ptd->magic = 0x0;
        os_free(ptd);
    }

    return index;
}

static void _pthread_data_destroy(pthread_t pth)
{
    os_ubase_t irq_key;

    _pthread_data_t *ptd = _pthread_get_data(pth);
    
    if (ptd)
    {
        /* Remove from pthread table. */
        os_spin_lock_irqsave(&pth_lock, &irq_key);
        
        gs_pthread_table[pth] = NULL;
        
        os_spin_unlock_irqrestore(&pth_lock, irq_key);

        /* Destroy joinable semaphore */
        if (OS_NULL != ptd->joinable_sem)
        {
            os_sem_destroy(ptd->joinable_sem);
        }

        /* Release thread resource. */
        if ((OS_NULL == ptd->attr.stackaddr) && (OS_NULL != ptd->tid->stack_begin))
        {
            /* Release thread allocated stack. */
            os_free(ptd->tid->stack_begin);
        }
        
        /* Clean stack addr pointer. */
        ptd->tid->stack_begin = OS_NULL;

        /* If this thread create the local thread data,delete it. */
        if (OS_NULL != ptd->tls)
        {
            os_free(ptd->tls);
        }
        
        os_free(ptd->tid);

        /* Clean magic. */
        ptd->magic = 0x0;

        os_free(ptd);
    }
}

static void _pthread_destroy(_pthread_data_t *ptd)
{
    pthread_t pth = _pthread_data_get_pth(ptd);
    
    if (PTHREAD_NUM_MAX != pth)
    {
        _pthread_data_destroy(pth);
    }

    return;
}

static void _pthread_cleanup(void *user_data)
{
    _pthread_data_t *ptd;

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)user_data;
    OS_ASSERT(OS_NULL != ptd);

    /* Clear cleanup function. */
    ptd->tid->cleanup = OS_NULL;
    
    if (PTHREAD_CREATE_JOINABLE == ptd->attr.detachstate)
    {
        os_sem_post(ptd->joinable_sem);
    }
    else
    {
        /* Destroy pthread resource. */
        _pthread_destroy(ptd);
    }
}

static int _pthread_prio_to_task_prio(int priority)
{
    int prio_max = sched_get_priority_max(SCHED_RR);
    int prio_min = sched_get_priority_min(SCHED_RR);
    int prio_tmp = 0;

    OS_ASSERT(priority <= prio_max);
    OS_ASSERT(priority >= prio_min);

    prio_tmp = (priority + 1 - prio_min) * OS_TASK_PRIORITY_MAX / (float)(prio_max + 1 - prio_min);

    return (OS_TASK_PRIORITY_MAX - prio_tmp);
}

int pthread_system_init(void)
{
    /* Initialize key area. */
    pthread_key_system_init();
    
    /* Initialize posix mqueue. */
    posix_mq_system_init();
    
    /* Initialize posix semaphore. */
    posix_sem_system_init();

    return 0;
}
OS_CMPOENT_INIT(pthread_system_init, "3");

static void pthread_entry_stub(void *parameter)
{
    void *value;
    _pthread_data_t *ptd;

    ptd = (_pthread_data_t *)parameter;

    /* Execute pthread entry. */
    value = ptd->thread_entry(ptd->thread_parameter);
    
    /* Set value. */
    ptd->return_value = value;
}

int pthread_create(pthread_t *pid, const pthread_attr_t *attr, void *(*start)(void *), void *parameter)
{
    int                ret;
    void              *stack;
    char               name[OS_NAME_MAX];
    pthread_t          pth_id;
    _pthread_data_t   *ptd;
    static os_uint16_t pthread_number = 0;
    os_uint8_t os_prio = OS_TASK_PRIORITY_MAX - 1;

    OS_ASSERT(OS_NULL != pid);

    ret = 0;
    
    /* Allocate posix thread data. */
    pth_id = _pthread_data_create();
    if (PTHREAD_NUM_MAX == pth_id) 
    {
        ret = ENOMEM;
        goto __exit;
    }
    
    /* Get pthread data. */
    ptd = _pthread_get_data(pth_id);

    if (OS_NULL != attr)
    {
        ptd->attr = *attr;
    }
    else
    {
        /* Use default attribute */
        pthread_attr_init(&ptd->attr);
    }
    
    os_prio = _pthread_prio_to_task_prio(ptd->attr.schedparam.sched_priority);

    os_snprintf(name, sizeof(name), "pth%02d", pthread_number++);

    /* Pthread is a static thread object. */
    ptd->tid = (os_task_t *)os_malloc(sizeof(os_task_t));
    if (OS_NULL == ptd->tid)
    {
        ret = ENOMEM;
        goto __exit;
    }
    memset(ptd->tid, 0, sizeof(os_task_t));

    if (PTHREAD_CREATE_JOINABLE == ptd->attr.detachstate)
    {
        ptd->joinable_sem = os_sem_create(name, 0, 1);
        if (OS_NULL == ptd->joinable_sem)
        {
            ret = ENOMEM;
            goto __exit;
        }
    }
    else
    {
        ptd->joinable_sem = OS_NULL;
    }

    ptd->thread_entry     = start;
    ptd->thread_parameter = parameter;

    if (NULL == ptd->attr.stackaddr)
    {
        stack = (void *)os_malloc(ptd->attr.stacksize);
    }
    else
    {
        stack = (void *)(ptd->attr.stackaddr);
    }

    if (OS_NULL == stack)
    {
        ret = ENOMEM;
        goto __exit;
    }

    /* Initial this pthread to system */
    if (OS_EOK != os_task_init(ptd->tid, 
                               name,
                               pthread_entry_stub,
                               ptd,
                               stack,
                               ptd->attr.stacksize,
                               os_prio))
    {
        ret = EINVAL;
        goto __exit;
    }

    /* Set pthread id. */
    *pid = pth_id;

    /* Set pthread cleanup function and ptd data. */
    /* ptd->tid->cleanup = _pthread_cleanup; */
    /* ptd->tid->user_data = ptd; */
    os_task_set_cleanup_callback(ptd->tid, _pthread_cleanup, ptd);

    /* Start thread */
    if (OS_EOK == os_task_startup(ptd->tid))
    {
        return 0;
    }

    /* Start thread failed. */
    os_task_deinit(ptd->tid);
    ret = EINVAL;

__exit:
    if (PTHREAD_NUM_MAX != pth_id)
    {
        _pthread_data_destroy(pth_id);
    }
    return ret;
}
EXPORT_SYMBOL(pthread_create);

int pthread_detach(pthread_t thread)
{
    int              ret;
    _pthread_data_t *ptd;

    ret = 0;
    ptd = _pthread_get_data(thread);

    /* os_schedule_lock(); */
    
    if (PTHREAD_CREATE_DETACHED == ptd->attr.detachstate)
    {
        /* The implementation has detected that the value specified by thread does not refer
         * to a joinable thread.
         */
        ret = EINVAL;
        goto __exit;
    }

    if (OS_TASK_STATE_CLOSE == (ptd->tid->state & OS_TASK_STATE_CLOSE))
    {
        /* This defunct pthread is not handled by idle. */
        if (OS_EOK != os_sem_wait(ptd->joinable_sem, OS_NO_WAIT))
        {
            os_sem_post(ptd->joinable_sem);

            /* Change to detach state. */
            ptd->attr.detachstate = PTHREAD_CREATE_DETACHED;

            /* Detach joinable semaphore. */
            if (ptd->joinable_sem)
            {
                os_sem_destroy(ptd->joinable_sem);
                ptd->joinable_sem = OS_NULL;
            }
        }
        else
        {
            /* Destroy this pthread. */
            _pthread_destroy(ptd);
        }

        goto __exit;
    }
    else
    {
        /* Change to detach state. */
        ptd->attr.detachstate = PTHREAD_CREATE_DETACHED;

        /* Detach joinable semaphore. */
        if (ptd->joinable_sem)
        {
            os_sem_destroy(ptd->joinable_sem);
            ptd->joinable_sem = OS_NULL;
        }
    }

__exit:
    /* os_schedule_unlock(); */
    
    return ret;
}
EXPORT_SYMBOL(pthread_detach);

int pthread_join(pthread_t thread, void **value_ptr)
{
    _pthread_data_t *ptd;
    os_err_t         result;

    ptd = _pthread_get_data(thread);
    
    if (NULL == ptd) 
    {
        return ESRCH;
    }

    if (ptd && (ptd->tid == os_task_self()))
    {
        /* Join self. */
        return EDEADLK;
    }

    if (PTHREAD_CREATE_DETACHED == ptd->attr.detachstate)
    {
        /* Join on a detached pthread. */
        return EINVAL; 
    }

    result = os_sem_wait(ptd->joinable_sem, OS_WAIT_FOREVER);
    if (OS_EOK == result)
    {
        /* Get return value. */
        if (OS_NULL != value_ptr)
        {
            *value_ptr = ptd->return_value;
        }

        /* Destroy this pthread. */
        _pthread_destroy(ptd);
    }
    else
    {
        return ESRCH;
    }

    return 0;
}
EXPORT_SYMBOL(pthread_join);

pthread_t pthread_self(void)
{
    os_task_t       *tid;
    _pthread_data_t *ptd;

    tid = os_task_self();
    if (OS_NULL == tid)
    {
        return PTHREAD_NUM_MAX;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(OS_NULL != ptd);

    return _pthread_data_get_pth(ptd);
}
EXPORT_SYMBOL(pthread_self);

void pthread_exit(void *value)
{
    _pthread_data_t    *ptd;
    _pthread_cleanup_t *cleanup;
    extern _pthread_key_data_t g_thread_keys[PTHREAD_KEY_MAX];

    if (OS_NULL == os_task_self())
    {
        return;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;

    os_schedule_lock();
    
    /* Disable cancel. */
    ptd->cancelstate = PTHREAD_CANCEL_DISABLE;
    
    ptd->return_value = value;
    
    os_schedule_unlock();

    /* Invoke pushed cleanup. */
    while (OS_NULL != ptd->cleanup)
    {
        cleanup = ptd->cleanup;
        ptd->cleanup = cleanup->next;

        cleanup->cleanup_func(cleanup->parameter);
        
        /* Release this cleanup function */
        os_free(cleanup);
    }

    /* Destruct thread local key */
    if (OS_NULL != ptd->tls)
    {
        void       *data;
        os_uint32_t index;

        for (index = 0; index < PTHREAD_KEY_MAX; index++)
        {
            if (g_thread_keys[index].is_used)
            {
                data = ptd->tls[index];
                if (data)
                {
                    g_thread_keys[index].destructor(data);
                }
            }
        }

        /* Release tls area. */
        os_free(ptd->tls);
        ptd->tls = OS_NULL;
    }

    /* Detach thread. */
    os_task_deinit(ptd->tid);
    
    /* Reschedule thread. */
    /* os_schedule(); */ // TODO: need check
}
EXPORT_SYMBOL(pthread_exit);

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    OS_ASSERT(OS_NULL != once_control);
    OS_ASSERT(OS_NULL != init_routine);

    os_schedule_lock();
    
    if (!(*once_control))
    {
        /* Call routine once. */
        *once_control = 1;
        os_schedule_unlock();

        init_routine();
        return 0;
    }
    
    os_schedule_unlock();

    return 0;
}
EXPORT_SYMBOL(pthread_once);

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
    return EOPNOTSUPP;
}
EXPORT_SYMBOL(pthread_atfork);

int pthread_kill(pthread_t thread, int sig)
{
#ifdef OS_USING_SIGNALS
    _pthread_data_t *ptd;

    ptd = _pthread_get_data(thread);
    if (ptd)
    {
        return os_task_kill(ptd->tid, sig);
    }

    return EINVAL;
#else
    return ENOSYS;
#endif
}
EXPORT_SYMBOL(pthread_kill);

#ifdef OS_USING_SIGNALS
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
    return sigprocmask(how, set, oset);
}
#endif

void pthread_cleanup_pop(int execute)
{
    _pthread_data_t    *ptd;
    _pthread_cleanup_t *cleanup;

    if (OS_NULL == os_task_self())
    {
        return;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(OS_NULL != ptd);

    if (execute)
    {
        os_schedule_lock();
        
        cleanup = ptd->cleanup;
        if (cleanup)
        {
            ptd->cleanup = cleanup->next;
        }
        
        os_schedule_unlock();

        if (cleanup)
        {
            cleanup->cleanup_func(cleanup->parameter);

            os_free(cleanup);
        }
    }
}
EXPORT_SYMBOL(pthread_cleanup_pop);

void pthread_cleanup_push(void (*routine)(void *), void *arg)
{
    _pthread_data_t *ptd;
    _pthread_cleanup_t *cleanup;

    if (OS_NULL == os_task_self())
    {
        return;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(OS_NULL != ptd);

    cleanup = (_pthread_cleanup_t *)os_malloc(sizeof(_pthread_cleanup_t));
    if (OS_NULL != cleanup)
    {
        cleanup->cleanup_func = routine;
        cleanup->parameter    = arg;

        os_schedule_lock();
        
        cleanup->next = ptd->cleanup;
        ptd->cleanup  = cleanup;
        
        os_schedule_unlock();
    }
}
EXPORT_SYMBOL(pthread_cleanup_push);

/*
 * According to IEEE Std 1003.1, 2004 Edition , following pthreads
 * interface support cancellation point:
 * mq_receive()
 * mq_send()
 * mq_timedreceive()
 * mq_timedsend()
 * msgrcv()
 * msgsnd()
 * msync()
 * pthread_cond_timedwait()
 * pthread_cond_wait()
 * pthread_join()
 * pthread_testcancel()
 * sem_timedwait()
 * sem_wait()
 *
 * A cancellation point may also occur when a thread is
 * executing the following functions:
 * pthread_rwlock_rdlock()
 * pthread_rwlock_timedrdlock()
 * pthread_rwlock_timedwrlock()
 * pthread_rwlock_wrlock()
 *
 * The pthread_cancel(), pthread_setcancelstate(), and pthread_setcanceltype()
 * functions are defined to be async-cancel safe.
 */

int pthread_setcancelstate(int state, int *oldstate)
{
    _pthread_data_t *ptd;

    if (OS_NULL == os_task_self())
    {
        return EINVAL;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(OS_NULL != ptd);

    if ((PTHREAD_CANCEL_ENABLE == state) || (PTHREAD_CANCEL_DISABLE == state))
    {
        if (oldstate)
        {
            *oldstate = ptd->cancelstate;
        }
        ptd->cancelstate = state;

        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_setcancelstate);

int pthread_setcanceltype(int type, int *oldtype)
{
    _pthread_data_t *ptd;

    if (OS_NULL == os_task_self())
    {
        return EINVAL;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(OS_NULL != ptd);

    if ((PTHREAD_CANCEL_DEFERRED != type) && (PTHREAD_CANCEL_ASYNCHRONOUS != type))
    {
        return EINVAL;
    }

    if (oldtype)
    {
        *oldtype = ptd->canceltype;
    }
    
    ptd->canceltype = type;

    return 0;
}
EXPORT_SYMBOL(pthread_setcanceltype);

void pthread_testcancel(void)
{
    int cancel = 0;
    _pthread_data_t *ptd;

    if (OS_NULL == os_task_self())
    {
        return;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(OS_NULL != ptd);

    if (PTHREAD_CANCEL_ENABLE == ptd->cancelstate)
    {
        cancel = ptd->canceled;
    }
    
    if (cancel)
    {
        pthread_exit((void *)PTHREAD_CANCELED);
    }
}
EXPORT_SYMBOL(pthread_testcancel);

int pthread_cancel(pthread_t thread)
{
    _pthread_data_t *ptd;

    /* Get posix thread data. */
    ptd = _pthread_get_data(thread);
    OS_ASSERT(OS_NULL != ptd);

    /* Cancel self. */
    if (ptd->tid == os_task_self())
    {
        return 0;
    }

    /* Set canceled. */
    if (PTHREAD_CANCEL_ENABLE == ptd->cancelstate)
    {
        ptd->canceled = 1;
        if (PTHREAD_CANCEL_ASYNCHRONOUS == ptd->canceltype)
        {
            /*
             * To detach thread.
             * this thread will be removed from scheduler list
             * and because there is a cleanup function in the
             * thread (pthread_cleanup), it will move to defunct
             * thread list and wait for handling in idle thread.
             */
            os_task_deinit(ptd->tid);
        }
    }

    return 0;
}
EXPORT_SYMBOL(pthread_cancel);


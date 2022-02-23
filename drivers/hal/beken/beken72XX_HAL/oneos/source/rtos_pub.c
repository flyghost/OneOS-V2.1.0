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
 * @file        rtos_pub.c
 *
 * @brief       This file implements the oneos IPC functions for beken drivers
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-18   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "sys_rtos.h"
#include "error.h"
#include "rtos_pub.h"
#include "os_errno.h"
#include "mem_pub.h"

#include "os_types.h"
#include "os_util.h"
#include "os_sem.h"
#include "os_mutex.h"
#include <os_clock.h>


#define THREAD_TIMESLICE  5

#define RTOS_DEBUG        0
#if RTOS_DEBUG
#define RTOS_DBG(...)     os_kprintf("[RTOS]"),os_kprintf(__VA_ARGS__)
#else
#define RTOS_DBG(...)
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/
OSStatus rtos_create_thread( beken_thread_t* thread, uint8_t priority, const char* name, 
                        beken_thread_function_t function, uint32_t stack_size, beken_thread_arg_t arg )
{
    if(thread == OS_NULL)
    {
        RTOS_DBG("can not create null thread\n");
        return kGeneralErr;
    }
	
    *thread = os_task_create(name, function, arg, stack_size, priority, THREAD_TIMESLICE);
    if(*thread != OS_NULL)
    {		
        os_task_startup(*thread);
    }
    else
    {
    	os_kprintf("create thread fail\n");
		
        RTOS_DBG("create thread fail\n");
        return kGeneralErr;
    }

    RTOS_DBG("create thread %s\n", name);

    return kNoErr;
}

OSStatus rtos_delete_thread( beken_thread_t* thread )
{
    if(thread != OS_NULL)
    {
        os_task_destroy(*thread);
    }
    else
    {
        RTOS_DBG("thread == NULL, %s delete self NC\r\n", __FUNCTION__);
        return kGeneralErr;
    }

    RTOS_DBG("deleate thread\n");

    return kNoErr;

}

void rtos_thread_sleep(uint32_t seconds)
{
    os_task_delay(os_tick_from_ms(seconds * 1000));
}

static os_uint32_t rtos_sem_cnt = 0;
static os_uint32_t rtos_mutex_cnt = 0;

OSStatus rtos_init_semaphore( beken_semaphore_t* semaphore, int maxCount )
{
    *semaphore = os_sem_create("rtos_sem", 0,  OS_IPC_FLAG_FIFO);
    RTOS_DBG("rtos_init_semaphore:%8x\n", *semaphore);
    rtos_sem_cnt++;

    return (*semaphore != OS_NULL) ? kNoErr : kGeneralErr;
}

OSStatus rtos_init_semaphore_ex( beken_semaphore_t* semaphore, const char *name, int maxCount, int initCount )
{
    *semaphore = os_sem_create(name, initCount,  OS_IPC_FLAG_FIFO);
    RTOS_DBG("rtos_init_semaphore_ex:%s %8x\n", name, *semaphore);
    rtos_sem_cnt++;

    return (*semaphore != OS_NULL) ? kNoErr : kGeneralErr;
}

OSStatus rtos_get_semaphore(beken_semaphore_t* semaphore, uint32_t timeout_ms )
{
    os_err_t result;

    result = os_sem_wait(*semaphore, os_tick_from_ms(timeout_ms));

    if(result == OS_EOK)
    {
        return kNoErr;
    }
    else
    {
        struct os_object *object = (struct os_object *)(*semaphore);
        RTOS_DBG("%s take semaphore failed %d \n", object->name, result);
        return kTimeoutErr;
    }
}

int rtos_get_sema_count(beken_semaphore_t* semaphore )
{
    RTOS_DBG("rtos_get_sema_count\n");
    os_sem_t *sem = *semaphore;
    
    return sem->count;
}


int rtos_set_semaphore( beken_semaphore_t* semaphore)
{
    os_err_t result;

    result = os_sem_post(*semaphore);
    if(result != OS_EOK)
    {
        RTOS_DBG("release semaphore failed %d \n", result);
        return kGeneralErr;
    }

    return kNoErr;    
}

OSStatus rtos_deinit_semaphore(beken_semaphore_t *semaphore )
{
    RTOS_DBG("rtos_deinit_semaphore:%8x\n", *semaphore);
    if(semaphore != OS_NULL)
    {
        RTOS_DBG("rtos_deinit_sucess:%8x\n");
        os_sem_destroy(*semaphore);
        rtos_sem_cnt--;        
        *semaphore = OS_NULL;
        return kNoErr;

    }

    return kGeneralErr;
}

OSStatus rtos_init_mutex( beken_mutex_t* mutex )
{
    RTOS_DBG("rtos_init_mutex\n");

    /* Mutex uses priority inheritance */
    *mutex = os_mutex_create("rtos_mutex", OS_IPC_FLAG_PRIO,OS_FALSE);
    if ( *mutex == OS_NULL )
    {
        return kGeneralErr;
    }

    rtos_mutex_cnt++;

    return kNoErr;
}

OSStatus rtos_lock_mutex( beken_mutex_t* mutex)
{
    RTOS_DBG("rtos_lock_mutex\n");
    os_mutex_lock(*mutex, BEKEN_WAIT_FOREVER);

    return OS_EOK;
}

OSStatus rtos_unlock_mutex( beken_mutex_t* mutex)
{
    RTOS_DBG("rtos_unlock_mutex\n");
    os_mutex_unlock(*mutex);

    return OS_EOK;
}

OSStatus rtos_deinit_mutex( beken_mutex_t* mutex)
{
    if(*mutex != OS_NULL)
    {
        os_mutex_destroy(*mutex);
        rtos_mutex_cnt--;
        *mutex = OS_NULL;
    }

    return OS_EOK;
}

OSStatus rtos_init_queue(beken_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages)
{
    beken_queue_t mq = OS_NULL;

    mq = os_malloc(sizeof(struct beken_queue));
	if(OS_NULL == mq)
		return kGeneralErr;
	
    os_memset(mq, 0, sizeof(struct beken_queue));
    *queue = mq;

    RTOS_DBG("rtos_init_queue!\r\n");
    mq->handle = os_mb_create("rtos_queue", number_of_messages, OS_IPC_FLAG_FIFO);
	if (mq->handle == OS_NULL)
	{
		RTOS_DBG("rt_mailbox_create: 0x%08X!\r\n", mq->handle);
		goto _exit;
	}
    RTOS_DBG("mq->handle:%08p!\r\n", mq->handle);
    RTOS_DBG("rt_mp_create!\r\n");
	mq->mp = os_mp_create("rtos_queue", number_of_messages, message_size);
	if (mq->mp == OS_NULL)
	{
		RTOS_DBG("rt_mempool_create: 0x%08X!\r\n", mq->mp);
		goto _exit;
	}
    
    RTOS_DBG("rt_mp_create done!\r\n");
    
	mq->message_size       = message_size;
    mq->number_of_messages = number_of_messages;

    return kNoErr;

_exit:
    if(mq->handle != OS_NULL)
    {
        os_mb_destroy(mq->handle);
        mq->handle = OS_NULL;
    }

    if(mq->mp != OS_NULL)
    {
        os_mp_destroy(mq->mp);
        mq->mp = OS_NULL;
    }
	
	if(mq != OS_NULL)
	{
		os_free(mq);
		mq = OS_NULL;
	}
	return kGeneralErr;
}

OSStatus rtos_push_to_queue(beken_queue_t* queue, void* message, UINT32 timeout_ms)
{
    os_err_t ret;
    void *msg_tmp = OS_NULL;
    beken_queue_t mq = *queue;

	msg_tmp = os_mp_alloc(mq->mp, os_tick_from_ms(timeout_ms));
	if(msg_tmp)
	{
		os_memcpy(msg_tmp, message, mq->message_size);
		ret = os_mb_send(mq->handle, (UINT32)msg_tmp, os_tick_from_ms(timeout_ms));
		if(ret != OS_EOK)
		{
			RTOS_DBG("%s rt_mb_send_wait ret:%d!\r\n", __FUNCTION__, ret);
			return kGeneralErr;
		}
	}
	else
	{
		RTOS_DBG("%s rt_mp_alloc failed! queue->message_size: %d\r\n", __FUNCTION__, mq->message_size);
		return kGeneralErr;
	}

	return kNoErr;
}

OSStatus rtos_push_to_queue_front(beken_queue_t* queue, void* message, uint32_t timeout_ms)
{
    beken_queue_t mq = *queue;
    
    os_kprintf("\nrtos_push_to_queue_front not implement!!!\n");

    return kGeneralErr;
}

OSStatus rtos_pop_from_queue(beken_queue_t* queue, void* message, uint32_t timeout_ms)
{
    void *msg_tmp = OS_NULL;
    beken_queue_t mq = *queue;
    os_err_t result;

	result = os_mb_recv(mq->handle, (os_uint32_t *)&msg_tmp, os_tick_from_ms(timeout_ms));
	if(result != OS_EOK)
	{
		RTOS_DBG("%s rt_mb_recv ret:%d, ms:=%d!\r\n", __FUNCTION__, result, os_tick_from_ms(timeout_ms));
        RTOS_DBG("mq->handle:0x%08p!\r\n", mq->handle);
		return kGeneralErr;
	}

	if(msg_tmp)
	{
		os_memcpy(message, msg_tmp, mq->message_size);
		os_mp_free(msg_tmp);
	}
	else
	{
		RTOS_DBG("%s rt_mb_recv item:0x%08X!\r\n", __FUNCTION__, msg_tmp);
		return kGeneralErr;
	}

	return kNoErr;
}

OSStatus rtos_deinit_queue(beken_queue_t* queue)
{
    beken_queue_t mq = *queue;


    if(mq->handle != OS_NULL)
    {
        os_mb_destroy(mq->handle);
        mq->handle =OS_NULL;
    }

    if(mq->mp != OS_NULL)
    {
        os_mp_destroy(mq->mp);
        mq->mp = OS_NULL;
    }

    mq->message_size       = 0;
    mq->number_of_messages = 0;

    os_free(mq);
	mq = OS_NULL;
    return kNoErr;
}

BOOL rtos_is_queue_empty(beken_queue_t* queue )
{
    os_uint32_t level;
    beken_queue_t mq = *queue;

    os_enter_critical();
    if(mq->handle->entry == 0)
    {
        os_exit_critical();
        
        return true;
    }
    
    os_exit_critical();

    return false;
}

BOOL rtos_is_queue_full(beken_queue_t* queue)
{
    os_uint32_t level;
    beken_queue_t mq = *queue;

    os_enter_critical();
    if(mq->handle->entry == mq->handle->size)
    {
        os_exit_critical();

        return true;
    }

    os_exit_critical();

    return false;
}

OSStatus rtos_delay_milliseconds( uint32_t num_ms)
{
    os_task_delay(os_tick_from_ms(num_ms));
} 

static void timer_oneshot_callback(void* parameter)
{
    beken2_timer_t *timer = (beken2_timer_t*)parameter;

    RTOS_DBG("one shot callback\n");

	if(BEKEN_MAGIC_WORD != timer->beken_magic)
	{
		return;
	}
    if (timer->function)
    {
        timer->function(timer->left_arg, timer->right_arg );
    }
}

OSStatus rtos_start_oneshot_timer( beken2_timer_t* timer)
{
    RTOS_DBG("oneshot_timer start %x\n",timer->handle);

    if(timer->handle != OS_NULL)
    {
        os_timer_start(timer->handle);
        return OS_EOK;
    }

    return OS_ERROR;
}

OSStatus rtos_stop_oneshot_timer(beken2_timer_t* timer)
{
    RTOS_DBG("oneshot_timer stop %x\n",timer->handle);

    if(timer->handle != OS_NULL)
    {
        os_timer_stop(timer->handle);
        return kNoErr;
    }

    return kGeneralErr;
}

BOOL rtos_is_oneshot_timer_init(beken2_timer_t* timer)
{
    RTOS_DBG("oneshot_timer is init \n");
    return timer->handle ? true : false;
}

BOOL rtos_is_oneshot_timer_running(beken2_timer_t* timer)
{
    os_timer_t *os_timer = (os_timer_t *)(timer->handle);

    RTOS_DBG("oneshot_timer is runing \n");
    return os_timer->parent.flag & OS_TIMER_FLAG_ACTIVATED ? true : false;
}

OSStatus rtos_oneshot_reload_timer( beken2_timer_t* timer)
{
    RTOS_DBG("reload oneshot_timer %x\n",timer->handle);

    if(rtos_is_oneshot_timer_running(timer))
    {
        os_timer_start(timer->handle);
        RTOS_DBG("timer is runing, set tick to 0 \r\n");
    }
    else
    {
        os_timer_start(timer->handle);
        RTOS_DBG("timer is stop, start timer \r\n");
    }
    return kNoErr;
}

OSStatus rtos_init_oneshot_timer( beken2_timer_t *timer, 
									uint32_t time_ms, 
									timer_2handler_t function,
									void* larg, 
									void* rarg )
{
	OSStatus ret = kNoErr;

    RTOS_DBG("create oneshot_timer \n");
    timer->function = function;
    timer->left_arg = larg;
    timer->right_arg = rarg;	
	timer->beken_magic = BEKEN_MAGIC_WORD;

    timer->handle = os_timer_create("rtos_oneshot_time",
                                    timer_oneshot_callback,
                                    timer,
                                    os_tick_from_ms(time_ms),
                                    OS_TIMER_FLAG_ONE_SHOT|OS_TIMER_FLAG_SOFT_TIMER);
    if ( timer->handle == NULL )
    {
        ret = kGeneralErr;
    }
    else
    {
        //((os_timer_t*)(timer->handle))->user_timer = timer;
        RTOS_DBG("create oneshot_timer %x\n",timer->handle);

    }

    return ret;
}

void rtos_deinit_free_beken_timer(os_timer_t *t)
{
	/*
    ((beken2_timer_t *)(t->user_timer))->handle = 0;
    ((beken2_timer_t *)(t->user_timer))->function = 0;
    ((beken2_timer_t *)(t->user_timer))->left_arg = 0;
    ((beken2_timer_t *)(t->user_timer))->right_arg = 0;
    ((beken2_timer_t *)(t->user_timer))->beken_magic = 0;
    */
}

OSStatus rtos_deinit_oneshot_timer( beken2_timer_t* timer )
{
	OSStatus ret = kNoErr;
    register os_base_t level;
    RTOS_DBG("delete oneshot_timer %x\n",timer->handle);

    /* disable interrupt */
    level = os_hw_interrupt_disable();
    os_timer_t *os_timer = (os_timer_t*)(timer->handle);

    if(os_timer_destroy((os_timer_t*)timer->handle) != OS_EOK)
    {
        ret = OS_ERROR;
    }
	else
	{
		timer->handle = 0;
		timer->function = 0;
		timer->left_arg = 0;
		timer->right_arg = 0;
		timer->beken_magic = 0;
	}
    /* enable interrupt */
    os_hw_interrupt_enable(level);

    return ret;
}

static void timer_period_callback(void* parameter)
{
    beken_timer_t *timer = (beken_timer_t*)parameter;

    RTOS_DBG("period timer callback\n");

    if (timer->function)
    {
        timer->function(timer->arg);
    }
}

OSStatus rtos_start_timer( beken_timer_t* timer)
{
    RTOS_DBG("period_timer start \n");
    if(timer->handle != OS_NULL)
    {
        os_timer_start(timer->handle);
        return OS_EOK;
    }

    return OS_ERROR;
}

OSStatus rtos_stop_timer(beken_timer_t* timer)
{
    RTOS_DBG("period_timer stop \n");

    if(timer->handle != OS_NULL)
    {
        os_timer_stop(timer->handle);
        return OS_EOK;
    }

    return OS_ERROR;
}

BOOL rtos_is_timer_init(beken_timer_t* timer)
{
    RTOS_DBG("period_timer is init \n");
    return timer->handle ? true : false;
}

BOOL rtos_is_timer_running(beken_timer_t* timer)
{
    os_timer_t *os_timer = (os_timer_t*)(timer->handle);

    RTOS_DBG("period_timer is runing \n");
    return os_timer->parent.flag & OS_TIMER_FLAG_ACTIVATED ? true : false;
}

OSStatus rtos_reload_timer( beken_timer_t* timer)
{
    RTOS_DBG("reload period timer\n");

    if(rtos_is_timer_running(timer))
    {
        os_timer_start(timer->handle);
        RTOS_DBG("timer is runing, set tick to 0 \r\n");
    }
    else
    {
        os_timer_start(timer->handle);
        // os_kprintf("timer is stop, start timer \r\n");
    }
    return kNoErr;
}

OSStatus rtos_change_period( beken_timer_t* timer, uint32_t time_ms)
{
    os_uint32_t timeout_value;

    timeout_value = os_tick_from_ms(time_ms);
    os_timer_control(timer->handle, OS_TIMER_CTRL_SET_TIME, (void *)&timeout_value);
    os_timer_start(timer->handle);

    return kNoErr;
}

OSStatus rtos_init_timer( beken_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg)
{
	OSStatus ret = kNoErr;

    RTOS_DBG("create period_timer \n");
    timer->function = function;
    timer->arg      = arg;

    timer->handle = os_timer_create("rtos_period_time",
                                    timer_period_callback,
                                    timer,
                                    os_tick_from_ms(time_ms),
                                    OS_TIMER_FLAG_PERIODIC|OS_TIMER_FLAG_SOFT_TIMER);
    if ( timer->handle == NULL )
    {
        ret = kNoErr;
    }
    else
    {
        //((os_timer_t*)(timer->handle))->user_timer = timer;
    }
    
    return ret;
}

OSStatus rtos_init_timer_ex( beken_timer_t* timer, const char* name, uint32_t time_ms, timer_handler_t function, void* arg)
{
	OSStatus ret = kNoErr;

    RTOS_DBG("create period_timer_ex \n");
    timer->function = function;
    timer->arg      = arg;

    timer->handle = os_timer_create(name,
                                    timer_period_callback,
                                    timer,
                                    os_tick_from_ms(time_ms),
                                    OS_TIMER_FLAG_PERIODIC|OS_TIMER_FLAG_SOFT_TIMER);
    if ( timer->handle == NULL )
    {
        ret = kNoErr;
    }
    else
    {
        //((rt_timer_t)(timer->handle))->user_timer = timer;
    }
    
    return ret;
}

OSStatus rtos_deinit_timer( beken_timer_t* timer )
{
	OSStatus ret = kNoErr;

    RTOS_DBG("delete period_timer \n");
    if(os_timer_destroy((os_timer_t*)timer->handle) != OS_EOK)
    {
        ret = kGeneralErr;
    }
	else
	{
		timer->handle = 0;
		timer->function = 0;
		timer->arg      = 0;
	}

    return ret;
}

uint32_t rtos_get_timer_expiry_time( beken_timer_t* timer )
{
    // TODO:
    os_kprintf("[TODO]:Note, rtos_get_timer_expiry_time is not implement \n");
    return 0;
}

uint32_t rtos_get_time(void)
{
    return os_tick_get();
}

uint32_t rtos_get_free_mem() {
#ifdef OS_USING_HEAP
    os_uint32_t total;
    os_uint32_t used;
    os_memory_info(&total, &used, NULL);

    return total - used;
#else
    return 0;
#endif
}


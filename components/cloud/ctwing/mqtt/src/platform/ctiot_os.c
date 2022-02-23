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
 * @file        ctiot_os.c
 * 
 * @brief       Implementation of the functions in ctiot_os.h for OneOS systems. 
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-11-26   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#include "ctiot_os.h"
#include "ctiot_log.h"

/* 包含的头文件 */
#include <stdio.h>
#include <stdlib.h>

/* 定义一个表示链表的结构体指针 */
struct list {
	int mem;			/* 内存大小 */
	void* p;		/* 指针地址 */
	struct list *next;	/* 指向下一个链表的指针 */
};
 
/* 定义一个链表头部 */
struct list list_head = {0};

int max_mem = 0;

static void list_add(struct list *list)
{
	struct list *temp;
    struct list * head = &list_head;

    if(head->next == NULL)
    {
        head->next = list;
        list->next = NULL;
    }
    else{
        temp = head->next;
        while(temp)
        {
            if(NULL == temp->next)
            {
                temp->next = list;
                list->next = NULL;
            }
            temp = temp->next;
        }
    }	
}

static int list_del(void* p){
    struct list *temp = NULL;
    struct list *pp = NULL;
    struct list * head = &list_head;
    if(head->next == NULL){
        return -1;
    }
    temp = head->next;
    if(temp->p == p){
        head->next = temp->next;
        free(temp); /* Add by OneOS Team, solve bug of list cause memory leak */
        return 0;
    }else{
        while(temp->next){
            if(temp->next->p == p){
                pp = temp->next;
                temp->next = temp->next->next;
                free(pp);
                return 0;
            }
            temp = temp->next;
        }
    }
    return -1;
}

#if defined(MEM_TEST)
static int get_count_mem()
{	
	struct list *temp;
    struct list * head = &list_head;
 
	temp = head;
    int count  = 0;
	while(temp)
	{
		count += temp->mem;
		temp = temp->next;
	}
    return count;
}
#endif

int ctiot_mem_count = 0;
int ctiot_mem_size = 0;
int ctiot_mem_get(int mem,void *p){
    struct list * list = NULL;
    list = (struct list *)malloc(sizeof(struct list));
    list->mem = mem;
    list->p = p;
    list->next = NULL;
    list_add(list);

#if defined(MEM_TEST)
    int use_mem = get_count_mem();
    if(use_mem > max_mem){
        max_mem = use_mem;
    }
    CTIOT_LOG(LOG_INFO,"used_mem: %d",use_mem);
    CTIOT_LOG(LOG_INFO,"max_used_mem: %d",max_mem);
#endif

    return 0;
}
int ctiot_mem_put(void *p){
    int result = list_del(p);
    if(result < 0)
    {
#if defined(MEM_TEST)
        CTIOT_LOG(LOG_WARNING,"can not free addr: %p",p);
#endif
        return result;
    }

#if defined(MEM_TEST)
    int use_mem = get_count_mem();
    if(use_mem > max_mem){
        max_mem = use_mem;
    }
    CTIOT_LOG(LOG_INFO,"used_mem: %d",use_mem);
    CTIOT_LOG(LOG_INFO,"max_used_mem: %d",max_mem);
#endif

    return 0;
}   

void* CTIOT_MALLOC(size_t size)
{
    void* p = NULL;
    OS_GET_MEM(p,char,size);
    return p;
}

void CTIOT_FREE(void *p)
{
    OS_PUT_MEM(p);
}


int ctiot_thread_create(CTIOT_Thread_t *thread_handler, 
                        void (*thread_routine)(void *), 
                        void *p_argument,
                        int32_t priority,
                        size_t stacksize)
{
    int status = true;
    os_task_t *iot_task = OS_NULL;
    int32_t task_priority;
    size_t task_stacksize;
    char name[OS_NAME_MAX] = {0};
    static os_uint16_t ctiot_thread_num = 0;

    if(!thread_handler || !thread_routine)
    {   
        CTIOT_LOG(LOG_ERR,"Failed to create new thread, invalid argument."); 

        return false;
    }

    snprintf(name, sizeof(name), "ctiotthd%02d", ctiot_thread_num++);

    if(CTIOT_THREAD_IGNORE_STACK_SIZE != stacksize)
    {
         task_stacksize = stacksize;
    }
    else
    {
         task_stacksize = CTIOT_THREAD_DEFAULT_STACK_SIZE;
    }

    if(CTIOT_THREAD_IGNORE_PRIORITY != priority)
    {
        task_priority = priority; 
    }
    else
    {
        task_priority = CTIOT_THREAD_DEFAULT_PRIORITY;
    }

    iot_task = os_task_create(name, thread_routine, p_argument, task_stacksize, task_priority);
    
    if(OS_NULL != iot_task)
    {
        os_task_startup(iot_task);

        *thread_handler = iot_task;
        
        status = true;

        CTIOT_LOG(LOG_INFO,"succeed to create new thread, thread name %s", name);
    }
    else
    {
        CTIOT_LOG(LOG_ERR,"Failed to create new thread, thread name %s", name);
        
        status = false;
    }

    return status;
}


void ctiot_thread_destroy(CTIOT_Thread_t *thread_handler)
{
    os_task_t *iot_task = OS_NULL;

    if(!thread_handler)
    {   
        CTIOT_LOG(LOG_ERR,"Failed to destroy thread, invalid argument."); 
        
        return;
    }

    iot_task = *thread_handler;
    
    os_task_destroy(iot_task);
}


int ctiot_mutex_create(CTIOT_Mutex_t * p_mutex, bool recursive)
{
    int status = true;
    os_mutex_t *iot_mutex = OS_NULL;
    char name[OS_NAME_MAX] = {0};
    static os_uint16_t ctiot_mutex_num = 0;
    
    if (!p_mutex)
    {   
        CTIOT_LOG(LOG_ERR,"Failed to create new mutex, invalid argument.");
        
        return false;
    }
    
    snprintf(name, sizeof(name), "ctiotmtx%02d", ctiot_mutex_num++);
  
    iot_mutex = os_mutex_create(name, recursive);
    
    if (OS_NULL == iot_mutex)
    {
        CTIOT_LOG(LOG_ERR,"Failed to create new mutex %p.", p_mutex);
        
        status = false;
    }
    else
    {
        *p_mutex = iot_mutex;
        
        status = true;
    }
    
    return status;
}


void ctiot_mutex_destroy(CTIOT_Mutex_t * p_mutex)
{
    os_err_t mutex_errno = 0;
    os_mutex_t *iot_mutex = *p_mutex;
    
    /* It's busy */
    if (OS_NULL != iot_mutex->owner)
    {   
        CTIOT_LOG(LOG_ERR,"Failed to destroy mutex %p. busy now.", p_mutex);

        OS_ASSERT( 0 );
    }
    
    mutex_errno = os_mutex_destroy(iot_mutex);

    if (OS_EOK != mutex_errno)
    {
        CTIOT_LOG(LOG_ERR,"Failed to destroy mutex %p. errno=%d.", p_mutex, mutex_errno);

        OS_ASSERT( 0 );
    }

    *p_mutex = NULL;
}


void ctiot_mutex_lock(CTIOT_Mutex_t * p_mutex)
{
    int mutex_type = 0;
    os_err_t mutex_errno = 0;
    os_mutex_t *iot_mutex = *p_mutex;
    
    mutex_type = iot_mutex->is_recursive;

    os_schedule_lock();
    
    if ((iot_mutex->owner == os_task_self()) && (OS_TRUE != mutex_type))
    {
        os_schedule_unlock();
        
        CTIOT_LOG(LOG_ERR,"Failed to lock mutex %p. illegal use of non-recursive lock.", p_mutex);

        OS_ASSERT( 0 );
    }
    
    os_schedule_unlock();

    if (OS_TRUE != mutex_type)
    {
        mutex_errno = os_mutex_lock(iot_mutex, OS_WAIT_FOREVER);
    }
    else
    {
        mutex_errno = os_mutex_recursive_lock(iot_mutex, OS_WAIT_FOREVER);
    }

    if (OS_EOK != mutex_errno)
    {
        CTIOT_LOG(LOG_ERR,"Failed to lock mutex %p. errno=%d.", p_mutex, mutex_errno);

        OS_ASSERT( 0 );
    }
}


int ctiot_mutex_trylock(CTIOT_Mutex_t * p_mutex)
{
    int status = true;
    int mutex_type = 0;
    os_err_t mutex_errno = 0;
    os_mutex_t *iot_mutex = *p_mutex;
    
    mutex_type = iot_mutex->is_recursive;
    
    os_schedule_lock();
    
    if ((iot_mutex->owner == os_task_self()) && (OS_TRUE != mutex_type))
    {
        os_schedule_unlock();
        
        CTIOT_LOG(LOG_ERR,"Failed to trylock mutex %p. non-recursive lock has been taken.", p_mutex);
        
        return false;
    }
    
    os_schedule_unlock();

    if (OS_TRUE != mutex_type)
    {
        mutex_errno = os_mutex_lock(iot_mutex, OS_NO_WAIT);
    }
    else
    {
        mutex_errno = os_mutex_recursive_lock(iot_mutex, OS_NO_WAIT);
    }

    if (OS_EOK != mutex_errno)
    {
        CTIOT_LOG(LOG_ERR,"Mutex mutex %p is not available. errno=%d.", p_mutex, mutex_errno);

        status = false;
    }

    return status;
}


void ctiot_mutex_unlock(CTIOT_Mutex_t * p_mutex)
{
    int mutex_type = 0;
    os_err_t mutex_errno = 0;
    os_mutex_t *iot_mutex = *p_mutex;

    mutex_type = iot_mutex->is_recursive;
    
    os_schedule_lock();
    
    if (iot_mutex->owner != os_task_self())
    {
        /* No thread waiting on this mutex. */
        if (OS_NULL == iot_mutex->owner)
        {   
            CTIOT_LOG(LOG_ERR,"iotMutex unlock fail, iotMutex owner is NULL.");
            
            os_schedule_unlock();
            
            return;
        }
        else
        {   
            CTIOT_LOG(LOG_ERR,"iotMutex unlock before, owner: %s\n", iot_mutex->owner->name);
            
            OS_ASSERT( 0 );
        }
    }
    
    os_schedule_unlock();

    if (OS_TRUE != mutex_type)
    {
        mutex_errno = os_mutex_unlock(iot_mutex);
    }
    else
    {
        mutex_errno = os_mutex_recursive_unlock(iot_mutex);
    }

    if (OS_EOK != mutex_errno)
    {
        CTIOT_LOG(LOG_ERR,"Failed to unlock mutex %p. errno=%d.", p_mutex, mutex_errno);
        
        OS_ASSERT( 0 );
    }
}


int ctiot_semaphore_create(CTIOT_Semaphore_t * p_semaphore,
                                 uint32_t initial_value,
                                 uint32_t max_value)
{
    int status = true;
    os_sem_t *iot_sem = OS_NULL;
    char name[OS_NAME_MAX] = {0};
    static os_uint16_t ctiot_sem_num = 0;
    
    if (!p_semaphore)
    {   
        CTIOT_LOG(LOG_ERR,"Failed to create new semaphore, invalid argument.");
        
        return false;
    }
    
    if (max_value > (uint32_t)OS_SEM_MAX_VALUE)
    {
        CTIOT_LOG(LOG_ERR,"%u is larger than the maximum value a semaphore may"
                  " have on this system.", max_value);

        return false;
    }

    snprintf(name, sizeof(name), "ctiotsem%02d", ctiot_sem_num++);
    
    iot_sem = os_sem_create(name, initial_value, max_value);
    
    if (OS_NULL == iot_sem)
    {
        CTIOT_LOG(LOG_ERR,"Failed to create new semaphore %p, because of no memory.", p_semaphore);
        
        status = false;
    }
    else
    {
        *p_semaphore = iot_sem;
        
        status = true;

        CTIOT_LOG(LOG_INFO,"Succeed to create semaphore %p. %p.", p_semaphore, *p_semaphore);
    }
        

    return status;
}

void ctiot_semaphore_destroy(CTIOT_Semaphore_t * p_semaphore)
{
    os_err_t sem_errno = 0;
    os_sem_t *iot_sem = *p_semaphore;
    
    if (!iot_sem)
    {
        CTIOT_LOG(LOG_ERR,"Failed to destroy semaphore %p, invalid argument.", p_semaphore);

        OS_ASSERT( 0 );
    }
    
    sem_errno = os_sem_destroy(iot_sem);
    
    if (OS_EOK != sem_errno)
    {
        CTIOT_LOG(LOG_ERR,"Failed to destroy semaphore %p", p_semaphore);

        OS_ASSERT( 0 );
    }
    
    *p_semaphore = NULL;
    
    CTIOT_LOG(LOG_INFO,"Succeed to destroy semaphore %p. %p.", p_semaphore, *p_semaphore);
}


uint32_t ctiot_semaphore_getcount(CTIOT_Semaphore_t * p_semaphore)
{
    int iot_sem_count = 0;
    os_sem_t *iot_sem = *p_semaphore;

    if (!iot_sem)
    {
        CTIOT_LOG(LOG_ERR,"Failed to get count on semaphore %p, invalid argument.", p_semaphore);

        OS_ASSERT( 0 );
    }
    
    iot_sem_count = iot_sem->count;

    return (uint32_t)iot_sem_count;
}


void ctiot_semaphore_wait(CTIOT_Semaphore_t * p_semaphore)
{
    os_err_t sem_errno = 0;
    os_sem_t *iot_sem = *p_semaphore;

    if (!iot_sem)
    {
        CTIOT_LOG(LOG_ERR,"Failed to wait on semaphore %p, invalid argument.", p_semaphore);

        OS_ASSERT( 0 );
    }

    sem_errno = os_sem_wait(iot_sem, OS_WAIT_FOREVER);
    
    if (OS_EOK != sem_errno)
    {   
        CTIOT_LOG(LOG_ERR,"Failed to wait on semaphore %p. errno=%d.", p_semaphore, sem_errno);

        OS_ASSERT( 0 );
    }
}


int ctiot_semaphore_trywait(CTIOT_Semaphore_t * p_semaphore)
{
    int status = true;
    os_err_t sem_errno = 0;
    os_sem_t *iot_sem = *p_semaphore;

    if (!iot_sem)
    {
        CTIOT_LOG(LOG_ERR,"Failed to trywait on semaphore %p, invalid argument.", p_semaphore);

        OS_ASSERT( 0 );
    }

    sem_errno = os_sem_wait(iot_sem, OS_NO_WAIT);
    
    if (OS_EOK != sem_errno)
    {
        CTIOT_LOG(LOG_DEBUG,"Trywait on semaphore %p is not available. errno=%d.", p_semaphore, sem_errno);

        status = false;
    }

    return status;
}


int ctiot_semaphore_timedwait(CTIOT_Semaphore_t * p_semaphore,
                             uint32_t timeout_ms)
{
    int status = true;
    os_err_t sem_errno = 0;
    os_tick_t ticks_to_wait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    os_sem_t *iot_sem = *p_semaphore;

    if (!iot_sem)
    {
        CTIOT_LOG(LOG_ERR,"Failed to Timedwait on semaphore %p, invalid argument.", p_semaphore);

        OS_ASSERT( 0 );
    }

    sem_errno = os_sem_wait(iot_sem, ticks_to_wait);
    
    if (OS_EOK != sem_errno)
    {   
        CTIOT_LOG(LOG_DEBUG,"Timedwait on semaphore %p is not available. errno=%d.", p_semaphore, sem_errno);

        status = false;
    }

    return status;
}


void ctiot_semaphore_post(CTIOT_Semaphore_t * p_semaphore)
{
    os_err_t sem_errno = 0;
    os_sem_t *iot_sem = *p_semaphore;

    if (!iot_sem)
    {
        CTIOT_LOG(LOG_ERR,"Failed to post on semaphore %p, invalid argument.", p_semaphore);
     
        OS_ASSERT( 0 );
    }

    sem_errno = os_sem_post(iot_sem);

    if (OS_EOK != sem_errno)
    {
        CTIOT_LOG(LOG_ERR,"Failed to post to semaphore %p. errno=%d.", p_semaphore, sem_errno);

        OS_ASSERT( 0 );
    }
}


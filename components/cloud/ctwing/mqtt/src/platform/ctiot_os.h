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
 * @file        ctiot_os.h
 * 
 * @brief       Header file for ctiot os interface.
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-11-26   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CTIOT_OS_H
#define __CTIOT_OS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <os_task.h>
#include <os_mutex.h>
#include <os_sem.h>
#include <os_assert.h>
#include <os_clock.h>
#include "ctiot_memory.h"


#define CTIOT_THREAD_IGNORE_STACK_SIZE   ( 2048 ) 

#define CTIOT_THREAD_IGNORE_PRIORITY     ( OS_TASK_PRIORITY_MAX / 2 + OS_TASK_PRIORITY_MAX / 4 )

#define CTIOT_THREAD_IGNORE_TIME_SLICE   ( 5 )

#ifndef CTIOT_THREAD_DEFAULT_PRIORITY
    #define CTIOT_THREAD_DEFAULT_PRIORITY      CTIOT_THREAD_IGNORE_PRIORITY
#endif

#ifndef CTIOT_THREAD_DEFAULT_STACK_SIZE
    #define CTIOT_THREAD_DEFAULT_STACK_SIZE    CTIOT_THREAD_IGNORE_STACK_SIZE
#endif


typedef os_task_t * CTIOT_Thread_t;

typedef os_mutex_t * CTIOT_Mutex_t;

typedef os_sem_t * CTIOT_Semaphore_t;


#define MEM_TEST  /* For print heap memory usage information */

#define OS_GET_MEM(PTR,TYPE,SIZE) {(PTR) = (void*)(long)ctiot_heap_malloc(SIZE);if(PTR != NULL){ctiot_mem_get(SIZE,PTR); /*CTIOT_LOG(LOG_INFO,"OS_GET_MEM %p,size:%u,%s,line:%u",PTR,SIZE,__FILE__,__LINE__)*/;}}
#define OS_PUT_MEM(PTR) {if(PTR != NULL){ if(!ctiot_mem_put(PTR)){ctiot_heap_free(PTR); /*CTIOT_LOG(LOG_INFO,"os_put_mem %p,%s,line:%u",PTR,__FILE__,__LINE__)*/; PTR = NULL;}else{CTIOT_LOG(LOG_WARNING,"OS_PUT_MEM ERROR, %p,%s,line:%u",PTR,__FILE__,__LINE__);}}}


#define OS_SLEEP(MS) os_task_msleep(MS)

int ctiot_mem_get(int mem,void *p);
int ctiot_mem_put(void *p);

void* CTIOT_MALLOC(size_t size);
void CTIOT_FREE(void *p);

int ctiot_thread_create(CTIOT_Thread_t *thread_handler, 
                        void (*thread_routine)(void *), 
                        void *p_argument,
                        int32_t priority,
                        size_t stacksize);
void ctiot_thread_destroy(CTIOT_Thread_t *thread_handler);
int ctiot_mutex_create(CTIOT_Mutex_t * p_mutex, bool recursive);
void ctiot_mutex_destroy(CTIOT_Mutex_t * p_mutex);
void ctiot_mutex_lock(CTIOT_Mutex_t * p_mutex);
int ctiot_mutex_trylock(CTIOT_Mutex_t * p_mutex);
void ctiot_mutex_unlock(CTIOT_Mutex_t * p_mutex);
int ctiot_semaphore_create(CTIOT_Semaphore_t * p_semaphore,
                           uint32_t initial_value,
                           uint32_t max_value);
void ctiot_semaphore_destroy(CTIOT_Semaphore_t * p_semaphore);
uint32_t ctiot_semaphore_getcount(CTIOT_Semaphore_t * p_semaphore);
void ctiot_semaphore_wait(CTIOT_Semaphore_t * p_semaphore);
int ctiot_semaphore_trywait(CTIOT_Semaphore_t * p_semaphore);
int ctiot_semaphore_timedwait(CTIOT_Semaphore_t * p_semaphore,
                              uint32_t timeout_ms);
void ctiot_semaphore_post(CTIOT_Semaphore_t * p_semaphore);

#endif

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
 * @file        pthread_internal.h
 *
 * @brief       Header files used internally by posix pthread.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __PTHREAD_INTERNAL_H__
#define __PTHREAD_INTERNAL_H__

#include <pthread.h>

#ifndef PTHREAD_NUM_MAX
#define PTHREAD_NUM_MAX 32
#endif

#define PTHREAD_MAGIC   0x70746873

struct _pthread_cleanup
{
    void (*cleanup_func)(void *parameter);
    void *parameter;

    struct _pthread_cleanup *next;
};
typedef struct _pthread_cleanup _pthread_cleanup_t;

struct _pthread_key_data
{
    int is_used;
    void (*destructor)(void *parameter);
};
typedef struct _pthread_key_data _pthread_key_data_t;

struct _pthread_data
{
    os_uint32_t    magic;
    pthread_attr_t attr;
    os_task_t      *tid;

    void *(*thread_entry)(void *parameter);
    void *thread_parameter;

    void *return_value;

    /* Semaphore for joinable thread. */
    os_sem_t *joinable_sem;

    /* Cancel state and type. */
    os_uint8_t          cancelstate;
    volatile os_uint8_t canceltype;
    volatile os_uint8_t canceled;

    _pthread_cleanup_t *cleanup;
    
    /* Thread-local storage area */
    void **tls; 
};
typedef struct _pthread_data _pthread_data_t;

int clock_time_to_tick(const struct timespec *time);

void posix_mq_system_init(void);
void posix_sem_system_init(void);
void pthread_key_system_init(void);

#endif

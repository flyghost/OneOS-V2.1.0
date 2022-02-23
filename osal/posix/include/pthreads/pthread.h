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
 * @file        pthread.h
 *
 * @brief       Posix pthread macro, structure definition and function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <oneos_config.h>
/* #include <os_kernel.h> */
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_list.h>
#include <os_util.h>
#include <os_assert.h>
#include <arch_interrupt.h>
#include <os_clock.h>
#include <os_timer.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_mutex.h>
#include <os_event.h>
#include <os_mq.h>
#include <os_mb.h>
#include <os_memory.h>
#include <os_workqueue.h>
#include <libc_ext.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <posix_types.h>
#include <sched.h>

#define PTHREAD_KEY_MAX             8

#define PTHREAD_COND_INITIALIZER    {-1, 0}
#define PTHREAD_RWLOCK_INITIALIZER  {-1, 0}
#define PTHREAD_MUTEX_INITIALIZER   {-1, 0}

#define PTHREAD_CREATE_JOINABLE     0x00
#define PTHREAD_CREATE_DETACHED     0x01

#define PTHREAD_EXPLICIT_SCHED      0
#define PTHREAD_INHERIT_SCHED       1

typedef long pthread_t;
typedef long pthread_condattr_t;
typedef long pthread_rwlockattr_t;
typedef long pthread_mutexattr_t;
typedef long pthread_barrierattr_t;

typedef int pthread_key_t;
typedef int pthread_once_t;

enum
{
    PTHREAD_CANCEL_ASYNCHRONOUS = 0,
    PTHREAD_CANCEL_ENABLE,
    PTHREAD_CANCEL_DEFERRED,
    PTHREAD_CANCEL_DISABLE,
    PTHREAD_CANCELED
};

enum
{
    PTHREAD_MUTEX_NORMAL        = 0,
    PTHREAD_MUTEX_RECURSIVE     = 1,
    PTHREAD_MUTEX_ERRORCHECK    = 2,
    PTHREAD_MUTEX_ERRORCHECK_NP = PTHREAD_MUTEX_ERRORCHECK,
    PTHREAD_MUTEX_RECURSIVE_NP  = PTHREAD_MUTEX_RECURSIVE,
    PTHREAD_MUTEX_DEFAULT       = PTHREAD_MUTEX_NORMAL
};

/* Init value for pthread_once_t */
#define PTHREAD_ONCE_INIT       0

enum
{
    PTHREAD_PRIO_INHERIT =0,
    PTHREAD_PRIO_NONE,
    PTHREAD_PRIO_PROTECT,
};

#define PTHREAD_PROCESS_PRIVATE  0
#define PTHREAD_PROCESS_SHARED   1

#define PTHREAD_SCOPE_PROCESS   0
#define PTHREAD_SCOPE_SYSTEM    1

struct sched_param
{
    int sched_priority;
};

struct pthread_attr
{
    void  *stackaddr;        /* Stack address of thread. */
    int   stacksize;         /* Stack size of thread. */

    int   inheritsched;      /* Inherit parent prio/policy. */
    int   schedpolicy;       /* Scheduler policy. */
    struct sched_param schedparam; /* Sched parameter. */

    int   detachstate;      /* Detach state */
};
typedef struct pthread_attr pthread_attr_t;

struct pthread_mutex
{
    pthread_mutexattr_t attr;
    os_mutex_t          lock;
};
typedef struct pthread_mutex pthread_mutex_t;

struct pthread_cond
{
    pthread_condattr_t attr;
    os_sem_t           sem;
};
typedef struct pthread_cond pthread_cond_t;

struct pthread_rwlock
{
    pthread_rwlockattr_t attr;

    pthread_mutex_t      rw_mutex;          /* Basic lock on this struct. */
    pthread_cond_t       rw_condreaders;    /* For reader threads waiting. */
    pthread_cond_t       rw_condwriters;    /* For writer threads waiting. */

    int rw_nwaitreaders;    /* The number of reader threads waiting. */
    int rw_nwaitwriters;    /* The number of writer threads waiting. */
    int rw_refcount;        /* 0: unlocked, -1: locked by writer, > 0 locked by n readers. */
};
typedef struct pthread_rwlock pthread_rwlock_t;

/* Spinlock implementation, (ADVANCED REALTIME THREADS).*/
struct pthread_spinlock
{
    int lock;
};
typedef struct pthread_spinlock pthread_spinlock_t;

struct pthread_barrier
{
    int             count;
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
};
typedef struct pthread_barrier pthread_barrier_t;

/* pthread thread interface. */
extern int pthread_attr_destroy(pthread_attr_t *attr);
extern int pthread_attr_init(pthread_attr_t *attr);
extern int pthread_attr_setdetachstate(pthread_attr_t *attr, int state);
extern int pthread_attr_getdetachstate(pthread_attr_t const *attr, int *state);
extern int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
extern int pthread_attr_getschedpolicy(pthread_attr_t const *attr, int *policy);
extern int pthread_attr_setschedparam(pthread_attr_t *attr,struct sched_param const *param);
extern int pthread_attr_getschedparam(pthread_attr_t const *attr,struct sched_param *param);
extern int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stack_size);
extern int pthread_attr_getstacksize(pthread_attr_t const *attr, size_t *stack_size);
extern int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack_addr);
extern int pthread_attr_getstackaddr(pthread_attr_t const *attr, void **stack_addr);
extern int pthread_attr_setstack(pthread_attr_t *attr, void *stack_base, size_t stack_size);
extern int pthread_attr_getstack(pthread_attr_t const *attr, void **stack_base, size_t *stack_size);
extern int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guard_size);
extern int pthread_attr_getguardsize(pthread_attr_t const *attr, size_t *guard_size);
extern int pthread_attr_setscope(pthread_attr_t *attr, int scope);
extern int pthread_attr_getscope(pthread_attr_t const *attr);
extern int pthread_system_init(void);
extern int pthread_create (pthread_t *tid, const pthread_attr_t *attr, void *(*start) (void *), void *arg);
extern int pthread_detach (pthread_t thread);
extern int pthread_join (pthread_t thread, void **value_ptr);

OS_INLINE int pthread_equal(pthread_t t1, pthread_t t2)
{
    return t1 == t2;
}

extern pthread_t pthread_self(void);

extern void pthread_exit(void *value_ptr);
extern int pthread_once(pthread_once_t *once_control, void (*init_routine) (void));

/* pthread cleanup */
extern void pthread_cleanup_pop(int execute);
extern void pthread_cleanup_push(void (*routine)(void*), void *arg);

/* pthread cancel */
extern int pthread_cancel(pthread_t thread);
extern void pthread_testcancel(void);
extern int pthread_setcancelstate(int state, int *oldstate);
extern int pthread_setcanceltype(int type, int *oldtype);

extern int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
extern int pthread_kill(pthread_t thread, int sig);

/* pthread mutex interface */
extern int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
extern int pthread_mutex_destroy(pthread_mutex_t *mutex);
extern int pthread_mutex_lock(pthread_mutex_t *mutex);
extern int pthread_mutex_unlock(pthread_mutex_t *mutex);
extern int pthread_mutex_trylock(pthread_mutex_t *mutex);

extern int pthread_mutexattr_init(pthread_mutexattr_t *attr);
extern int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
extern int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
extern int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
extern int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
extern int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared);

/* pthread condition interface */
extern int pthread_condattr_destroy(pthread_condattr_t *attr);
extern int pthread_condattr_init(pthread_condattr_t *attr);

/* ADVANCED REALTIME feature in IEEE Std 1003.1, 2004 Edition */
extern int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id);
extern int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id);
extern int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
extern int pthread_cond_destroy(pthread_cond_t *cond);
extern int pthread_cond_broadcast(pthread_cond_t *cond);
extern int pthread_cond_signal(pthread_cond_t *cond);

extern int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
extern int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

/* pthread rwlock interface */
extern int pthread_rwlockattr_init (pthread_rwlockattr_t *attr);
extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *attr);
extern int pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *attr, int *pshared);
extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *attr, int pshared);

extern int pthread_rwlock_init (pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
extern int pthread_rwlock_destroy (pthread_rwlock_t *rwlock);

extern int pthread_rwlock_rdlock (pthread_rwlock_t *rwlock);
extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock);

extern int pthread_rwlock_timedrdlock (pthread_rwlock_t *rwlock, const struct timespec *abstime);
extern int pthread_rwlock_timedwrlock (pthread_rwlock_t *rwlock, const struct timespec *abstime);

extern int pthread_rwlock_unlock (pthread_rwlock_t *rwlock);

extern int pthread_rwlock_wrlock (pthread_rwlock_t *rwlock);
extern int pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock);

/* pthread spinlock interface */
extern int pthread_spin_init (pthread_spinlock_t *lock, int pshared);
extern int pthread_spin_destroy (pthread_spinlock_t *lock);

extern int pthread_spin_lock (pthread_spinlock_t * lock);
extern int pthread_spin_trylock (pthread_spinlock_t * lock);
extern int pthread_spin_unlock (pthread_spinlock_t * lock);

/* pthread barrier interface */
extern int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
extern int pthread_barrierattr_init(pthread_barrierattr_t *attr);
extern int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared);
extern int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared);

extern int pthread_barrier_destroy(pthread_barrier_t *barrier);
extern int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count);
extern int pthread_barrier_wait(pthread_barrier_t *barrier);

extern void *pthread_getspecific(pthread_key_t key);
extern int pthread_setspecific(pthread_key_t key, const void *value);
extern int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
extern int pthread_key_delete(pthread_key_t key);

extern unsigned int sleep(unsigned int seconds);

#ifdef __cplusplus
}
#endif

#endif

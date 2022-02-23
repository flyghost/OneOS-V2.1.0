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
 * @file        pthread_attr.c
 *
 * @brief       This file provides posix pthread attribute functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>
#include <sched.h>
#include <string.h>

#define DEFAULT_STACK_SIZE  2048
#define DEFAULT_PRIORITY    (OS_TASK_PRIORITY_MAX / 2 + OS_TASK_PRIORITY_MAX / 4)

const pthread_attr_t g_pthread_default_attr = 
{
    0,                          /* Stack base. */
    DEFAULT_STACK_SIZE,         /* Stack size. */

    PTHREAD_INHERIT_SCHED,      /* Inherit parent prio/policy. */
    SCHED_FIFO,                 /* Scheduler policy. */
    {
        DEFAULT_PRIORITY,       /* Scheduler priority. */
    },
    PTHREAD_CREATE_JOINABLE,    /* Detach state. */
};

int pthread_attr_init(pthread_attr_t *attr)
{
    OS_ASSERT(OS_NULL != attr);

    *attr = g_pthread_default_attr;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_init);

int pthread_attr_destroy(pthread_attr_t *attr)
{
    OS_ASSERT(OS_NULL != attr);

    memset(attr, 0, sizeof(pthread_attr_t));

    return 0;
}
EXPORT_SYMBOL(pthread_attr_destroy);

int pthread_attr_setdetachstate(pthread_attr_t *attr, int state)
{
    OS_ASSERT(OS_NULL != attr);

    if ((PTHREAD_CREATE_JOINABLE != state) && (PTHREAD_CREATE_DETACHED != state))
    {
        return EINVAL;
    }

    attr->detachstate = state;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_setdetachstate);

int pthread_attr_getdetachstate(pthread_attr_t const *attr, int *state)
{
    OS_ASSERT(OS_NULL != attr);

    *state = (int)attr->detachstate;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_getdetachstate);

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    OS_ASSERT(OS_NULL != attr);

    attr->schedpolicy = policy;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_setschedpolicy);

int pthread_attr_getschedpolicy(pthread_attr_t const *attr, int *policy)
{
    OS_ASSERT(OS_NULL != attr);

    *policy = (int)attr->schedpolicy;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_getschedpolicy);

int pthread_attr_setschedparam(pthread_attr_t *attr, struct sched_param const *param)
{
    OS_ASSERT(OS_NULL != attr);
    OS_ASSERT(OS_NULL != param);

    attr->schedparam.sched_priority = param->sched_priority;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_setschedparam);

int pthread_attr_getschedparam(pthread_attr_t const *attr, struct sched_param *param)
{
    OS_ASSERT(OS_NULL != attr);
    OS_ASSERT(OS_NULL != param);

    param->sched_priority = attr->schedparam.sched_priority;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_getschedparam);

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stack_size)
{
    OS_ASSERT(OS_NULL != attr);

    attr->stacksize = stack_size;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_setstacksize);

int pthread_attr_getstacksize(pthread_attr_t const *attr, size_t *stack_size)
{
    OS_ASSERT(OS_NULL != attr);

    *stack_size = attr->stacksize;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_getstacksize);

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack_addr)
{
    OS_ASSERT(OS_NULL != attr);

    return EOPNOTSUPP;
}
EXPORT_SYMBOL(pthread_attr_setstackaddr);

int pthread_attr_getstackaddr(pthread_attr_t const *attr, void **stack_addr)
{
   OS_ASSERT(OS_NULL != attr);

    return EOPNOTSUPP;
}
EXPORT_SYMBOL(pthread_attr_getstackaddr);

int pthread_attr_setstack(pthread_attr_t *attr, void *stack_base, size_t stack_size)
{
    OS_ASSERT(OS_NULL != attr);

    attr->stackaddr = stack_base;
    attr->stacksize = OS_ALIGN_DOWN(stack_size, OS_ALIGN_SIZE);

    return 0;
}
EXPORT_SYMBOL(pthread_attr_setstack);

int pthread_attr_getstack(pthread_attr_t const *attr, void **stack_base, size_t *stack_size)
{
    OS_ASSERT(OS_NULL != attr);

    *stack_base = attr->stackaddr;
    *stack_size = attr->stacksize;

    return 0;
}
EXPORT_SYMBOL(pthread_attr_getstack);

int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guard_size)
{
    return EOPNOTSUPP;
}

int pthread_attr_getguardsize(pthread_attr_t const *attr, size_t *guard_size)
{
    return EOPNOTSUPP;
}
EXPORT_SYMBOL(pthread_attr_getguardsize);

int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    if (PTHREAD_SCOPE_SYSTEM == scope)
    {
        return 0;
    }
    
    if (PTHREAD_SCOPE_PROCESS == scope)
    {
        return EOPNOTSUPP;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_attr_setscope);

int pthread_attr_getscope(pthread_attr_t const *attr)
{
    return PTHREAD_SCOPE_SYSTEM;
}
EXPORT_SYMBOL(pthread_attr_getscope);

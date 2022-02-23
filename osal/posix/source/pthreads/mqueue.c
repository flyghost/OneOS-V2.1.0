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
 * @file        mqueue.c
 *
 * @brief       This file implements the posix message queue functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <libc_ext.h>
#include <fcntl.h>
#include <mqueue.h>
#include "pthread_internal.h"

static mqd_t gs_posix_mq_list = OS_NULL;
static os_sem_t gs_posix_mq_lock;

void posix_mq_system_init()
{
    os_sem_init(&gs_posix_mq_lock, "pmq", 1, 1); // TODO: need check
}

OS_INLINE void posix_mq_insert(mqd_t pmq)
{
    pmq->next = gs_posix_mq_list;
    gs_posix_mq_list = pmq;
}

static void posix_mq_delete(mqd_t pmq)
{
    mqd_t iter;
    
    if (gs_posix_mq_list == pmq)
    {
        gs_posix_mq_list = pmq->next;

        os_mq_destroy(pmq->mq);
        os_free(pmq);

        return;
    }
    
    for (iter = gs_posix_mq_list; iter->next != OS_NULL; iter = iter->next)
    {
        if (iter->next == pmq)
        {
            /* Delete this mq. */
            if (OS_NULL != pmq->next)
            {
                iter->next = pmq->next;
            }
            else
            {
                iter->next = OS_NULL;
            }

            /* Destroy OneOS mqueue. */
            os_mq_destroy(pmq->mq);
            os_free(pmq);

            return ;
        }
    }
}

static mqd_t posix_mq_find(const char* name)
{
    mqd_t iter;
    /* os_object_t *object; */

    for (iter = gs_posix_mq_list; iter != OS_NULL; iter = iter->next)
    {
        /* object = (os_object_t *)(iter->mq); */

        if (strncmp(iter->mq->name, name, OS_NAME_MAX) == 0)
        {
            return iter;
        }
    }

    return OS_NULL;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
    os_set_errno(OS_ERROR);

    return -1;
}
EXPORT_SYMBOL(mq_setattr);

int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
    if ((OS_NULL == mqdes) || (OS_NULL == mqstat))
    {
        os_set_errno(EBADF);

        return -1;
    }

    mqstat->mq_maxmsg  = mqdes->mq->queue_depth; // TODO: need check
    mqstat->mq_msgsize = mqdes->mq->max_msg_size;
    mqstat->mq_curmsgs = 0;
    mqstat->mq_flags   = 0;

    return 0;
}
EXPORT_SYMBOL(mq_getattr);

mqd_t mq_open(const char *name, int oflag, ...)
{
    mqd_t mqdes;
    va_list arg;
    mode_t mode;
    struct mq_attr *attr;

    /* Lock posix mqueue list. */
    os_sem_wait(&gs_posix_mq_lock, OS_WAIT_FOREVER);

    attr  = OS_NULL;
    mqdes = OS_NULL;
    
    if (oflag & O_CREAT)
    {
        va_start(arg, oflag);
        mode = (mode_t)va_arg(arg, unsigned int);
        mode = mode;
        attr = (struct mq_attr *)va_arg(arg, struct mq_attr *);
        va_end(arg);

        if (oflag & O_EXCL)
        {
            if (OS_NULL != posix_mq_find(name))
            {
                os_set_errno(EEXIST);
                goto __return;
            }
        }
        
        mqdes = (mqd_t)os_malloc(sizeof(struct mqdes));
        if (OS_NULL == mqdes)
        {
            os_set_errno(ENFILE);
            goto __return;
        }

        /* Create OneOS message queue. */
        mqdes->mq = os_mq_create(name, attr->mq_msgsize, attr->mq_maxmsg);
        if (OS_NULL == mqdes->mq)
        {
            os_set_errno(ENFILE);
            goto __return;
        }
        
        /* Initialize reference count. */
        mqdes->refcount = 1;
        mqdes->unlinked = 0;

        /* Insert mq to posix mq list. */
        posix_mq_insert(mqdes);
    }
    else
    {
        mqdes = posix_mq_find(name);
        if (OS_NULL != mqdes)
        {
            mqdes->refcount++;
        }
        else
        {
            os_set_errno(ENOENT);
            goto __return;
        }
    }
    
    os_sem_post(&gs_posix_mq_lock);

    return mqdes;

__return:
    /* Release lock. */
    os_sem_post(&gs_posix_mq_lock);

    /* Release allocated memory. */
    if (OS_NULL != mqdes)
    {
        if (OS_NULL != mqdes->mq)
        {
            /* Destroy OneOS message queue. */
            os_mq_destroy(mqdes->mq);
        }
        
        os_free(mqdes);
    }
    return OS_NULL;
}
EXPORT_SYMBOL(mq_open);

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio)
{
    os_err_t  result;
    os_size_t recv_len;

    if ((OS_NULL == mqdes) || (OS_NULL == msg_ptr))
    {
        os_set_errno(EINVAL);

        return -1;
    }

    result = os_mq_recv(mqdes->mq, msg_ptr, msg_len, OS_WAIT_FOREVER, &recv_len);
    if (OS_EOK == result)
    {
        return recv_len;
    }

    os_set_errno(EBADF);
    return -1;
}
EXPORT_SYMBOL(mq_receive);

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio)
{
    os_err_t result;

    if ((OS_NULL == mqdes) || (OS_NULL == msg_ptr))
    {
        os_set_errno(EINVAL);

        return -1;
    }

    result = os_mq_send(mqdes->mq, (void*)msg_ptr, msg_len, 0);
    if (OS_EOK == result)
    {
        return 0;
    }

    os_set_errno(EBADF);

    return -1;
}
EXPORT_SYMBOL(mq_send);

ssize_t mq_timedreceive(mqd_t                   mqdes,
                        char                   *msg_ptr,
                        size_t                  msg_len,
                        unsigned               *msg_prio,
                        const struct timespec  *abs_timeout)
{
    int       tick;
    os_err_t  result;
    os_size_t recv_len;

    if ((OS_NULL == mqdes) || (OS_NULL == msg_ptr))
    {
        os_set_errno(EINVAL);

        return -1;
    }

    tick = clock_time_to_tick(abs_timeout);

    result = os_mq_recv(mqdes->mq, msg_ptr, msg_len, tick, &recv_len);
    if (OS_EOK == result)
    {
        return recv_len;
    }

    if (OS_ETIMEOUT == result)
    {
        os_set_errno(ETIMEDOUT);
    }
    else
    {
        os_set_errno(EBADMSG);
    }

    return -1;
}
EXPORT_SYMBOL(mq_timedreceive);

int mq_timedsend(mqd_t                  mqdes,
                 const char            *msg_ptr,
                 size_t                 msg_len,
                 unsigned               msg_prio,
                 const struct timespec *abs_timeout)
{
    int tick;
    os_err_t result;

    if ((OS_NULL == mqdes) || (OS_NULL == msg_ptr))
    {
        os_set_errno(EINVAL);

        return -1;
    }

    tick = clock_time_to_tick(abs_timeout);

    result = os_mq_send(mqdes->mq, (void*)msg_ptr, msg_len, tick);
    if (OS_EOK == result)
    {
        return 0;
    }

    if (OS_ETIMEOUT == result)
    {
        os_set_errno(ETIMEDOUT);
    }
    else
    {
        os_set_errno(EBADMSG);
    }

    return -1;

}
EXPORT_SYMBOL(mq_timedsend);

int mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
    os_set_errno(OS_ERROR);

    return -1;
}
EXPORT_SYMBOL(mq_notify);

int mq_close(mqd_t mqdes)
{
    if (OS_NULL == mqdes)
    {
        os_set_errno(EINVAL);

        return -1;
    }

    /* Lock posix mqueue list. */
    os_sem_wait(&gs_posix_mq_lock, OS_WAIT_FOREVER);
    
    mqdes->refcount--;
    if(0 == mqdes->refcount)
    {
        /* Delete from posix mqueue list. */
        if (mqdes->unlinked)
        {
            posix_mq_delete(mqdes);
        }
    }
    
    os_sem_post(&gs_posix_mq_lock);

    return 0;
}
EXPORT_SYMBOL(mq_close);

int mq_unlink(const char *name)
{
    mqd_t pmq;

    /* Lock posix mqueue list. */
    os_sem_wait(&gs_posix_mq_lock, OS_WAIT_FOREVER);
    
    pmq = posix_mq_find(name);
    if (OS_NULL != pmq)
    {
        pmq->unlinked = 1;
        if (pmq->refcount == 0)
        {
            /* Delete this mqueue. */
            posix_mq_delete(pmq);
        }
        
        os_sem_post(&gs_posix_mq_lock);

        return 0;
    }
    
    os_sem_post(&gs_posix_mq_lock);

    /* No this entry. */
    os_set_errno(ENOENT);

    return -1;
}
EXPORT_SYMBOL(mq_unlink);

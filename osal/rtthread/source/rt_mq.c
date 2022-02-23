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
 * @file        rt_mq.c
 *
 * @brief       Implementation of RT-Thread adaper messagequeue function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "os_assert.h"
#include "rtthread.h"
#include "os_errno.h"

#ifdef RT_USING_MESSAGEQUEUE
rt_err_t rt_mq_init(rt_mq_t     mq,
                    const char *name,
                    void       *msgpool,
                    rt_size_t   msg_size,
                    rt_size_t   pool_size,
                    rt_uint8_t  flag)
{
    os_err_t ret;

    OS_ASSERT(mq);
    OS_ASSERT(msgpool);
    OS_ASSERT(msg_size >= OS_ALIGN_SIZE);
    OS_ASSERT(pool_size >= (OS_ALIGN_SIZE + sizeof(os_mq_msg_t)));

    ret = os_mq_init(&mq->os_mq, name, msgpool, pool_size, msg_size);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    if (RT_IPC_FLAG_PRIO == flag)
    {
        ret = os_mq_set_wake_type(&mq->os_mq, OS_MQ_WAKE_TYPE_PRIO);
    }
    else
    {
        ret = os_mq_set_wake_type(&mq->os_mq, OS_MQ_WAKE_TYPE_FIFO);
    }
    if (OS_EOK != ret)
    {
         return -RT_ERROR;
    }

    mq->is_static  = OS_TRUE;

    return RT_EOK;
}

rt_err_t rt_mq_detach(rt_mq_t mq)
{
    os_err_t ret;

    OS_ASSERT(mq);
    OS_ASSERT(OS_TRUE == mq->is_static);

    ret = os_mq_deinit(&mq->os_mq);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

#ifdef RT_USING_HEAP
rt_mq_t rt_mq_create(const char *name,
                     rt_size_t   msg_size,
                     rt_size_t   max_msgs,
                     rt_uint8_t  flag)
{
    rt_mq_t    mq;
    os_size_t  pool_size;
    os_size_t  align_msg_size;
    os_err_t   ret;
    void      *msgpool;

    OS_ASSERT(msg_size >= OS_ALIGN_SIZE);
    OS_ASSERT(max_msgs > 0);

    mq = (rt_mq_t)os_malloc(sizeof(struct rt_messagequeue));
    if(OS_NULL == mq)
    {
        return RT_NULL;
    }

    align_msg_size  = OS_ALIGN_UP(msg_size, OS_ALIGN_SIZE);
    pool_size       = max_msgs * (align_msg_size + sizeof(os_mq_msg_t));

    msgpool = os_malloc(pool_size);
    if(OS_NULL == msgpool)
    {
        os_free(mq);
        return RT_NULL;
    }

    ret = os_mq_init(&mq->os_mq, name, msgpool, pool_size, msg_size);
    if(OS_EOK != ret)
    {
        os_free(msgpool);
        os_free(mq);

        return RT_NULL;
    }

    mq->is_static  = OS_FALSE;
    mq->start_addr = msgpool;

    return mq;
}

rt_err_t rt_mq_delete(rt_mq_t mq)
{
    os_err_t   ret;

    OS_ASSERT(mq);
    OS_ASSERT(OS_FALSE == mq->is_static);
    OS_ASSERT(mq->start_addr);

    ret = os_mq_deinit(&mq->os_mq);
    os_free(mq->start_addr);
    os_free(mq);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }
    return RT_EOK;
}
#endif

rt_err_t rt_mq_send(rt_mq_t mq, void *buffer, rt_size_t size)
{
    os_err_t   ret;

    OS_ASSERT(mq);
    OS_ASSERT(buffer);
    OS_ASSERT(size);

    ret = os_mq_send(&mq->os_mq, buffer, size, OS_NO_WAIT);
    if (OS_EOK != ret)
    {
        if (OS_EFULL == ret)
        {
            return -RT_EFULL;
        }

        return RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_mq_urgent(rt_mq_t mq, void *buffer, rt_size_t size)
{
    os_err_t   ret;

    OS_ASSERT(mq);
    OS_ASSERT(buffer);
    OS_ASSERT(size);

    ret = os_mq_send_urgent(&mq->os_mq, buffer, size, OS_NO_WAIT);
    if (OS_EOK != ret)
    {
       if (OS_EFULL == ret)
       {
           return -RT_EFULL;
       }

       return RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_mq_recv(rt_mq_t    mq,
                    void      *buffer,
                    rt_size_t  size,
                    rt_int32_t timeout)
{
    os_size_t recv_size;
    os_tick_t timeout_tmp;
    os_err_t  ret;

    OS_ASSERT(mq);
    OS_ASSERT(buffer);
    OS_ASSERT(size);

    /*For OneOS,only support -1 for timeout,so set timeout is -1 when timeout is less than zero*/
    if (timeout < 0)
    {
        timeout_tmp = OS_WAIT_FOREVER;
    }
    else if (0 == timeout)
    {
        timeout_tmp = OS_NO_WAIT;
    }
    else
    {
        timeout_tmp = (os_tick_t)timeout;
    }

    ret = os_mq_recv(&mq->os_mq, buffer, size, timeout_tmp, &recv_size);
    if (OS_EOK != ret)
    {
        if (OS_EEMPTY == ret || OS_ETIMEOUT == ret)
        {
            return -RT_ETIMEOUT;
        }

        return -RT_ERROR;
    }

    return RT_EOK;
}
#endif /* RT_USING_MESSAGEQUEUE */


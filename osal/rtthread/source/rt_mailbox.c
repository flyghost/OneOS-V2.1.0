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
 * @file        rt_mailbox.c
 *
 * @brief       Implementation of RT-Thread adaper mailbox function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "os_assert.h"
#include "rtthread.h"
#include "os_errno.h"

#ifdef RT_USING_MAILBOX
rt_err_t rt_mb_init(rt_mailbox_t mb,
                       const char  *name,
                       void        *msgpool,
                       rt_size_t    size,
                       rt_uint8_t   flag)
{
    os_err_t ret;

    OS_ASSERT(mb);
    OS_ASSERT(msgpool);

    ret = os_mb_init(&mb->os_mb, name, msgpool, size);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    if (RT_IPC_FLAG_PRIO == flag)
    {
        ret = os_mb_set_wake_type(&mb->os_mb, OS_MB_WAKE_TYPE_PRIO);
    }
    else
    {
        ret = os_mb_set_wake_type(&mb->os_mb, OS_MB_WAKE_TYPE_FIFO);
    }
    if (OS_EOK != ret)
    {
         return -RT_ERROR;
    }

    mb->is_static  = OS_TRUE;

    return RT_EOK;
}

rt_err_t rt_mb_detach(rt_mailbox_t mb)
{
    os_err_t ret;

    OS_ASSERT(mb);
    OS_ASSERT(OS_TRUE == mb->is_static);

    ret = os_mb_deinit(&mb->os_mb);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

#ifdef RT_USING_HEAP
rt_mailbox_t rt_mb_create(const char *name, rt_size_t size, rt_uint8_t flag)
{
    void         *msg_pool;
    os_err_t      ret;
    os_size_t     pool_size;
    rt_mailbox_t  mailbox;

    mailbox = (rt_mailbox_t)os_malloc(sizeof(struct rt_mailbox));
    if (OS_NULL == mailbox)
    {
        return RT_NULL;
    }

    /* Calculates the size of the requested mailbox */
    pool_size = size * sizeof(os_ubase_t);
    msg_pool = os_malloc(pool_size);
    if (OS_NULL == msg_pool)
    {
        os_free(mailbox);
        return RT_NULL;
    }

    ret = os_mb_init(&mailbox->os_mb, name, msg_pool, pool_size);
    if (OS_EOK != ret)
    {
        os_free(mailbox);
        os_free(msg_pool);

        return RT_NULL;
    }

    mailbox->is_static  = OS_FALSE;
    mailbox->start_addr = msg_pool;

    return mailbox;
}

rt_err_t rt_mb_delete(rt_mailbox_t mb)
{
    os_err_t      ret;

    OS_ASSERT(mb);
    OS_ASSERT(OS_FALSE == mb->is_static);
    OS_ASSERT(mb->start_addr);

    ret = os_mb_deinit(&mb->os_mb);
    os_free(mb->start_addr);
    os_free(mb);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}
#endif /* RT_USING_HEAP */

rt_err_t rt_mb_send(rt_mailbox_t mb, rt_uint32_t value)
{
    return rt_mb_send_wait(mb, value, 0);
}

rt_err_t rt_mb_send_wait(rt_mailbox_t mb,
                                rt_uint32_t  value,
                                rt_int32_t   timeout)
{
    os_err_t  ret;
    os_tick_t timeout_tmp;

    OS_ASSERT(mb);

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

    ret = os_mb_send(&mb->os_mb, value, timeout_tmp);
    if (OS_EOK != ret)
    {
        if (OS_EFULL == ret)
        {
            return -RT_EFULL;
        }

        if (OS_ETIMEOUT == ret)
        {
            return -RT_ETIMEOUT;
        }
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_mb_recv(rt_mailbox_t mb, rt_uint32_t *value, rt_int32_t timeout)
{
    os_err_t  ret;
    os_tick_t timeout_tmp;

    OS_ASSERT(mb);
    OS_ASSERT(value);

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

    ret = os_mb_recv(&mb->os_mb, (os_ubase_t *)value, timeout_tmp);
    if (RT_EOK != ret)
    {
        if (OS_EEMPTY == ret || OS_ETIMEOUT == ret)
        {
            return -RT_ETIMEOUT;
        }

        return -RT_ERROR;
    }

    return RT_EOK;
}
#endif /* RT_USING_MAILBOX */


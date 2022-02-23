/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * Copyright (c) 2006-2018, RT-Thread Development Team.
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
 * @file        ring_blk_buff.c
 *
 * @brief       This function implements a ring block buffer.
 *
 * @details     The ring block buffer(rbb) is the ring buffer which is composed with many blocks. It is different from
 *              the ring buffer. The ring buffer is only composed with chars. The rbb put and get supported zero copies.
 *              So the rbb is very suitable for put block and get block by a certain order. Such as DMA block transmit,
 *              communicate frame send/recv, and so on.
 *
 * @revision
 * Date         Author          Notes
 * 2018-08-25   armink          The first version.
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_spinlock.h>
#include <os_list.h>
#include <os_memory.h>
#include <arch_interrupt.h>
#include <ring_blk_buff.h>

static rbb_blk_t *rbb_find_empty_blk_in_set(rbb_ctrl_info_t *rbb)
{
    rbb_blk_t  *unused_block;
    os_size_t   index;

    unused_block = OS_NULL;
    for (index = 0; index < rbb->blk_max_num; index++)
    {
        if (RBB_BLK_STATUS_UNUSED == rbb->blk_set[index].status)
        {
            unused_block = &rbb->blk_set[index];
            break;
        }
    }

    return unused_block;
}

static void rbb_do_init(rbb_ctrl_info_t   *rbb,
                        os_uint8_t        *buf,
                        os_size_t          buf_size,
                        rbb_blk_t         *block_set,
                        os_size_t          blk_max_num)
{
    os_size_t index;

    rbb->buf         = buf;
    rbb->buf_size    = buf_size;
    rbb->blk_set     = block_set;
    rbb->blk_max_num = blk_max_num;

    os_list_init(&rbb->blk_list_head);
    os_spin_lock_init(&rbb->rbb_locker);

    /* Initialize block status */
    for (index = 0; index < blk_max_num; index++)
    {
        block_set[index].status = RBB_BLK_STATUS_UNUSED;
    }

    return;
}

void rbb_init(rbb_ctrl_info_t   *rbb,
              os_uint8_t        *buf,
              os_size_t          buf_size,
              rbb_blk_t         *block_set,
              os_size_t          blk_max_num)
{
    OS_ASSERT(OS_NULL != rbb);
    OS_ASSERT(OS_NULL != buf);
    OS_ASSERT(OS_NULL != block_set);
    OS_ASSERT(buf_size > 0);
    OS_ASSERT(blk_max_num > 0);

    rbb_do_init(rbb, buf, buf_size, block_set, blk_max_num);

    return;
}

#ifdef OS_USING_SYS_HEAP
rbb_ctrl_info_t *rbb_create(os_size_t buf_size, os_size_t blk_max_num)
{
    rbb_ctrl_info_t *rbb;
    os_uint8_t      *buf;
    rbb_blk_t       *blk_set;

    rbb     = (rbb_ctrl_info_t *)os_malloc(sizeof(rbb_ctrl_info_t));
    buf     = (os_uint8_t *)os_malloc(buf_size);
    blk_set = (rbb_blk_t *)os_malloc(sizeof(rbb_blk_t) * blk_max_num);

    if ((OS_NULL == rbb) || (OS_NULL == buf) || (OS_NULL == blk_set))
    {
        if (OS_NULL != rbb)
        {
            os_free(rbb);
            rbb = OS_NULL;
        }

        if (OS_NULL != buf)
        {
            os_free(buf);
            buf = OS_NULL;
        }

        if (OS_NULL != blk_set)
        {
            os_free(blk_set);
            blk_set = OS_NULL;
        }
    }
    else
    {
        rbb_do_init(rbb, buf, buf_size, blk_set, blk_max_num);
    }

    return rbb;
}

void rbb_destroy(rbb_ctrl_info_t *rbb)
{
    OS_ASSERT(OS_NULL != rbb);

    os_free(rbb->blk_set);
    os_free(rbb->buf);
    os_free(rbb);

    return;
}
#endif /* OS_USING_SYS_HEAP */

os_size_t rbb_get_buf_size(rbb_ctrl_info_t *rbb)
{
    OS_ASSERT(OS_NULL != rbb);

    return rbb->buf_size;
}

rbb_blk_t *rbb_blk_alloc(rbb_ctrl_info_t *rbb, os_size_t alloc_size)
{
    os_ubase_t irq_save;
    os_size_t  empty1;
    os_size_t  empty2;
    rbb_blk_t *head;
    rbb_blk_t *tail;
    rbb_blk_t *new_blk;

    OS_ASSERT(OS_NULL != rbb);
    OS_ASSERT(alloc_size > 0);

    os_spin_lock_irqsave(&rbb->rbb_locker, &irq_save);

    new_blk = rbb_find_empty_blk_in_set(rbb);
    
    if (OS_NULL != new_blk)
    {
        if (!os_list_empty(&rbb->blk_list_head))
        {
            head = os_list_first_entry(&rbb->blk_list_head, rbb_blk_t, list_node);
            tail = os_list_tail_entry(&rbb->blk_list_head, rbb_blk_t, list_node);

            if (head->buf <= tail->buf)
            {
                /**
                 *                      head                     tail
                 * +--------------------------------------+-----------------+------------------+
                 * |      empty2     | block1 |   block2  |      block3     |       empty1     |
                 * +--------------------------------------+-----------------+------------------+
                 *                            rbb->buf
                 */
                empty1 = (rbb->buf + rbb->buf_size) - (tail->buf + tail->buf_size);
                empty2 = head->buf - rbb->buf;

                if (empty1 >= alloc_size)
                {
                    new_blk->status   = RBB_BLK_STATUS_INITED;
                    new_blk->buf      = tail->buf + tail->buf_size;
                    new_blk->buf_size = alloc_size;

                    os_list_add_tail(&rbb->blk_list_head, &new_blk->list_node);
                }
                else
                {
                    if (empty2 >= alloc_size)
                    {
                        new_blk->status   = RBB_BLK_STATUS_INITED;
                        new_blk->buf      = rbb->buf;
                        new_blk->buf_size = alloc_size;

                        os_list_add_tail(&rbb->blk_list_head, &new_blk->list_node);
                    }
                    else
                    {
                        /* No space */
                        new_blk = OS_NULL;
                    }
                }
            }
            else
            {
                /**
                 *        tail                                              head
                 * +----------------+-------------------------------------+--------+-----------+
                 * |     block3     |                empty1               | block1 |  block2   |
                 * +----------------+-------------------------------------+--------+-----------+
                 *                            rbb->buf
                 */
                empty1 = head->buf - (tail->buf + tail->buf_size);
                
                if (empty1 >= alloc_size)
                {
                    new_blk->status   = RBB_BLK_STATUS_INITED;
                    new_blk->buf      = tail->buf + tail->buf_size;
                    new_blk->buf_size = alloc_size;

                    os_list_add_tail(&rbb->blk_list_head, &new_blk->list_node);
                }
                else
                {
                    /* No space */
                    new_blk = OS_NULL;
                }
            }
        }
        else
        {
            /* The list is empty */
            os_list_add_tail(&rbb->blk_list_head, &new_blk->list_node);
            new_blk->status   = RBB_BLK_STATUS_INITED;
            new_blk->buf      = rbb->buf;
            new_blk->buf_size = alloc_size;
        }
    }

    os_spin_unlock_irqrestore(&rbb->rbb_locker, irq_save);

    return new_blk;
}

void rbb_blk_put(rbb_blk_t *block)
{
    OS_ASSERT(OS_NULL != block);
    OS_ASSERT(RBB_BLK_STATUS_INITED == block->status);

    block->status = RBB_BLK_STATUS_PUT;
    return;
}

rbb_blk_t *rbb_blk_get(rbb_ctrl_info_t *rbb)
{
    os_ubase_t  irq_save;
    rbb_blk_t  *block;

    OS_ASSERT(OS_NULL != rbb);

    os_spin_lock_irqsave(&rbb->rbb_locker, &irq_save);

    if (os_list_empty(&rbb->blk_list_head))
    {
        block = OS_NULL;
    }
    else
    {
        block = os_list_first_entry(&rbb->blk_list_head, rbb_blk_t, list_node);
        if (RBB_BLK_STATUS_PUT == block->status)
        {
            block->status = RBB_BLK_STATUS_GET;
        }
        else
        {
            block = OS_NULL;
        }
    }

    os_spin_unlock_irqrestore(&rbb->rbb_locker, irq_save);

    return block;
}

void rbb_blk_free(rbb_ctrl_info_t *rbb, rbb_blk_t *block)
{
    os_ubase_t irq_save;

    OS_ASSERT(OS_NULL != rbb);
    OS_ASSERT(OS_NULL != block);
    OS_ASSERT(RBB_BLK_STATUS_GET == block->status);

    os_spin_lock_irqsave(&rbb->rbb_locker, &irq_save);

    /* Remove it from rbb block list */
    os_list_del(&block->list_node);
    block->status = RBB_BLK_STATUS_UNUSED;

    os_spin_unlock_irqrestore(&rbb->rbb_locker, irq_save);

    return;
}


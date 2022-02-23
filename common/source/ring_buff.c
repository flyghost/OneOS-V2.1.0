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
 * @file        ring_buff.c
 *
 * @brief       This function implements a ring buffer.
 *
 * @revision
 * Date         Author          Notes
 * 2012-09-30   Bernard         First version.
 * 2013-05-08   Grissiom        Reimplement
 * 2016-08-18   Heyuanjie       Add interface
 * 2020-04-08   OneOS Team      Modify Ring buffer control block structrue.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>
#include <ring_buff.h>

typedef enum
{
    RB_RING_BUFF_EMPTY,
    RB_RING_BUFF_FULL,
    RB_RING_BUFF_HALF_FULL,     /* Half full is neither full nor empty */
}rb_ring_buff_state_t;

static rb_ring_buff_state_t rb_ring_buff_state(rb_ring_buff_t *rb)
{
    if (rb->read_index == rb->write_index)
    {
        if (rb->read_mirror == rb->write_mirror)
        {
            return RB_RING_BUFF_EMPTY;
        }
        else
        {
            return RB_RING_BUFF_FULL;
        }
    }
    
    return RB_RING_BUFF_HALF_FULL;
}

/**
 ***********************************************************************************************************************
 * @brief           Initialize ring buffer.
 *
 * @param[in]       rb              Ring buffer control block
 * @param[in]       pool            The pool use as ring buffer.
 * @param[in]       pool_size       The pool size
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rb_ring_buff_init(rb_ring_buff_t *rb, os_uint8_t *pool, os_uint32_t pool_size)
{
    OS_ASSERT(rb && pool && pool_size);

    rb->read_index   = 0;
    rb->write_index  = 0;
    
    rb->read_mirror  = OS_FALSE;
    rb->write_mirror = OS_FALSE;
    
    rb->buff         = pool;
    rb->buff_size    = pool_size;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Empty the ring buffer.
 *
 * @param[in]       rb              Ring buffer control block
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rb_ring_buff_reset(rb_ring_buff_t *rb)
{
    OS_ASSERT(rb != OS_NULL);

    rb->read_index   = 0;
    rb->write_index  = 0;
    
    rb->read_mirror  = OS_FALSE;
    rb->write_mirror = OS_FALSE;

    return;    
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           Create ring buffer.
 *
 * @param[in]       size            The ring buffer size to be created.
 *
 * @return          Ring buffer control block.
 * @retval          OS_NULL         Create ring buffer failed.
 * @retval          else            Create ring buffer success.
 ***********************************************************************************************************************
 */
rb_ring_buff_t *rb_ring_buff_create(os_uint32_t size)
{
    rb_ring_buff_t *rb;
    os_uint8_t     *pool;

	OS_ASSERT(size > 0);

    rb   = OS_NULL;
    pool = OS_NULL;
    
    do
    {
        size = OS_ALIGN_DOWN(size, OS_ALIGN_SIZE);

        rb = os_malloc(sizeof(rb_ring_buff_t));
        if (rb == OS_NULL)
        {
            break;
        }

        pool = os_malloc(size);
        if (pool == OS_NULL)
        {
            os_free(rb);
            rb = OS_NULL;
            
            break;
        }
        
        rb_ring_buff_init(rb, pool, size);
        
    } while (0);
    
    return rb;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy ring buffer.
 *
 * @param[in]       rb              Ring buffer control block
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void rb_ring_buff_destroy(rb_ring_buff_t *rb)
{
    OS_ASSERT(rb != OS_NULL);

    os_free(rb->buff);
    os_free(rb);

    return;
}
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           Get the size of data in ring buffer.
 *
 * @param[in]       rb              Ring buffer control block
 *
 * @return          The size of data.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_data_len(rb_ring_buff_t *rb)
{
    rb_ring_buff_state_t state;
    os_uint32_t          data_len;
    
    OS_ASSERT(rb != OS_NULL);

    data_len = 0;
    
    state = rb_ring_buff_state(rb);
    switch (state)
    {
    case RB_RING_BUFF_EMPTY:
        data_len = 0;
        break;
    case RB_RING_BUFF_FULL:
        data_len = rb->buff_size;
        break;
    case RB_RING_BUFF_HALF_FULL:
    default:
        if (rb->write_index > rb->read_index)
        {
            data_len = rb->write_index - rb->read_index;
        }
        else
        {
            data_len = rb->buff_size - (rb->read_index - rb->write_index);
        }

        break;
    };

    return data_len;
}

/**
 ***********************************************************************************************************************
 * @brief           Get free size in ring buffer.
 *
 * @param[in]       rb              Ring buffer control block
 *
 * @return          The free size.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_space_len(rb_ring_buff_t *rb)
{
    os_uint32_t data_len;
    os_uint32_t space_len;

    OS_ASSERT(rb != OS_NULL);

    data_len = rb_ring_buff_data_len(rb);
    space_len = rb->buff_size - data_len;

    return space_len;
}

/**
 ***********************************************************************************************************************
 * @brief           Put a block of data into ring buffer.
 *
 * @param[in]       rb              Ring buffer control block.
 * @param[in]       buff            The buffer.
 * @param[in]       buff_len        The buffer size.
 *
 * @return          Size of data actually put.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_put(rb_ring_buff_t *rb, const os_uint8_t *buff, os_uint32_t buff_len)
{
    os_uint32_t size;

    OS_ASSERT(rb && buff && buff_len);

    /* Whether has enough space */
    size = rb_ring_buff_space_len(rb);
    if (size == 0)
    {
        return 0;
    }
    
    /* Drop some data */
    if (size < buff_len)
    {
        buff_len = size;
    }
    
    if (rb->buff_size - rb->write_index  > buff_len )
    {
        memcpy(rb->buff + rb->write_index, buff, buff_len);
        rb->write_index += buff_len;
        return buff_len;
    }

    memcpy(rb->buff + rb->write_index, buff, rb->buff_size - rb->write_index);
    memcpy(rb->buff,
           buff + (rb->buff_size - rb->write_index),
           buff_len - (rb->buff_size - rb->write_index));

    /* We are going into the other side of the mirror */
    rb->write_mirror = !rb->write_mirror;
    rb->write_index = buff_len - (rb->buff_size - rb->write_index);

    return buff_len;
}

/**
 ***********************************************************************************************************************
 * @brief           Put a block of data into ring buffer.
 *
 * @attention       When the buffer is full, it will discard the old data.
 *
 * @param[in]       rb              Ring buffer control block.
 * @param[in]       buff            The buffer.
 * @param[in]       buff_len        The buffer size.
 *
 * @return          Size of data actually put.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_put_force(rb_ring_buff_t *rb, const os_uint8_t *buff, os_uint32_t buff_len)
{
    os_uint32_t space_len;
    os_uint32_t copy_len;

    OS_ASSERT(rb && buff && buff_len);

    space_len = rb_ring_buff_space_len(rb);

    if (buff_len > rb->buff_size)
    {
        /* Only copy data of last, size is rb->buff_size */
        buff = buff + (buff_len - rb->buff_size);
        buff_len = rb->buff_size;
    }

    if (rb->buff_size - rb->write_index > buff_len)
    {
        /* 
         * This should not cause overflow because there is enough space for
         * length of data in current mirror
         */
        memcpy(rb->buff + rb->write_index, buff, buff_len);
        rb->write_index += buff_len;

        /* read_index - write_index = empty space */
        if (buff_len > space_len)
        {
            rb->read_index = rb->write_index;
        }
        
        return buff_len;
    }

    copy_len = rb->buff_size - rb->write_index;
    memcpy(rb->buff + rb->write_index, buff, copy_len);
    memcpy(rb->buff, buff + copy_len, buff_len - copy_len);

    /* We are going into the other side of the mirror */
    rb->write_mirror = !rb->write_mirror;
    rb->write_index  = buff_len - (rb->buff_size - rb->write_index);

    if (buff_len > space_len)
    {
        rb->read_mirror = !rb->read_mirror;
        rb->read_index  = rb->write_index;
    }

    return buff_len;
}

/**
 ***********************************************************************************************************************
 * @brief           Get a block of data from ring buffer.
 *
 * @param[in]       rb              Ring buffer control block.
 * @param[out]      buff            The buffer.
 * @param[in]       buff_len        The buffer size.
 *
 * @return          Size of data actually get.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_get(rb_ring_buff_t *rb, os_uint8_t *buff, os_uint32_t buff_len)
{
    os_uint32_t size;

    OS_ASSERT(rb && buff && buff_len);

    /* Whether has enough data  */
    size = rb_ring_buff_data_len(rb);
    if (size == 0)
    {
        /* No data */
        return 0;
    }
    else if (size < buff_len)
    {
        /* Less data */
        buff_len = size;
    }
    
    if (rb->buff_size - rb->read_index > buff_len)
    {
        /* 
         * This should not cause overflow because there is enough space for
         * length of data in current mirror
         */
        memcpy(buff, rb->buff + rb->read_index, buff_len);
        rb->read_index += buff_len;
        
        return buff_len;
    }

    memcpy(buff, rb->buff + rb->read_index, rb->buff_size - rb->read_index);
    memcpy(buff + (rb->buff_size - rb->read_index),
           rb->buff,
           buff_len - (rb->buff_size - rb->read_index));

    /* We are going into the other side of the mirror */
    rb->read_mirror = !rb->read_mirror;
    rb->read_index = buff_len - (rb->buff_size - rb->read_index);

    return buff_len;
}

/**
 ***********************************************************************************************************************
 * @brief           Put a character into ring buffer.
 *
 * @param[in]       rb              The ring buffer control block.
 * @param[in]       ch              The charater to put.
 *
 * @return          Size of data actually put.
 * @retval          1               Put a character success.
 * @retval          0               Ring buffer is full, put a character failed.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_put_char(rb_ring_buff_t *rb, const os_uint8_t ch)
{
    os_uint32_t space_len;
    
    OS_ASSERT(rb != OS_NULL);

    /* Whether has enough space */
    space_len = rb_ring_buff_space_len(rb);
    if (!space_len)
    {
        return 0;
    }
    
    rb->buff[rb->write_index] = ch;

    /* Flip mirror */
    if (rb->write_index == rb->buff_size-1)
    {
        rb->write_mirror = !rb->write_mirror;
        rb->write_index = 0;
    }
    else
    {
        rb->write_index++;
    }

    return 1;
}

/**
 ***********************************************************************************************************************
 * @brief           Put a character into ring buffer.
 *
 * @attention       When the buffer is full, it will discard one old data.
 *
 * @param[in]       rb              The ring buffer control block.
 * @param[in]       ch              The charater to put.
 *
 * @return          Size of data actually put, always 1.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_put_char_force(rb_ring_buff_t *rb, const os_uint8_t ch)
{
    rb_ring_buff_state_t old_state;

    OS_ASSERT(rb != OS_NULL);

    old_state = rb_ring_buff_state(rb);

    rb->buff[rb->write_index] = ch;

    /* Flip mirror */
    if (rb->write_index == rb->buff_size-1)
    {
        rb->write_mirror = !rb->write_mirror;
        rb->write_index = 0;
        
        if (old_state == RB_RING_BUFF_FULL)
        {
            rb->read_mirror = !rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    }
    else
    {
        rb->write_index++;
        
        if (old_state == RB_RING_BUFF_FULL)
        {
            rb->read_index = rb->write_index;
        }
    }

    return 1;
}

/**
 ***********************************************************************************************************************
 * @brief           Get a character from a ring buffer
 *
 * @param[in]       rb              The ring buffer control block.
 * @param[out]      ch              The charater to get.
 *
 * @return          Size of data actually get.
 * @retval          1               Get character success.
 * @retval          0               The ring buffer is empty, get character failed.
 ***********************************************************************************************************************
 */
os_uint32_t rb_ring_buff_get_char(rb_ring_buff_t *rb, os_uint8_t *ch)
{
    os_uint32_t size;
    
    OS_ASSERT(rb && ch);

    /* Ring buffer is empty */
    size = rb_ring_buff_data_len(rb);
    if (!size)
    {
        return 0;
    }
    
    /* Get character */
    *ch = rb->buff[rb->read_index];

    if (rb->read_index == rb->buff_size-1)
    {
        rb->read_mirror = !rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}


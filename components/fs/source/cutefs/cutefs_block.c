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
 * @file        cutefs_block.c
 *
 * @brief       Block management implement of cute filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <dlog.h>
#include <os_errno.h>
#include <os_memory.h>
#include <os_list.h>
#include <device.h>
#include <block/block_device.h>

struct block_node
{
    os_slist_node_t list_node;
    os_size_t       bolck_id;                           /* The block id*/
};
typedef struct block_node block_node_t;

struct block_pool
{
    os_device_t        *dev_id;
    os_size_t           block_size;                     /* Size of memory blocks. */                       
    os_size_t           block_total_cnt;                /* Numbers of memory block. */
    os_size_t           block_free_cnt;                 /* Numbers of free memory block. */
    os_slist_node_t     free_list_head;                 /* The list head of free block*/
    void *              block_node_buff;                /* The buffer to save block node info */
};
typedef struct block_pool block_pool_t;

void *cutefs_block_init(os_device_t *dev)
{
    os_uint32_t i = 0;
    struct os_blk_geometry geometry;
    block_node_t *block_node;
    block_pool_t *block_pool;

    block_pool = (block_pool_t *)os_malloc(sizeof(block_pool_t));
    if (!block_pool)
    {
        return OS_NULL;
    }
    block_pool->dev_id = dev;

    if (OS_EOK != os_device_control(dev, OS_DEVICE_CTRL_BLK_GETGEOME, &geometry))
    {
        os_free(block_pool);
        return OS_NULL;
    }
    block_pool->block_total_cnt = geometry.capacity / geometry.block_size;
    block_pool->block_size      = geometry.block_size;
    block_pool->block_free_cnt  = block_pool->block_total_cnt;

    os_slist_init(&block_pool->free_list_head);
    block_node = os_malloc(sizeof(block_node_t) * block_pool->block_total_cnt);
    if (!block_node)
    {
        os_free(block_pool);
        return OS_NULL;
    }
    for (i = 0; i < block_pool->block_total_cnt; i++)
    {
        block_node[i].bolck_id = i;
        os_slist_add(&block_pool->free_list_head, &block_node[i].list_node);
    }
    block_pool->block_node_buff = block_node;

    return block_pool;
}

void cutefs_block_deinit(void *blk_pool)
{    
    block_pool_t *block_pool = (block_pool_t *)blk_pool;

    if ((!block_pool) || (!block_pool->block_node_buff))
    {
        return;
    }
    os_free(block_pool->block_node_buff);
    os_free(block_pool);
}

os_err_t cutefs_block_getinfo(void *blk_pool, os_uint32_t *block_cnt, os_uint32_t *block_size)
{
    block_pool_t *block_pool = (block_pool_t *)blk_pool;

    if ((!block_pool) || (!block_cnt) || (!block_size))
    {
        return OS_ERROR;
    }

    *block_cnt =  block_pool->block_total_cnt;
    *block_size = block_pool->block_size;

    return OS_EOK;
}

void* cutefs_block_alloc(void *blk_pool)
{
    block_node_t *block_node;    
    block_pool_t *block_pool = (block_pool_t *)blk_pool;

    if ((!block_pool) || (!block_pool->block_free_cnt))
    {
        return OS_NULL;
    }
    block_node = (block_node_t *)os_slist_first(&block_pool->free_list_head);
    if (!block_node)
    {
        return OS_NULL;
    }
    os_slist_del(&block_pool->free_list_head, &block_node->list_node);
    block_pool->block_free_cnt--;

    return block_node;
}

void cutefs_block_free(void *blk_pool, void *blk_node)
{
    block_pool_t *block_pool = (block_pool_t *)blk_pool;
    block_node_t *block_node = (block_node_t *)blk_node;  

    if ((!block_pool) || (!block_node))
    {
        return;
    }

    os_slist_add(&block_pool->free_list_head, &block_node->list_node);
    block_pool->block_free_cnt++;
}

os_size_t cutefs_block_write(void *blk_pool, void *blk_node, void *buf)
{    
    block_pool_t *block_pool = (block_pool_t *)blk_pool;
    block_node_t *block_node = (block_node_t *)blk_node;  

    if ((!block_pool) || (!block_node) || (!buf))
    {
        return 0;
    }

    return os_device_write_nonblock(block_pool->dev_id, block_node->bolck_id, buf, 1);
}

os_size_t cutefs_block_read(void *blk_pool, void *blk_node, void *buf)
{
    block_pool_t *block_pool = (block_pool_t *)blk_pool;
    block_node_t *block_node = (block_node_t *)blk_node;

    if ((!block_pool) || (!block_node) || (!buf))
    {
        return 0;
    }

    return os_device_read_nonblock(block_pool->dev_id, block_node->bolck_id, buf, 1);
}


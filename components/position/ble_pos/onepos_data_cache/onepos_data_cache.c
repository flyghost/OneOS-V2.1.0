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
 * @file        onepos_data_cache.c
 *
 * @brief       data cache based on a circular queue with data protection function
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-29   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_errno.h> 
#include <os_memory.h>
#include "onepos_state.h"
#include "onepos_common.h"
#include "onepos_data_cache.h"

/**
  ***********************************************************************************************************************
  * @brief           create a data_cache
  *
  * @param[in]       data_num_max           max member num in a cache  
  * @param[in]       data_size              single cache member size 
  *
  * @return          The pointer to the cache that is created
  * @retval          onepos_data_cache_t*   succeeded
  * @retval          OS_NULL                failed
  ***********************************************************************************************************************
  */
onepos_data_cache_t *onepos_data_cache_create(os_uint32_t data_num_max, os_uint32_t data_size)
{
    if(data_num_max <= 0 && data_size <= 0)
    {
        return OS_NULL;
    }
    
    os_sem_t *cache_reader = OS_NULL;
    onepos_queue_t *queue  = OS_NULL;
    onepos_data_cache_t *data_cache = OS_NULL;
    
    data_cache = (onepos_data_cache_t *)os_malloc(sizeof(onepos_data_cache_t));
    if(OS_NULL == data_cache)
    {
        return OS_NULL;
    }

    cache_reader = os_sem_create("onepos_data_cache_reader", 1, SEM_VALUE_TO_MUTEX);
    if(OS_NULL == cache_reader)
    {
        os_free(data_cache);
        return OS_NULL;
    }

    queue = onepos_queue_create(data_num_max, data_size);
    if(OS_NULL == queue)
    {
        os_free(data_cache);
        os_sem_destroy(cache_reader);
        return OS_NULL;    
    }

    data_cache->queue = queue;
    data_cache->cache_reader = cache_reader;
   
    return data_cache;
}


/**
  ***********************************************************************************************************************
  * @brief           destroy a data_cache
  *
  * @param[in]       data_cache       the onepos_data_cache_t need to destroy
  ***********************************************************************************************************************
  */
void onepos_data_cache_destroy(onepos_data_cache_t *data_cache)
{
    if(OS_NULL == data_cache)
    {
        return;
    }
    
    onepos_queue_destroy(data_cache->queue);
    os_sem_destroy(data_cache->cache_reader);
    os_free(data_cache);

    return;
}


/**
  ***********************************************************************************************************************
  * @brief            add data to cache
  *
  * @param[in]        data_cache    
  * @param[in]        data          data need to add to a cache
  * @param[in]        size          data size
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_ENULL
  * @retval           POS_ESIZE  
  ***********************************************************************************************************************
  */
os_err_t onepos_data_cache_add(onepos_data_cache_t *data_cache, void *data, os_uint32_t size)
{
    if(OS_NULL == data_cache || OS_NULL == data)
    {
        return POS_ENULL;
    }

    os_err_t res;
   
    os_sem_wait(data_cache->cache_reader, OS_WAIT_FOREVER);
    res = onepos_enqueue(data_cache->queue, data, size);
    os_sem_post(data_cache->cache_reader);

    return res;
}


/**
  ***********************************************************************************************************************
  * @brief            fetch all data in the cache
  *
  * @param[in]  data_cache       
  * @param[out] data                   addr to storage cache data 
  * @param[out] data_num               0       (default)
  * @param[out] single_mem_max_size            (bytes)
  * @param[in]  data_size_list_handle  member content of the data_size_list: data actual size when it is added to the cache
  *
  * @return     the operation result
  * @retval     OS_EOK                 *only when the retval is OS_EOK, all return parameter values are valid
  * @retval     POS_ENULL  
  * @retval     POS_WEMPTY             no cache data to fetch
***********************************************************************************************************************
*/
os_err_t onepos_data_cache_fetch_all(onepos_data_cache_t *data_cache, 
                                     void                *data, 
                                     os_int32_t          *data_num,
                                     os_uint32_t         *single_mem_max_size,
                                     os_uint32_t         *data_size_list_handle)
{
    if(OS_NULL == data_cache 
       || OS_NULL == data                
       || OS_NULL == data_num            
       || OS_NULL == single_mem_max_size 
       || OS_NULL == data_size_list_handle)
    {
        return POS_ENULL;
    }

    os_uint32_t  i = 0;
    void *addr     = OS_NULL;
    os_uint32_t  cache_mem_num  = 0;
       
    *data_num = 0;
        
    os_sem_wait(data_cache->cache_reader, OS_WAIT_FOREVER);
    
    /*1. malloc for data_size_list */
    cache_mem_num = onepos_get_queue_node_num(data_cache->queue);
    
    if(0 == cache_mem_num)
    {
        os_sem_post(data_cache->cache_reader);
        return POS_WEMPTY;
    }
       
    /* 2.fetch data */ 
    addr = data;
    while(OS_EOK == onepos_dequeue(data_cache->queue, addr, &(data_size_list_handle[i])))
    {
        i++;
        (*data_num)++;
        addr = (void *)((char *)data + i * (data_cache->queue->node_size_max));           
    }  
    
    data_cache->queue->front = data_cache->queue->rear;
    os_sem_post(data_cache->cache_reader);

    *single_mem_max_size = data_cache->queue->node_size_max;
    
    return OS_EOK;
}


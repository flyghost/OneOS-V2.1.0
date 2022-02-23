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
 * @file        onepos_queue.c
 *
 * @brief       circular queue
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-28   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_errno.h>
#include <string.h> 
#include <os_memory.h>                      
#include "onepos_queue.h"
#include "onepos_common.h"


/**
  ***********************************************************************************************************************
  * @brief           create a circular queue
  *
  * @param[in]       node_num_max           max member_num in a queue  
  * @param[in]       node_size_max          single queue member max size 
  *
  * @return          The pointer to the queue that is created
  * @retval          onepos_queue_t *       succeeded
  * @retval          OS_NULL                failed
  ***********************************************************************************************************************
  */
onepos_queue_t *onepos_queue_create(os_uint32_t node_num_max, os_uint32_t node_size_max)
{
    if(node_num_max <= 0 && node_size_max <= 0)
    {
        return OS_NULL;
    }

    onepos_queue_t *queue          = OS_NULL;
    void           *buff_head      = OS_NULL;
    os_uint32_t    *size_list_head = OS_NULL;

    queue = (onepos_queue_t *)os_malloc(sizeof(onepos_queue_t));
    if(OS_NULL == queue)
    {
        return OS_NULL;
    }

    buff_head = (void *)os_malloc(node_num_max * node_size_max);
    if(OS_NULL == buff_head)
    {
        os_free(queue);
        return OS_NULL;
    } 

    size_list_head = (os_uint32_t *)os_malloc(node_num_max * sizeof(os_uint32_t));
    if(OS_NULL == size_list_head)
    {
        os_free(queue);
        os_free(buff_head);
        return OS_NULL;
    } 
    
    queue->data          = buff_head;
    queue->size_data     = size_list_head;
    queue->front         = -1;
    queue->rear          = -1;
    queue->node_num_max  = node_num_max;
    queue->node_num_now  = 0;
    queue->node_size_max = node_size_max;
    
    return queue;
}


/**
  ***********************************************************************************************************************
  * @brief           destroy a circular queue
  *
  * @param[in]       queue     queue need to destroy  
  ***********************************************************************************************************************
  */
void onepos_queue_destroy(onepos_queue_t * queue)
{
    if(OS_NULL == queue)
    {
        return;
    }

    os_free(queue->data);
    os_free(queue->size_data);
    os_free(queue);

    return;
}


/**
  ***********************************************************************************************************************
  * @brief            wether the queue is full
  *
  * @param[in]        queue         
  *
  * @return           
  * @retval           OS_TRUE
  * @retval           OS_FALSE
  ***********************************************************************************************************************
  */
static os_bool_t onepos_queue_is_full(onepos_queue_t * queue)
{
    if(queue->node_num_now == queue->node_num_max)
    {
        return OS_TRUE;
    }
    return OS_FALSE;

}
 

/**
  ***********************************************************************************************************************
  * @brief            wether the queue is empty
  *
  * @param[in]        queue         
  *
  * @return           
  * @retval           OS_TRUE
  * @retval           OS_FALSE
  ***********************************************************************************************************************
  */
static os_bool_t onepos_queue_is_empty(onepos_queue_t * queue)
{
    if(queue->node_num_now == 0)
    {
         return OS_TRUE;
    }
    return OS_FALSE;
}


/**
  ***********************************************************************************************************************
  * @brief            enqueue
  *
  * @param[in]        queue         target queue
  * @param[in]        data          data need to enqueue
  * @param[in]        size          data size
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_ENULL
  * @retval           POS_ESIZE  
  * @retval           OS_ERROR      
  ***********************************************************************************************************************
  */
os_err_t onepos_enqueue(onepos_queue_t * queue, void *data, os_uint32_t size)
{
    if(OS_NULL == queue || OS_NULL == data)
    {
        return POS_ENULL;
    }

    if(size > queue->node_size_max || size <= 0)
    {
        return POS_ESIZE;
    }

    char        *data_buf_dst = OS_NULL;
    os_uint32_t  offset_bytes;
    os_uint32_t *size_list_head = queue->size_data;

    if(OS_TRUE == onepos_queue_is_full(queue))
    {
        offset_bytes = ((queue->rear + 1) % queue->node_num_max) * queue->node_size_max;
        data_buf_dst = (char *)queue->data + offset_bytes;
        memset(data_buf_dst, 0, queue->node_size_max);
        memcpy(data_buf_dst, data, size);
   
        queue->rear = (queue->rear + 1) % queue->node_num_max;
        size_list_head[queue->rear] = size;
        return OS_EOK;
    } 
    
    if(OS_FALSE == onepos_queue_is_full(queue))
    {
        offset_bytes = ((queue->rear + 1) % queue->node_num_max) * queue->node_size_max;
        data_buf_dst = (char *)queue->data + offset_bytes;
        memset(data_buf_dst, 0, queue->node_size_max);
        memcpy(data_buf_dst, data, size);
   
        queue->rear = (queue->rear + 1) % queue->node_num_max;
        size_list_head[queue->rear] = size;
        queue->node_num_now++;
        return OS_EOK;
    }
    
    return OS_ERROR;
}


/**
  ***********************************************************************************************************************
  * @brief            dequeue
  *
  * @param[in]        queue       
  * @param[out]       data                 addr to storage queue member 
  * @param[out]       valid_size           queue member actual size when the queue member enqueue
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_ENULL
  * @retval           OS_ERROR             queue is empty
  ***********************************************************************************************************************
  */
os_err_t onepos_dequeue(onepos_queue_t * queue, void *data, os_uint32_t *valid_size)
{
    if(OS_NULL == queue || OS_NULL == data)
    {
        return POS_ENULL;
    }

    char        *data_buf_src = OS_NULL;
    os_uint32_t  offset_bytes;
    os_uint32_t *size_list_head = queue->size_data;
    memset(valid_size, 0, sizeof(os_uint32_t));
    
    if(OS_FALSE == onepos_queue_is_empty(queue))
    {
        offset_bytes = ((queue->front + 1) % queue->node_num_max) * queue->node_size_max;
        data_buf_src = (char *)queue->data + offset_bytes;
        memcpy(data, data_buf_src, queue->node_size_max);

        queue->front = (queue->front + 1) % queue->node_num_max;
        *valid_size  = size_list_head[queue->front];
        queue->node_num_now--;
        return OS_EOK;
    }

    return OS_ERROR;
}


/**
  ***********************************************************************************************************************
  * @brief            get queue node num now
  *
  * @param[in]        queue       
  ***********************************************************************************************************************
  */
os_uint32_t onepos_get_queue_node_num(onepos_queue_t * queue)
{
    return queue->node_num_now;
}





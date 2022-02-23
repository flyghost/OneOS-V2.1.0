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
 * @file        onepos_queue.h
 *
 * @brief       circular queue
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-28   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ONEPOS_QUEUE_H__
#define __ONEPOS_QUEUE_H__

#include <os_types.h>    

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    void        *data;          /* queue member storage buff     */
    os_uint32_t *size_data;     /* queue member actual size list */
    os_uint32_t  node_num_max;  /* max member_num in a queue     */ 
    os_uint32_t  node_size_max; /* single queue member max size  */
    os_uint32_t  node_num_now;  /* queue member num now          */
    os_int32_t   front;         /* last pop member index         */
    os_int32_t   rear;          /* last push memmber index       */
}onepos_queue_t;


extern onepos_queue_t *onepos_queue_create(os_uint32_t node_num_max, os_uint32_t node_size_max);
extern void            onepos_queue_destroy(onepos_queue_t * queue);
extern os_err_t        onepos_enqueue(onepos_queue_t * queue, void *data, os_uint32_t size);
extern os_err_t        onepos_dequeue(onepos_queue_t * queue, void *data, os_uint32_t *valid_size);
extern os_uint32_t     onepos_get_queue_node_num(onepos_queue_t * queue);

#ifdef __cplusplus
}
#endif

#endif /*__ONEPOS_QUEUE_H__ */


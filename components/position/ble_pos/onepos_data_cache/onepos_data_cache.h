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
 * @file        onepos_data_cache.h
 *
 * @brief       data cache based on a circular queue with data protection function
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-28   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ONEPOS_DATA_CACHE_H__
#define __ONEPOS_DATA_CACHE_H__

#include <os_sem.h>
#include <os_types.h>    
#include "onepos_queue.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    onepos_queue_t *queue;
    os_sem_t       *cache_reader;
}onepos_data_cache_t;


extern onepos_data_cache_t *onepos_data_cache_create(os_uint32_t data_num_max, os_uint32_t data_size);
extern void                 onepos_data_cache_destroy(onepos_data_cache_t * data_cache);
extern os_err_t             onepos_data_cache_add(onepos_data_cache_t *data_cache, void *data, os_uint32_t size);
extern os_err_t             onepos_data_cache_fetch_all(onepos_data_cache_t *data_cache, 
                                                        void                *data, 
                                                        os_int32_t          *data_num,
                                                        os_uint32_t         *single_data_max_size,
                                                        os_uint32_t         *data_size_list_handle);

#ifdef __cplusplus
}
#endif

#endif /*__ONEPOS_DATA_CACHE_H__ */


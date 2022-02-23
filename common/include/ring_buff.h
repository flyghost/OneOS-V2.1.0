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
 * @file        ring_buff.h
 *
 * @brief       Header file for ring buffer.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-08   OneOS Team      Modify Ring buffer control block structrue.
 ***********************************************************************************************************************
 */

#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @struct      rb_ring_buff
 *
 * @brief       Ring buffer control block      
 ***********************************************************************************************************************
 */
struct rb_ring_buff
{
    os_uint8_t  *buff;              /* The buffer */
    os_uint32_t  buff_size;         /* The buffer size */

    os_uint32_t  read_index;        /* Current read index */
    os_uint32_t  write_index;       /* Current write index */      
    
    os_bool_t    read_mirror;       /* Read mirror flag */    
    os_bool_t    write_mirror;      /* Write mirror flag */ 
};
typedef struct rb_ring_buff rb_ring_buff_t;


extern void            rb_ring_buff_init(rb_ring_buff_t *rb, os_uint8_t *pool, os_uint32_t pool_size);

#ifdef OS_USING_SYS_HEAP
extern rb_ring_buff_t *rb_ring_buff_create(os_uint32_t size);
extern void            rb_ring_buff_destroy(rb_ring_buff_t *rb);
#endif

extern void            rb_ring_buff_reset(rb_ring_buff_t *rb);
extern os_uint32_t     rb_ring_buff_data_len(rb_ring_buff_t *rb);
extern os_uint32_t     rb_ring_buff_space_len(rb_ring_buff_t *rb);
extern os_uint32_t     rb_ring_buff_put(rb_ring_buff_t *rb, const os_uint8_t *buff, os_uint32_t buff_len);
extern os_uint32_t     rb_ring_buff_put_force(rb_ring_buff_t *rb, const os_uint8_t *buff, os_uint32_t buff_len);
extern os_uint32_t     rb_ring_buff_get(rb_ring_buff_t *rb, os_uint8_t *buff, os_uint32_t buff_len);
extern os_uint32_t     rb_ring_buff_put_char(rb_ring_buff_t *rb, const os_uint8_t ch);
extern os_uint32_t     rb_ring_buff_put_char_force(rb_ring_buff_t *rb, const os_uint8_t ch);
extern os_uint32_t     rb_ring_buff_get_char(rb_ring_buff_t *rb, os_uint8_t *ch);

#ifdef __cplusplus
}
#endif

#endif /* __RING_BUFF_H__ */


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
 * @file        ring_blk_buff.h
 *
 * @brief       Header file for ring block buffer.
 *
 * @revision
 * Date         Author          Notes
 * 2018-08-25   armink          The first version.
 ***********************************************************************************************************************
 */

#ifndef __RING_BLK_BUFF_H__
#define __RING_BLK_BUFF_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

enum rbb_blk_status
{
    RBB_BLK_STATUS_UNUSED,      /* Unused status when first initialize or after rbb_blk_free() */
    RBB_BLK_STATUS_INITED,      /* Initialized status after rbb_blk_alloc() */
    RBB_BLK_STATUS_PUT,         /* Put status after rbb_blk_put() */
    RBB_BLK_STATUS_GET,         /* Get status after rbb_blk_get() */
};
typedef enum rbb_blk_status rbb_blk_status_t;

/* The block of rbb */
struct rbb_blk
{
    os_list_node_t    list_node;
    
    os_uint8_t       *buf;
    os_size_t         buf_size;
    
    rbb_blk_status_t  status;  
};
typedef struct rbb_blk rbb_blk_t;

/* Ring block buffer ctrl info */
struct rbb_ctrl_info
{
    os_uint8_t      *buf;
    os_size_t        buf_size;
    
    rbb_blk_t       *blk_set;           /* All of blocks */
    os_size_t        blk_max_num;
    
    os_list_node_t   blk_list_head;     /* Saved the initialized and put status blocks */
    
    os_spinlock_t    rbb_locker;
};
typedef struct rbb_ctrl_info rbb_ctrl_info_t;

extern void        rbb_init(rbb_ctrl_info_t    *rbb,
                            os_uint8_t         *buf,
                            os_size_t           buf_size,
                            rbb_blk_t          *block_set,
                            os_size_t           blk_max_num);
extern os_size_t   rbb_get_buf_size(rbb_ctrl_info_t *rbb);

#ifdef OS_USING_SYS_HEAP
extern rbb_ctrl_info_t *rbb_create(os_size_t buf_size, os_size_t blk_max_num);
extern void             rbb_destroy(rbb_ctrl_info_t *rbb);
#endif

/* rbb block API */
extern rbb_blk_t  *rbb_blk_alloc(rbb_ctrl_info_t *rbb, os_size_t blk_size);
extern void        rbb_blk_put(rbb_blk_t *block);
extern rbb_blk_t  *rbb_blk_get(rbb_ctrl_info_t *rbb);
extern void        rbb_blk_free(rbb_ctrl_info_t *rbb, rbb_blk_t *block);
#ifdef __cplusplus
}
#endif

#endif /* __RING_BLK_BUFF_H__ */


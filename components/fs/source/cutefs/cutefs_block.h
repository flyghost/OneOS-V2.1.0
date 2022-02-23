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
 * @file        cutefs_block.h
 *
 * @brief       Header file for block management of cute filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __CUTEFS_BLOCK_H__
#define __CUTEFS_BLOCK_H__

extern void     *cutefs_block_init(os_device_t *dev);
extern void      cutefs_block_deinit(void *blk_pool);
extern os_err_t  cutefs_block_getinfo(void *blk_pool, os_uint32_t *block_cnt, os_uint32_t *block_size);
extern void     *cutefs_block_alloc(void *blk_pool);
extern void      cutefs_block_free(void *blk_pool, void *blk_node);
extern os_size_t cutefs_block_write(void *blk_pool, void *blk_node, void *buf);
extern os_size_t cutefs_block_read(void *blk_pool, void *blk_node, void *buf);

#endif

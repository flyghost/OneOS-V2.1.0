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
 * @file        os_dma.h
 *
 * @brief       this file implements dam
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-10-29    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _DRIVERS_DMA_RAM_H_
#define _DRIVERS_DMA_RAM_H_

void os_dma_mem_init(void);
void *os_dma_malloc_align(os_size_t size, os_size_t align);
void os_dma_free_align(void *ptr);

#endif /* _DRIVERS_DMA_RAM_H_ */

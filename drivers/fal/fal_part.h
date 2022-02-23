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
 * @file        fal_part.h
 *
 * @brief       fal_part
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __FAL_PART_H__
#define __FAL_PART_H__

#include <os_assert.h>
#include <device.h>
#include <fal/fal.h>

#ifndef FAL_DEV_NAME_MAX
#define FAL_DEV_NAME_MAX 16
#endif

#define FAL_PART_INFO_FLAGS_LOCKED    (1 << 0)
#define FAL_PART_INFO_FLAGS_UNLOCKED  (0 << 0)

typedef struct fal_part_info
{
    char name[FAL_DEV_NAME_MAX];

    char flash_name[FAL_DEV_NAME_MAX];

    os_uint32_t offset;
    os_uint32_t size;

    os_uint32_t flags;
}fal_part_info_t;

typedef struct fal_part
{
    fal_part_info_t *info;
    fal_flash_t     *flash;

    os_uint32_t flags;
}fal_part_t;

fal_part_t *fal_part_find(const char *name);
int fal_part_read(fal_part_t *part, uint32_t addr, uint8_t *buf, size_t size);
int fal_part_write(fal_part_t *part, uint32_t addr, const uint8_t *buf, size_t size);
int fal_part_erase(fal_part_t *part, uint32_t addr, size_t size);
int fal_part_erase_all(fal_part_t *part);
int fal_part_lock(fal_part_t *part);
int fal_part_unlock(fal_part_t *part);

#define fal_part_size(part)         (part->info->size)
#define fal_part_block_size(part)   (part->flash->block_size)
#define fal_part_page_size(part)    (part->flash->page_size)

#endif /* __FAL_PART_H__ */

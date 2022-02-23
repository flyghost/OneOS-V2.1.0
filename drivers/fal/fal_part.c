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
 * @file        fal_part.c
 *
 * @brief       fal_part
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include <fal.h>
#include <fal_part.h>
#include <os_memory.h>
#include <driver.h>
#include <fal_cfg.c>
#include "dlog.h"

#define DBG_TAG   "drv.fal_part"

static fal_part_t *fal_part_table = NULL;
static const int fal_part_table_size = ARRAY_SIZE(fal_part_info);

static void fal_show_part_table(void)
{
    char *item1 = "name", *item2 = "flash_dev";
    size_t i, part_name_max = strlen(item1), flash_dev_name_max = strlen(item2);
    struct fal_part *part;

    part = &fal_part_table[0];
    for (i = 0; i < fal_part_table_size; i++, part++)
    {
        if (strlen(part->info->name) > part_name_max)
        {
            part_name_max = strlen(part->info->name);
        }
        if (strlen(part->info->flash_name) > flash_dev_name_max)
        {
            flash_dev_name_max = strlen(part->info->flash_name);
        }
    }

    os_kprintf("==================== FAL partition table ====================\r\n");
    os_kprintf("| %-*.*s | %-*.*s |   offset   |    length  |\r\n", part_name_max, FAL_DEV_NAME_MAX, item1, flash_dev_name_max,
            FAL_DEV_NAME_MAX, item2);
    os_kprintf("-------------------------------------------------------------\r\n");

    part = &fal_part_table[0];
    for (i = 0; i < fal_part_table_size; i++, part++)
    {
        os_kprintf("| %-*.*s | %-*.*s | 0x%08lx | 0x%08x |\r\n", part_name_max, FAL_DEV_NAME_MAX, part->info->name, flash_dev_name_max,
                FAL_DEV_NAME_MAX, part->info->flash_name, part->info->offset, part->info->size);
    }
    os_kprintf("=============================================================\r\n");
}

int fal_part_init(void)
{
    int i;
    fal_flash_t *flash_dev = NULL;

    fal_part_table = os_calloc(fal_part_table_size, sizeof(fal_part_t));
    OS_ASSERT(fal_part_table);

    for (i = 0; i < fal_part_table_size; i++)
    {
        flash_dev = fal_flash_find(fal_part_info[i].flash_name);
        if (flash_dev == NULL)
        {
            LOG_E(DBG_TAG, "Initialize failed! Didn't find the flash device(%s).\r\n", fal_part_info[i].flash_name);
            os_free(fal_part_table);
            fal_part_table = OS_NULL;
            return -1;
        }

        if (fal_part_info[i].offset >= (long)flash_dev->capacity)
        {
            LOG_E(DBG_TAG, "Initialize failed! Partition(%s) offset address(%ld) out of flash bound(<%d).\r\n",
                    fal_part_info[i].name, fal_part_info[i].offset, flash_dev->capacity);
            os_free(fal_part_table);
            fal_part_table = OS_NULL;
            return -1;
        }

        fal_part_table[i].info  = (fal_part_info_t *)&fal_part_info[i];
        fal_part_table[i].flash = flash_dev;
        fal_part_table[i].flags = fal_part_info[i].flags;
    }

    fal_show_part_table();

    return 0;
}

OS_DEVICE_INIT(fal_part_init, OS_INIT_SUBLEVEL_LOW);

fal_part_t *fal_part_find(const char *name)
{
    int i;

    OS_ASSERT(fal_part_table != OS_NULL);

    for (i = 0; i < fal_part_table_size; i++)
    {
        if (!strcmp(name, fal_part_table[i].info->name))
        {
            return &fal_part_table[i];
        }
    }

    return NULL;
}

int fal_part_read(fal_part_t *part, uint32_t offset, uint8_t *buf, size_t size)
{
    int ret = 0;
    os_uint32_t  page_addr;
    os_uint8_t  *page_buff;
    os_uint32_t  page_size;
    os_uint32_t  count, remain;
    fal_flash_t *flash_dev = OS_NULL;

    OS_ASSERT(part);
    OS_ASSERT(buf);

    flash_dev = part->flash;

    if (offset + size > part->info->size)
    {
        LOG_E(DBG_TAG, "part read [%s] address out of bound offset:0x%x, size:0x%x, len:0x%x.\r\n",
                   part->info->name, offset, size, part->info->size);
        return -1;
    }

    page_size = flash_dev->page_size;
    page_buff = os_calloc(1, page_size);
    if (page_buff == OS_NULL)
    {
        LOG_E(DBG_TAG, "fal read, memroy lack %s.\r\n", part->info->name);
        return 0;
    }

    os_uint32_t page_shift  = fal_page_shift(flash_dev);

    offset += part->info->offset;

    page_addr = offset >> fal_page_shift(flash_dev);
    offset   &= page_size - 1;

    remain = size;
    while (remain != 0)
    {
        if (offset != 0 || remain < page_size)
        {
            ret = flash_dev->ops.read_page(flash_dev, page_addr, page_buff, 1);
        }
        else
        {
            count = remain & ~(page_size - 1);
            ret = flash_dev->ops.read_page(flash_dev, page_addr, buf, remain >> page_shift);
        }

        if (ret != 0)
        {
            LOG_E(DBG_TAG, "flash device(%s) read error, %d .\r\n", part->info->name, page_addr);
            break;
        }

        if (offset != 0 || remain < page_size)
        {            
            count = min(page_size - offset, remain);
            memcpy(buf, page_buff + offset, count);
            page_addr++;
        }
        else
        {
            page_addr += remain >> page_shift;
        }
        
        buf    += count;
        remain -= count;
        offset += count;
        offset &= page_size - 1;
    }

    os_free(page_buff);
    return (size - remain);
}

int fal_part_write(fal_part_t *part, uint32_t offset, const uint8_t *buf, size_t size)
{
    int ret = 0;
    os_uint32_t page_addr;
    os_uint8_t *page_buff;
    os_uint32_t page_size;
    os_uint32_t count, remain;
    struct fal_flash *flash_dev = OS_NULL;

    OS_ASSERT(part);
    OS_ASSERT(buf);

    flash_dev = part->flash;

    if (part->flags & FAL_PART_INFO_FLAGS_LOCKED)
    {
        LOG_E(DBG_TAG, "part write [%s] locked.\r\n", part->info->name);
        return -2;
    }

    if (offset + size > part->info->size)
    {
        LOG_E(DBG_TAG, "part write [%s] address out of bound offset:0x%x, size:0x%x, len:0x%x.\r\n",
                   part->info->name, offset, size, part->info->size);
        return -1;
    }

    remain = size;
    page_size = flash_dev->page_size;
    page_buff = os_calloc(1, page_size);
    if (page_buff == OS_NULL)
    {
        LOG_E(DBG_TAG, "fal write, memroy lack %s.\r\n", part->info->name);
        return 0;
    }

    os_uint32_t page_shift  = fal_page_shift(flash_dev);

    offset += part->info->offset;
    
    page_addr = offset >> page_shift;
    offset   &= page_size - 1;

write_loop:
    if (offset != 0 || remain < page_size)
    {
        ret = flash_dev->ops.read_page(flash_dev, page_addr, page_buff, 1);
        if (ret != 0)
        {
            LOG_E(DBG_TAG, "fal pre read failed %s, %d .\r\n", part->info->name, page_addr);
            goto end;
        }
    }

    while (remain != 0)
    {
        if (offset != 0 || remain < page_size)
        {
            count = min(page_size - offset, remain);
            memcpy(page_buff + offset, buf, count);
            ret = flash_dev->ops.write_page(flash_dev, page_addr, page_buff, 1);
        }
        else
        {
            count = remain & ~(page_size - 1);
            ret = flash_dev->ops.write_page(flash_dev, page_addr, buf, remain >> page_shift);
        }
        
        if (ret != 0)
        {
            LOG_E(DBG_TAG, "fal write failed %s, %d .\r\n", part->info->name, page_addr);
            break;
        }

        if (offset != 0 || remain < page_size)
        {
            page_addr++;
        }
        else
        {
            page_addr += remain >> page_shift;
        }

        buf    += count;
        remain -= count;
        offset += count;
        offset &= page_size - 1;

        /* last not full page */
        if ((remain < page_size) && (remain != 0))
            goto write_loop;
    }

end:
    os_free(page_buff);
    return (size - remain);
}

int fal_part_erase(fal_part_t *part, uint32_t offset, size_t size)
{
    int ret = 0;
    struct fal_flash *flash_dev = OS_NULL;

    OS_ASSERT(part);

    flash_dev = part->flash;

    if (part->flags & FAL_PART_INFO_FLAGS_LOCKED)
    {
        LOG_E(DBG_TAG, "part erase [%s] locked.\r\n", part->info->name);
        return -2;
    }
    
    if ((offset & (flash_dev->block_size - 1)) != 0)
    {
        LOG_E(DBG_TAG, "erase offset error! offset/block_size: 0x%x/0x%x \r\n", offset, flash_dev->block_size);
        return -1;
    }
    
    if ((size & (flash_dev->block_size - 1)) != 0)
    {
        LOG_E(DBG_TAG, "erase size error! size/block_size: 0x%x/0x%x \r\n", size, flash_dev->block_size);
        return -1;
    }
    
    if (offset + size > part->info->size)
    {
        LOG_E(DBG_TAG, "part erase [%s] address out of bound offset:0x%x, size:0x%x, len:0x%x.\r\n",
                   part->info->name, offset, size, part->info->size);
        return -1;
    }

    os_uint32_t end;
    os_uint32_t page_addr;
    os_uint32_t block_size = flash_dev->block_size;
    os_uint32_t page_shift  = fal_page_shift(flash_dev);

    offset += part->info->offset;
    end     = offset + size;

    page_addr = offset >> page_shift;
    offset   &= ~(block_size - 1);

    if ((end - offset) & ~(block_size - 1))
    {
        ret = flash_dev->ops.erase_block(flash_dev, page_addr, ((end - offset) & ~(block_size - 1)) >> page_shift);
        if (ret != 0)
        {
            LOG_E(DBG_TAG, "fal erase failed %s, %d.\r\n", part->info->name, page_addr);
            return -1;
        }
        page_addr += ((end - offset) & ~(block_size - 1)) >> page_shift;
    }

    if ((end - offset) & (block_size - 1))
    {
        ret = flash_dev->ops.erase_block(flash_dev, page_addr, 1);
        if (ret != 0)
        {
            LOG_E(DBG_TAG, "fal erase failed %s, %d.\r\n", part->info->name, page_addr);
            return -2;
        }

        return ((end - offset) & ~(block_size - 1)) + block_size;
    }
    
    return (end - offset) & ~(block_size - 1);
}

int fal_part_erase_all(fal_part_t *part)
{
    if (part->flags & FAL_PART_INFO_FLAGS_LOCKED)
    {
        LOG_E(DBG_TAG, "part eraseall [%s] locked.\r\n", part->info->name);
        return -2;
    }

    return fal_part_erase(part, 0, part->info->size);
}

int fal_part_lock(fal_part_t *part)
{
    part->flags |= FAL_PART_INFO_FLAGS_LOCKED;
    LOG_I(DBG_TAG, "part [%s] locked.\r\n", part->info->name);
    return 0;
}

int fal_part_unlock(fal_part_t *part)
{
    part->flags &= ~FAL_PART_INFO_FLAGS_LOCKED;
    LOG_I(DBG_TAG, "part [%s] unlocked.\r\n", part->info->name);
    return 0;
}


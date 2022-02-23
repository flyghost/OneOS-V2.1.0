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
 * @file        block.c
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <fal/fal.h>
#include <os_memory.h>
#include <string.h>
#include <stdlib.h>
#include <os_clock.h>
#include <arch_misc.h>
#include <os_task.h>
#include <drv_cfg.h>
#include <dlog.h>

#define DBG_TAG "drv.block"

#define mtd_block_shift(mtd_dev)    (os_ffs(mtd_dev->geometry.block_size)-1)
#define mtd_page_shift(mtd_dev)     (os_ffs(mtd_dev->geometry.page_size)-1)

static os_size_t char_dev_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, char_dev);

    int ret = 0;
    os_uint8_t  *buff = buffer;
    os_uint32_t  page_addr;
    os_uint8_t  *page_buff;
    os_uint32_t  page_size;
    os_uint32_t  count, remain;

    OS_ASSERT(dev);
    OS_ASSERT(buff);

    if (pos + size > mtd_dev->geometry.capacity)
    {
        os_kprintf("read [%s] address out of bound offset:0x%x, size:0x%x, len:0x%x.\r\n",
                   dev->name, pos, size, mtd_dev->geometry.capacity);
        return 0;
    }

    page_size = mtd_dev->geometry.page_size;
    page_buff = os_calloc(1, page_size);
    if (page_buff == OS_NULL)
    {
        os_kprintf("read, memroy lack %s.\r\n", dev->name);
        return 0;
    }

    os_uint32_t page_shift  = mtd_page_shift(mtd_dev);

    page_addr = pos >> mtd_page_shift(mtd_dev);
    pos   &= page_size - 1;

    remain = size;
    while (remain != 0)
    {
        if (pos != 0 || remain < page_size)
        {
            ret = mtd_dev->mtd_ops->read_page(mtd_dev, page_addr, page_buff, 1);
        }
        else
        {
            count = remain & ~(page_size - 1);
            ret = mtd_dev->mtd_ops->read_page(mtd_dev, page_addr, buff, remain >> page_shift);
        }

        if (ret != 0)
        {
            os_kprintf("flash device(%s) read error, %d .\r\n", dev->name, page_addr);
            break;
        }

        if (pos != 0 || remain < page_size)
        {            
            count = min(page_size - pos, remain);
            memcpy(buff, page_buff + pos, count);
            page_addr++;
        }
        else
        {
            page_addr += remain >> page_shift;
        }
        
        buff   += count;
        remain -= count;
        pos += count;
        pos &= page_size - 1;
    }

    os_free(page_buff);
    return (size - remain);
}

static os_size_t char_dev_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, char_dev);

    int ret = 0;
    const os_uint8_t *buff = buffer;
    os_uint32_t page_addr;
    os_uint8_t *page_buff;
    os_uint32_t page_size;
    os_uint32_t count, remain;

    OS_ASSERT(mtd_dev);
    OS_ASSERT(buff);

    if (pos + size > mtd_dev->geometry.capacity)
    {
        os_kprintf("write [%s] address out of bound offset:0x%x, size:0x%x, len:0x%x.\r\n",
                   dev->name, pos, size, mtd_dev->geometry.capacity);
        return 0;
    }

    remain = size;
    page_size = mtd_dev->geometry.page_size;
    page_buff = os_calloc(1, page_size);
    if (page_buff == OS_NULL)
    {
        os_kprintf("write, memroy lack %s.\r\n", dev->name);
        return 0;
    }

    os_uint32_t page_shift  = mtd_page_shift(mtd_dev);
    
    page_addr = pos >> page_shift;
    pos   &= page_size - 1;

write_loop:
    if (pos != 0 || remain < page_size)
    {
        ret = mtd_dev->mtd_ops->read_page(mtd_dev, page_addr, page_buff, 1);
        if (ret != 0)
        {
            os_kprintf("pre read failed %s, %d .\r\n", dev->name, page_addr);
            goto end;
        }
    }

    while (remain != 0)
    {
        if (pos != 0 || remain < page_size)
        {
            count = min(page_size - pos, remain);
            memcpy(page_buff + pos, buff, count);
            ret = mtd_dev->mtd_ops->write_page(mtd_dev, page_addr, page_buff, 1);
        }
        else
        {
            count = remain & ~(page_size - 1);
            ret = mtd_dev->mtd_ops->write_page(mtd_dev, page_addr, buff, remain >> page_shift);
        }
        
        if (ret != 0)
        {
            os_kprintf("write failed %s, %d .\r\n", dev->name, page_addr);
            break;
        }

        if (pos != 0 || remain < page_size)
        {
            page_addr++;
        }
        else
        {
            page_addr += remain >> page_shift;
        }

        buff   += count;
        remain -= count;
        pos += count;
        pos &= page_size - 1;

        /* last not full page */
        if ((remain < page_size) && (remain != 0))
            goto write_loop;
    }

end:
    os_free(page_buff);
    return (size - remain);
}

static int char_dev_erase(os_device_t *dev, uint32_t offset, size_t size)
{
    int ret = 0;

    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, char_dev);

    OS_ASSERT(mtd_dev);

    if (offset + size > mtd_dev->geometry.capacity)
    {
        os_kprintf("part erase [%s] address out of bound offset:0x%x, size:0x%x, total:0x%x.\r\n",
                   dev->name, offset, size, mtd_dev->geometry.capacity);
        return -1;
    }

    os_uint32_t end;
    os_uint32_t page_addr;
    os_uint32_t block_size = mtd_dev->geometry.block_size;
    os_uint32_t page_shift = mtd_page_shift(mtd_dev);

    end     = offset + size;

    page_addr = offset >> page_shift;
    offset   &= ~(block_size - 1);

    if ((end - offset) & ~(block_size - 1))
    {
        ret = mtd_dev->mtd_ops->erase_block(mtd_dev, page_addr, ((end - offset) & ~(block_size - 1)) >> page_shift);
        if (ret != 0)
        {
            os_kprintf("erase failed %s, %d.\r\n", dev->name, page_addr);
            return -1;
        }
        page_addr += ((end - offset) & ~(block_size - 1)) >> page_shift;
    }

    if ((end - offset) & (block_size - 1))
    {
        ret = mtd_dev->mtd_ops->erase_block(mtd_dev, page_addr, 1);
        if (ret != 0)
        {
            os_kprintf("erase failed %s, %d.\r\n", dev->name, page_addr);
            return -2;
        }

        return ((end - offset) & ~(block_size - 1)) + block_size;
    }
    
    return (end - offset) & ~(block_size - 1);
}

static os_err_t char_dev_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t ret;
    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, char_dev);

    OS_ASSERT(mtd_dev != OS_NULL);

    if (cmd == OS_DEVICE_CTRL_MTD_GETGEOME)
    {
        struct os_mtd_geometry *geometry;

        geometry = (struct os_mtd_geometry *) args;
        if (geometry == OS_NULL)
        {
            return OS_ERROR;
        }

        memcpy(geometry, &mtd_dev->geometry, sizeof(struct os_mtd_geometry));
    }
    else if (cmd == OS_DEVICE_CTRL_MTD_ERASE)
    {
        uint32_t offset = *((os_uint32_t *)args);
        size_t size = *((os_uint32_t *)args + 1);

        ret = char_dev_erase(dev, offset, size);
        if (ret < 0)
        {
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

const static struct os_device_ops char_ops = {
    .read    = char_dev_read,
    .write   = char_dev_write,
    .control = char_dev_control,
};

static os_size_t mtd_dev_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    int ret;
    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, mtd_dev);

    os_uint32_t page_addr = pos;
    os_uint32_t page_nr   = size;

    ret = mtd_dev->mtd_ops->read_page(mtd_dev, page_addr, buffer, page_nr);

    if (ret == OS_EOK)
        return size;

    return 0;
}

static os_size_t mtd_dev_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    int ret;
    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, mtd_dev);

    os_uint32_t page_addr = pos;
    os_uint32_t page_nr   = size;
    
    ret = mtd_dev->mtd_ops->write_page(mtd_dev, page_addr, buffer, page_nr);

    if (ret == OS_EOK)
        return size;

    return 0;
}

static int mtd_dev_erase(os_device_t *dev, uint32_t page_addr, size_t page_nr)
{
    int ret = 0;
    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, mtd_dev);

    OS_ASSERT(mtd_dev);

    if (page_addr + page_nr > mtd_dev->geometry.capacity / mtd_dev->geometry.page_size)
    {
        os_kprintf("erase [%s] address out of bound page_addr:0x%x, page_nr:0x%x, total:0x%x.\r\n",
                   dev->name, page_addr, page_nr, mtd_dev->geometry.capacity / mtd_dev->geometry.page_size);
        return 0;
    }

    ret = mtd_dev->mtd_ops->erase_block(mtd_dev, page_addr, page_nr);
    if (ret != 0)
    {
        os_kprintf("erase failed %s, %d.\r\n", dev->name, page_addr);
        return 0;
    }
    
    return page_nr;
}

static os_err_t mtd_dev_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t ret;
    os_mtd_device_t *mtd_dev = (os_mtd_device_t *)os_container_of(dev, os_mtd_device_t, mtd_dev);

    OS_ASSERT(mtd_dev != OS_NULL);

    if (cmd == OS_DEVICE_CTRL_MTD_GETGEOME)
    {
        struct os_mtd_geometry *geometry;

        geometry = (struct os_mtd_geometry *) args;
        if (geometry == OS_NULL)
        {
            return OS_ERROR;
        }

        memcpy(geometry, &mtd_dev->geometry, sizeof(struct os_mtd_geometry));
    }
    else if (cmd == OS_DEVICE_CTRL_MTD_ERASE)
    {
        uint32_t page_addr = *((os_uint32_t *)args);
        size_t page_nr = *((os_uint32_t *)args + 1);

        ret = mtd_dev_erase(dev, page_addr, page_nr);
        if (ret != page_nr)
        {
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

const static struct os_device_ops mtd_ops = {
    .read    = mtd_dev_read,
    .write   = mtd_dev_write,
    .control = mtd_dev_control,
};

os_err_t mtd_device_register(os_mtd_device_t *mtd_dev, const char *name)
{
    char mtd_name[32] = {0};

    OS_ASSERT(mtd_dev->mtd_ops != OS_NULL);
    OS_ASSERT(mtd_dev->mtd_ops->read_page != OS_NULL);
    OS_ASSERT(mtd_dev->mtd_ops->write_page != OS_NULL);
    OS_ASSERT(mtd_dev->mtd_ops->erase_block != OS_NULL);

    /* mtd device */
    mtd_dev->mtd_dev.type = OS_DEVICE_TYPE_MTD;
    mtd_dev->mtd_dev.ops  = &mtd_ops;

    strcat(mtd_name, name);
    strcat(mtd_name, "block");

    os_device_register(&mtd_dev->mtd_dev, mtd_name);

    /* char device */
    mtd_dev->char_dev.type = OS_DEVICE_TYPE_CHAR;
    mtd_dev->char_dev.ops  = &char_ops;

    os_device_register(&mtd_dev->char_dev, name);

    return OS_EOK;
}

os_err_t mtd_device_unregister(os_mtd_device_t *mtd_dev)
{
    os_device_unregister(&mtd_dev->char_dev);
    os_device_unregister(&mtd_dev->mtd_dev);
    return OS_EOK;
}


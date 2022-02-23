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
 * @file        mtd_nor_test.c
 *
 * @brief       The test file for mtd nor.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_memory.h>
#include <string.h>
#include <shell.h>
#include <string.h>
#include <stdlib.h>
#include <driver.h>
#include <fal/fal.h>

#ifdef OS_USING_VFS
#include <vfs_fs.h>
#endif

static int fal_read(const char *name, os_off_t offset, os_uint32_t length)
{
    os_size_t      count;
    os_uint8_t    *data;

    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        os_kprintf("invalide fal part!\r\n");
        return OS_ERROR;
    }

    os_kprintf("part page size: 0x%x, erase size:0x%x, size:0x%x\r\n", 
               fal_part_page_size(part), 
               fal_part_block_size(part), 
               fal_part_size(part));

    data = os_calloc(1, length);
    if (data == OS_NULL)
    {
        os_kprintf("out of memory!\r\n");
        return OS_ENOMEM;
    }

    memset(data, 0xff, length);

    count = fal_part_read(part, offset, data, length);

    os_kprintf("read offset:0x%x count=%d/%d\r\n", offset, count, length);

    if (count == length)
    {
        os_kprintf("read success\r\n");
        hex_dump(data, length);
    }
    else
    {
        os_kprintf("read failed %d/%d\r\n", count, length);
    }

    os_free(data);
    return OS_EOK;
}

static int fal_write(const char *name, os_off_t offset, os_uint32_t length)
{
    os_size_t      count;
    os_uint8_t    *data;
    os_uint32_t    index;

    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        os_kprintf("invalide fal part!\r\n");
        return OS_ERROR;
    }

    os_kprintf("part page size: 0x%x, erase size:0x%x, size:0x%x\r\n", 
               fal_part_page_size(part), 
               fal_part_block_size(part), 
               fal_part_size(part));

    data = os_calloc(1, length);
    if (data == OS_NULL)
    {
        os_kprintf("out of memory!\r\n");
        return OS_ENOMEM;
    }

    for (index = 0; index < length; index++)
    {
        data[index] = index & 0xff;
    }

    count = fal_part_write(part, offset, data, length);

    os_kprintf("write offset:0x%x count=%d/%d\r\n", offset, count, length);

    if (count == length)
        os_kprintf("write success\r\n");
    else
        os_kprintf("write failed %d/%d\r\n", count, length);

    os_free(data);
    return OS_EOK;
}

static int fal_erase(const char *name, os_off_t offset, os_size_t length)
{
    os_size_t      count;
    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        os_kprintf("invalide fal part!\r\n");
        return OS_ERROR;
    }

    os_kprintf("part page size: 0x%x, erase size:0x%x, size:0x%x\r\n", 
               fal_part_page_size(part), 
               fal_part_block_size(part), 
               fal_part_size(part));

    count = fal_part_erase(part, offset, length);
    if (count == length)
        os_kprintf("erase success\r\n");
    else
        os_kprintf("erase failed %d/%d\r\n", count, length);

    return OS_EOK;
}

static int fal_erase_all(const char *name)
{
    os_size_t      count;

    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        os_kprintf("invalide fal part!\r\n");
        return OS_ERROR;
    }

    os_kprintf("part erase size:0x%x, size:0x%x\r\n", fal_part_block_size(part), fal_part_size(part));

    count = fal_part_erase_all(part);
    if (count == part->info->size)
        os_kprintf("erase success\r\n");
    else
        os_kprintf("erase failed %d/%d\r\n", count, part->info->size);

    return OS_EOK;
}

static int fal_write_read(const char *name, os_off_t offset, os_uint32_t length)
{
    os_size_t    count;
    os_uint8_t  *data;
    os_uint32_t  index;
    os_uint16_t  write_crc = 0;
    os_uint16_t  read_crc = 0;

    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        os_kprintf("invalide fal part!\r\n");
        return OS_ERROR;
    }

    os_kprintf("part page size: 0x%x, erase size:0x%x, size:0x%x\r\n", 
               fal_part_page_size(part), 
               fal_part_block_size(part), 
               fal_part_size(part));

    data = os_calloc(1, length);
    if (data == OS_NULL)
    {
        os_kprintf("out of memory!\r\n");
        return OS_ENOMEM;
    }

    /* write */
    for (index = 0; index < length; index++)
    {
        data[index] = rand();
    }

    write_crc = crc16(write_crc, data, length);

    count = fal_part_write(part, offset, data, length);

    os_kprintf("write offset:0x%x count=%d/%d\r\n", offset, count, length);

    if (count == length)
        os_kprintf("write success\r\n");
    else
        os_kprintf("write failed %d/%d\r\n", count, length);

    /* read */
    memset(data, 0xff, length);

    count = fal_part_read(part, offset, data, length);

    os_kprintf("read offset:0x%x count=%d/%d\r\n", offset, count, length);

    if (count == length)
    {
        os_kprintf("read success\r\n");
        read_crc = crc16(read_crc, data, length);
        if (write_crc == read_crc)
        {
            os_kprintf("write read success, crc %08x == %08x\r\n", write_crc, read_crc);
        }
        else
        {
            os_kprintf("write read failed, crc %08x != %08x\r\n", write_crc, read_crc);
        }
    }
    else
    {
        os_kprintf("read failed %d/%d\r\n", count, length);
    }

    os_free(data);
    return OS_EOK;
}

static int fal_lock(const char *name)
{
    int ret;

    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        os_kprintf("invalide fal part!\r\n");
        return OS_ERROR;
    }

    os_kprintf("part erase size:0x%x, size:0x%x\r\n", fal_part_block_size(part), fal_part_size(part));

    ret = fal_part_lock(part);
    if (ret == 0)
        os_kprintf("lock success\r\n");
    else
        os_kprintf("lock failed %d\r\n", ret);

    return OS_EOK;
}

static int fal_unlock(const char *name)
{
    int ret;

    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        os_kprintf("invalide fal part!\r\n");
        return OS_ERROR;
    }

    os_kprintf("part erase size:0x%x, size:0x%x\r\n", fal_part_block_size(part), fal_part_size(part));

    ret = fal_part_unlock(part);
    if (ret == 0)
        os_kprintf("unlock success\r\n");
    else
        os_kprintf("unlock failed %d\r\n", ret);

    return OS_EOK;
}

int fal_mount(const char *part_name, const char *mount_point, const char *fs_type)
{
#ifdef OS_USING_VFS
    if (fal_blk_device_create(part_name))
    {
        os_kprintf("Create a block device on the %s partition of flash successful.\r\n", part_name);
    }
    else
    {
        os_kprintf("Can't create a block device on '%s' partition.\r\n", part_name);
    }

    if (vfs_mount(part_name, mount_point, fs_type, 0, 0) == 0)
    {
        os_kprintf("filesystem mount successful.\r\n");
    }
    else
    {
        os_kprintf("filesystem mount fail.\r\n");
        
        if (strcmp(fs_type, "fat") == 0)
        {
            os_kprintf("You should mkfs first, then reset board ! cmd: mkfs -t fat %s\r\n", part_name);
        }
    }
    
#else
    os_kprintf("filesystem unsupport.\r\n");
#endif

    return 0;
}

static void fal(int argc, char **argv)
{
    if (argc < 3)
    {
help:
        os_kprintf("\r\n");
        os_kprintf("fal [OPTION] [PARAM ...]\r\n");
        os_kprintf("    read       <part> <off> <len>  Read <len> Bytes from <off> of <part>\r\n");
        os_kprintf("    write      <part> <off> <len>  Write <len> Bytes from <off> of <part>\r\n");
        os_kprintf("    erase      <part> <off> <len>  Erase <len> Bytes from <off> of <part>\r\n");
        os_kprintf("    eraseall   <part>              Erase all data on <part>\r\n");
        os_kprintf("    write_read <part> <off> <len>  Write & Read <len> Bytes from <off> of <part>\r\n");
        os_kprintf("    lock       <part>              Lock <part>\r\n");
        os_kprintf("    unlock     <part>              Unlock <part>\r\n");
        os_kprintf("    mount      <part> [dir] [fs]   Mount <part> [/] [fat, jffs2]\r\n");
        return;
    }

    if (!strcmp(argv[1], "read"))
    {
        if (argc < 5)
        {
            os_kprintf("The input parameters are too few!\r\n");
            goto help;
        }
        fal_read(argv[2], strtol(argv[3], NULL, 0), strtol(argv[4], NULL, 0));
    }
    else if (!strcmp(argv[1], "write"))
    {
        if (argc < 5)
        {
            os_kprintf("The input parameters are too few!\r\n");
            goto help;
        }
        fal_write(argv[2], strtol(argv[3], NULL, 0), strtol(argv[4], NULL, 0));
    }
    else if (!strcmp(argv[1], "erase"))
    {
        if (argc < 5)
        {
            os_kprintf("The input parameters are too few!\r\n");
            goto help;
        }
        fal_erase(argv[2], strtol(argv[3], NULL, 0), strtol(argv[4], NULL, 0));
    }
    else if (!strcmp(argv[1], "eraseall"))
    {
        fal_erase_all(argv[2]);
    }
    else if (!strcmp(argv[1], "write_read"))
    {
        if (argc < 5)
        {
            os_kprintf("The input parameters are too few!\r\n");
            goto help;
        }
        fal_write_read(argv[2], strtol(argv[3], NULL, 0), strtol(argv[4], NULL, 0));
    }
    else if (!strcmp(argv[1], "lock"))
    {
        fal_lock(argv[2]);
    }
    else if (!strcmp(argv[1], "unlock"))
    {
        fal_unlock(argv[2]);
    }
    else if (!strcmp(argv[1], "mount"))
    {
        const char *dir = "/";
        const char *fs  = "fat";
        
        if (argc >= 4)
            dir = argv[3];
        
        if (argc >= 5)
            fs = argv[4];

        fal_mount(argv[2], dir, fs);
    }
    else
    {
        os_kprintf("Input parameters are not supported!\r\n");
        goto help;
    }
}

SH_CMD_EXPORT(fal, fal, "fal test");


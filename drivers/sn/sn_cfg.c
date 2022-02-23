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
#include <board.h>
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
#include <ctype.h>

#ifdef OS_USING_VFS
#include <vfs_fs.h>
#endif

#include "dlog.h"

#define DBG_TAG   "drv.sn_cfg"

#define SN_PARTITION_NAME "cfg"
#define SN_ADDR_OFFSET 0
#define SN_LENGTH 20


static void char_dump(unsigned char *buff, int count)
{
    int i;
    os_kprintf("sn:");
    for (i = 0; i < count; i++)
    {
        if (buff[i] == 0xff)
            os_kprintf("?");
        else if (isalnum(buff[i]))
            os_kprintf("%c", buff[i]);
        else
            os_kprintf("?");
    }

    os_kprintf("\r\n");
}

static int sn_read(const char *name, os_off_t offset, os_uint32_t length)
{
    os_size_t      count;
    os_uint8_t    *data;

    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        LOG_E(DBG_TAG, "invalide fal part!\r\n");
        return OS_ERROR;
    }
   
    data = os_malloc(length);
    if (data == OS_NULL)
    {
        LOG_E(DBG_TAG, "out of memory!\r\n");
        return OS_ENOMEM;
    }

    memset(data, 0xff, length);

    count = fal_part_read(part, offset, data, length);

    if (count == length)
    {
        char_dump(data, length);
    }
    else
    {
        LOG_E(DBG_TAG, "read failed %d/%d\r\n", count, length);
    }

    os_free(data);
    return OS_EOK;
}

static int sn_write(const char *name, os_off_t offset, char * buf, os_uint32_t length)
{
    os_size_t      count;
    os_size_t      block_size;
    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        LOG_E(DBG_TAG, "invalide fal part!\r\n");
        return OS_ERROR;
    }

    fal_part_unlock(part);
    
    block_size = fal_part_block_size(part);
    
    count = fal_part_erase(part, offset, block_size);
    
    if (count != block_size)
    {
        LOG_E(DBG_TAG, "erase failed %d/%d\r\n", count, block_size);
        fal_part_lock(part);
        return OS_ERROR;
    }
    
    count = fal_part_write(part, offset, (os_uint8_t *)buf, length);

    fal_part_lock(part);
    
    if (count == length)
    {
        LOG_D(DBG_TAG, "write success\r\n");
        return OS_EOK;
    }
    else
    {
        LOG_E(DBG_TAG, "write failed %d/%d\r\n", count, length);
        return OS_ERROR;
    }
    
}

static int sn_erase(const char *name, os_off_t offset)
{
    os_size_t      count;
    os_size_t      block_size;
    fal_part_t *part;

    part = fal_part_find(name);

    if (part == OS_NULL)
    {
        LOG_E(DBG_TAG, "invalide fal part!\r\n");
        return OS_ERROR;
    }

    fal_part_unlock(part);

    block_size = fal_part_block_size(part);
    
    count = fal_part_erase(part, offset, block_size);
    
    fal_part_lock(part);
    
    if (count == block_size)
    {
        LOG_D(DBG_TAG, "erase success\r\n");
        return OS_EOK;
    }
    else
    {
        LOG_E(DBG_TAG, "erase failed %d/%d\r\n", count, block_size);
        return OS_ERROR;
    }
}

static void sn(int argc, char **argv)
{
    if ((argc > 3) || (argc == 1))
    {
help:
        os_kprintf("\r\n");
        os_kprintf("sn [OPTION] [PARAM ...]\r\n");
        os_kprintf("    read       Read <%d> Bytes from <%d> of <%s>\r\n", SN_LENGTH, SN_ADDR_OFFSET, SN_PARTITION_NAME);
        os_kprintf("    write      Write <%d> Bytes from <%d> of <%s>\r\n", SN_LENGTH, SN_ADDR_OFFSET, SN_PARTITION_NAME);
        return;
    }

    if (!strcmp(argv[1], "read"))
    {
        if (argc != 2)
        {
            os_kprintf("The input parameters are not correct!\r\n");
            goto help;
        }
        sn_read(SN_PARTITION_NAME, SN_ADDR_OFFSET, SN_LENGTH);
    }
    else if (!strcmp(argv[1], "write"))
    {
        if (argc != 3)
        {
            os_kprintf("The input parameters are not correct!\r\n");
            goto help;
        }
        if (strlen(argv[2]) == 20)
        {
            if (OS_EOK == sn_write(SN_PARTITION_NAME, SN_ADDR_OFFSET, argv[2], SN_LENGTH))
            {
                os_kprintf("write success\r\n");
            }
            else
            {
                os_kprintf("write fail\r\n");
            }
        }
        else
        {
            os_kprintf("SN char number is not euqal 20!\r\n");
        }
    }
    else if (!strcmp(argv[1], "erase"))
    {
        if (argc != 2)
        {
            os_kprintf("The input parameters are not correct!\r\n");
            goto help;
        }
        
        if (OS_EOK == sn_erase(SN_PARTITION_NAME, SN_ADDR_OFFSET))
        {
            os_kprintf("erase success\r\n");
        }
        else
        {
            os_kprintf("erase erase\r\n");
        }
    }
    else
    {
        os_kprintf("Input parameters are not supported!\r\n");
        goto help;
    }
}

SH_CMD_EXPORT(sn, sn, "sn config");


static int slot_no_read(void)
{
    int ret = 0;
    int i;
    for (i = 0; i < slot_no_pin_tab_size; i++)
    {
        ret = ret << 1;
        ret |= os_pin_read(slot_no_pin_tab[i]);
    }
    return ret;
}

static int board_no_read(void)
{
    int ret = 0;
    int i;
    for (i = 0; i < board_no_pin_tab_size; i++)
    {
        ret = ret << 1;
        ret |= os_pin_read(board_no_pin_tab[i]);
    }
    return ret;
}

static os_err_t board_no_pin_init(void)
{
    int i;
    for (i = 0; i < board_no_pin_tab_size; i++)
    {
        os_pin_mode(board_no_pin_tab[i], PIN_MODE_INPUT);
    }
    
    for (i = 0; i < slot_no_pin_tab_size; i++)
    {
        os_pin_mode(slot_no_pin_tab[i], PIN_MODE_INPUT);
    }
    return OS_EOK;
}
OS_DEVICE_INIT(board_no_pin_init, OS_INIT_SUBLEVEL_HIGH);

static os_err_t get_device_info(os_int32_t argc, char **argv)
{
    int board_no;
    int slot_no;
    board_no = board_no_read();
    slot_no  = slot_no_read();
    sn_read(SN_PARTITION_NAME, SN_ADDR_OFFSET, SN_LENGTH);
    os_kprintf("type:%s\r\n", SOC_MODEL);
    os_kprintf("version:%s-%s\r\n", __DATE__, __TIME__);
    os_kprintf("back_bd_no:%d\r\n", board_no);
    os_kprintf("core_bd_slot:%d\r\n", slot_no);

    return OS_EOK;
}
SH_CMD_EXPORT(get_device_info, get_device_info, "show board info");


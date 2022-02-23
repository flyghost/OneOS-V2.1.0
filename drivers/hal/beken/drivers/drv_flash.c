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
 * @file        drv_flash.c
 *
 * @brief       This file provides beken flash read/write functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-29   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_mutex.h>
#include <os_types.h>
#include <os_dbg.h>


#ifdef BEKEN_USING_FLASH

#include "typedef.h"
#include "drv_flash.h"
#include "flash.h"

#define DBG_SECTION_NAME  "[FLASH]"

static struct os_mutex flash_mutex;


int beken_flash_read(os_uint32_t address, void *data, os_uint32_t size)
{
    if (size == 0)
    {
        LOG_D(DBG_SECTION_NAME, "flash read len is NULL\n");
        return -1;
    }
    os_mutex_lock(&flash_mutex,OS_IPC_WAITING_FOREVER);
    flash_read(data, size, address);
    os_mutex_unlock(&flash_mutex);
    return 0;
}

int beken_flash_write(os_uint32_t address, const void *data, os_uint32_t size)
{
    if (size == 0)
    {
        LOG_D(DBG_SECTION_NAME, "flash write len is NULL\n");
        return -1;
    }

    os_mutex_lock(&flash_mutex,OS_IPC_WAITING_FOREVER);
    flash_write((char *)data, size, address);
    os_mutex_unlock(&flash_mutex);
    
    return 0;
}

int beken_flash_erase(os_uint32_t address)
{
    os_mutex_lock(&flash_mutex,OS_IPC_WAITING_FOREVER);
    address &= (0xFFF000);
    flash_ctrl(CMD_FLASH_ERASE_SECTOR, &address);
    os_mutex_unlock(&flash_mutex);
    
    return 0;
}

/*erase current sector*/
int beken_flash_erase_with_len(os_uint32_t address, os_uint32_t size)
{
    os_mutex_lock(&flash_mutex,OS_IPC_WAITING_FOREVER);
    address &= (0xFFF000);
    os_uint32_t cnt = 0;
    os_uint32_t addrtmp = address;
    for(cnt = 0; cnt<((size - 1) / 4096 + 1); cnt++)
    {
        addrtmp += cnt*4096;
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &addrtmp);
    }
    os_mutex_unlock(&flash_mutex);
    
    return 0;
}


static int beken_flash_init(void)
{
    return os_mutex_init(&flash_mutex, "flash", OS_IPC_FLAG_PRIO, OS_FALSE);
}

OS_PREV_INIT(beken_flash_init);

#endif



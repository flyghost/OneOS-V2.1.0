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
 * @file        drv_flash_f1.c
 *
 * @brief       This file provides flash read/write/erase functions for f1.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"

#include "drv_flash.h"

#define LOG_TAG "drv.flash"
#include <drv_log.h>

#define DBG_TAG "drv.flash"

#if defined (HK32F10X_HD) || defined (HK32F10X_MD) || defined (HK32F10X_MDA)
#define FLASH_PAGE_SIZE    ((uint16_t)0x800)
#else
#define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#endif

/**
 ***********************************************************************************************************************
 * @brief           hk32_flash_read:Read data from flash,and this operation's units is word.
 *
 * @param[in]       addr            flash address.
 * @param[out]      buf             buffer to store read data.
 * @param[in]       size            read bytes size.
 *
 * @return          Return read size or status.
 * @retval          size            read bytes size.
 * @retval          Others          read failed.
 ***********************************************************************************************************************
 */
int hk32_flash_read(os_uint32_t addr, os_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > HK32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return OS_EINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(os_uint8_t *)addr;
    }

    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           Write data to flash.This operation's units is word.
 *
 * @attention       This operation must after erase.
 *
 * @param[in]       addr            flash address.
 * @param[in]       buf             the write data buffer.
 * @param[in]       size            write bytes size.
 *
 * @return          Return write size or status.
 * @retval          size            write bytes size.
 * @retval          Others          write failed.
 ***********************************************************************************************************************
 */
int hk32_flash_write(os_uint32_t addr, const os_uint8_t *buf, size_t size)
{
    uint32_t i;

    FLASH_Unlock();
    
    if (addr % 4 != 0 || size % 4 != 0)
    {
        return -1;
    }

    for (i = 0; i < size / 4; i++)
    {
        FLASH_ProgramWord(addr + i * 4, *((uint32_t *)(buf + i * 4)));
    }
    
    FLASH_Lock();

    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           Erase data on flash.This operation is irreversible and it's units is different which on many chips.
 *
 * @param[in]       addr            Flash address.
 * @param[in]       size            Erase bytes size.
 *
 * @return          Return erase result or status.
 * @retval          size            Erase bytes size.
 * @retval          Others          Erase failed.
 ***********************************************************************************************************************
 */
int hk32_flash_erase(os_uint32_t addr, size_t size)
{
    FLASH_Status status;
    
    uint32_t page_num = size / FLASH_PAGE_SIZE;
    uint32_t EraseCounter;
    
    if ((addr + size) > HK32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: erase outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }
    
    if (size % FLASH_PAGE_SIZE != 0)
    {
        page_num += 1;
    }
    
    FLASH_Unlock();
    for (EraseCounter = 0; (EraseCounter < page_num) && (status == FLASH_COMPLETE); EraseCounter++)
    {
        status = FLASH_ErasePage(addr + (FLASH_PAGE_SIZE * EraseCounter));
    }
    FLASH_Lock();

    if (status != FLASH_COMPLETE)
    {
        return OS_ERROR;
    }

    LOG_D(DBG_TAG,"erase done: addr (0x%p), size %d", (void *)addr, size);
    return size;
}

#ifdef OS_USING_FAL

#define HK32_FLASH_BLOCK_SIZE  (FLASH_PAGE_SIZE)
#define HK32_FLASH_PAGE_SIZE   (4)

#include "fal_drv_flash.c"

#endif


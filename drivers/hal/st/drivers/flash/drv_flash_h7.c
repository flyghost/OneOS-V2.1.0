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
 * @file        drv_flash_h7.c
 *
 * @brief       This file provides flash read/write/erase functions for h7.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <string.h>
#include "drv_flash.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DBG_TAG "drv.flash"
#include <drv_log.h>

static uint32_t GetSector(uint32_t Addr)
{
    uint32_t sector = 0;

    sector = (Addr - FLASH_BASE) / FLASH_SECTOR_SIZE % FLASH_SECTOR_TOTAL;

    return sector;
}

/**
 ***********************************************************************************************************************
 * @brief            Gets the bank of a given address
 *
 * @param[in]       Addr            Address of the FLASH Memory
 *
 * @return          [Return the bank of a given address.
 ***********************************************************************************************************************
 */
static uint32_t GetBank(uint32_t Addr)
{
    uint32_t bank = 0;
#ifndef FLASH_BANK_2
    bank = FLASH_BANK_1;
#else
    FLASH_OBProgramInitTypeDef OBInit;

    /* Get the Dual boot configuration status */
    HAL_FLASHEx_OBGetConfig(&OBInit);

    /* Get FLASH_WRP_SECTORS write protection status */
    OBInit.Banks = FLASH_BANK_1;
    HAL_FLASHEx_OBGetConfig(&OBInit);

    /* Check Swap Flash banks  status */
    if ((OBInit.USERConfig & OB_SWAP_BANK_ENABLE) == OB_SWAP_BANK_DISABLE)
    {
        /* No Bank swap */
        if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
        {
            bank = FLASH_BANK_1;
        }
        else
        {
            bank = FLASH_BANK_2;
        }
    }
    else
    {
        /* Bank swap */
        if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
        {
            bank = FLASH_BANK_2;
        }
        else
        {
            bank = FLASH_BANK_1;
        }
    }
#endif
    return bank;
}

/**
 ***********************************************************************************************************************
 * @brief           stm32_flash_read:Read data from flash,and this operation's units is word.
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
int stm32_flash_read(os_uint32_t addr, os_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
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
int stm32_flash_write(os_uint32_t addr, const uint8_t *buf, size_t size)
{
    os_err_t result = 0;
    size_t   remain = size;
    uint8_t  FlashWord[32];
    os_base_t   level;
    HAL_StatusTypeDef status;
    
    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: write outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 32 != 0)
    {
        LOG_E(DBG_TAG,"write addr must be 32-byte alignment");
        return OS_EINVAL;
    }

    if (size == 0)
    {
        LOG_E(DBG_TAG,"write size 0");
        return OS_ERROR;
    }

    HAL_FLASH_Unlock();

    while (remain > 0)
    {
        if (remain >= 32)
        {
            memcpy(FlashWord, buf, 32);
            buf += 32;
            remain -= 32;
        }
        else
        {
            memcpy(FlashWord, buf, remain);
            memset(FlashWord + remain, 0, 32 - remain);
            buf += remain;
            remain = 0;
        }

        level = os_irq_lock();

        /* write data */
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr, (uint32_t)FlashWord);
        
        os_irq_unlock(level);
        
        if (status == HAL_OK)
        {
            /* Check the written value */
            if (memcmp((void *)addr, FlashWord, 32))
            {
                LOG_E(DBG_TAG,"ERROR: write data != read data\r\n");
                result = OS_ERROR;
                goto __exit;
            }
        }
        else
        {
            result = OS_ERROR;
            goto __exit;
        }

        addr += 32;
    }

__exit:
    HAL_FLASH_Lock();
    if (result != 0)
    {
        return result;
    }

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
int stm32_flash_erase(os_uint32_t addr, size_t size)
{
    os_err_t               result      = OS_EOK;
    uint32_t               FirstSector = 0;
    uint32_t               NbOfSectors = 0;
    uint32_t               SECTORError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    os_uint32_t            addr_end = addr + size - 1;

    if (addr_end > STM32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: erase outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    HAL_FLASH_Unlock();

    int start_bank = GetBank(addr);
    int end_bank   = GetBank(addr_end);

    if (start_bank != end_bank)
    {
        FirstSector = GetSector(addr);
        NbOfSectors = GetSector(FLASH_BASE + FLASH_BANK_SIZE - 1) - FirstSector + 1;

        EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        EraseInitStruct.Banks        = start_bank;
        EraseInitStruct.Sector       = FirstSector;
        EraseInitStruct.NbSectors    = NbOfSectors;

        result = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
        if (result != HAL_OK)
        {
            LOG_E(DBG_TAG,"ERROR: erase failed! addr 0x%x sector %d, %d\r\n", addr, FirstSector, NbOfSectors);
            goto __exit;
        }

        addr = FLASH_BASE + FLASH_BANK_SIZE;
    }

    if (addr != addr_end)
    {
        FirstSector = GetSector(addr);
        NbOfSectors = GetSector(addr_end) - FirstSector + 1;

        EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        EraseInitStruct.Banks        = end_bank;
        EraseInitStruct.Sector       = FirstSector;
        EraseInitStruct.NbSectors    = NbOfSectors;

        result = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
        if (result != HAL_OK)
        {
            LOG_E(DBG_TAG,"ERROR: erase failed! addr 0x%x sector %d, %d\r\n", addr, FirstSector, NbOfSectors);
            goto __exit;
        }
    }

__exit:
    HAL_FLASH_Lock();

    if (result != OS_EOK)
    {
        return result;
    }

    LOG_D(DBG_TAG,"erase done: addr (0x%p), size %d", (void *)addr, size);
    return size;
}

#define FLASH_PAGE_SIZE FLASH_SECTOR_SIZE

#define STM32_FLASH_BLOCK_SIZE  (FLASH_PAGE_SIZE)
#define STM32_FLASH_PAGE_SIZE   (32)

#include "fal_drv_flash.c"

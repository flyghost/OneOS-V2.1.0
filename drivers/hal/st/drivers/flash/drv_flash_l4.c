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
 * @file        drv_flash_l4.c
 *
 * @brief       This file provides flash read/write/erase functions for l4.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_flash.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DBG_TAG "drv.flash"
#include <drv_log.h>

/**
 ***********************************************************************************************************************
 * @brief           GetPage:Gets the page of a given address
 *
 * @param[in]       Addr            Address of the FLASH Memory
 *
 * @return          Return the page number.
 * @retval          page            The page of a given address
 ***********************************************************************************************************************
 */
static uint32_t GetPage(uint32_t Addr)
{
    uint32_t page = 0;

    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;

    return page;
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
    if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
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
    size_t      i, j;
    os_err_t    result     = 0;
    os_uint64_t write_data = 0, temp_data = 0;
    os_base_t   level;
    HAL_StatusTypeDef status;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: write outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 8 != 0)
    {
        LOG_E(DBG_TAG,"write addr must be 8-byte alignment");
        return OS_EINVAL;
    }

    if (size == 0)
    {
        LOG_E(DBG_TAG,"write size 0");
        return OS_ERROR;
    }

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGSERR | FLASH_FLAG_OPTVERR);

    for (i = 0; i < size;)
    {
        if ((size - i) < 8)
        {
            for (j = 0; (size - i) > 0; i++, j++)
            {
                temp_data  = *buf;
                write_data = (write_data) | (temp_data << 8 * j);
                buf++;
            }
        }
        else
        {
            for (j = 0; j < 8; j++, i++)
            {
                temp_data  = *buf;
                write_data = (write_data) | (temp_data << 8 * j);
                buf++;
            }
        }

        level = os_irq_lock();
        
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, write_data);
        
        os_irq_unlock(level);
        
        /* write data */
        if (status == HAL_OK)
        {
            /* Check the written value */
            if (*(uint64_t *)addr != write_data)
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

        temp_data  = 0;
        write_data = 0;

        addr += 8;
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
    os_err_t result    = OS_EOK;
    uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
    uint32_t PageNbOfBank = 0;
    uint32_t PAGEError    = 0;

    PageNbOfBank = FLASH_BANK_SIZE / FLASH_PAGE_SIZE;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: erase outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    FLASH_EraseInitTypeDef EraseInitStruct;     /* Variable used for Erase procedure */
    HAL_FLASH_Unlock();                         /* Unlock the Flash to enable the flash control register access */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS); /* Clear All errors flags */
    FirstPage            = GetPage(addr);       /* Get the 1st page to erase */
    EraseInitStruct.Page = FirstPage;

    /* Get the bank */
    BankNumber = GetBank(addr);

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; /* FLASH_TYPEERASE_MASSERASE or FLASH_TYPEERASE_PAGES */

    if (size % FLASH_PAGE_SIZE == 0)
        NbOfPages = size / FLASH_PAGE_SIZE; /* Calculate the number of pages*/
    else
        NbOfPages = size / FLASH_PAGE_SIZE + 1; /* Calculate the number of pages*/

    if (EraseInitStruct.Page < PageNbOfBank && (NbOfPages + EraseInitStruct.Page) > PageNbOfBank)
    {
        if ((NbOfPages + EraseInitStruct.Page) > PageNbOfBank * 2)
        {
            LOG_E(DBG_TAG,"ERROR: erase outrange flash size!\r\n");
            goto __exit;
        }

        EraseInitStruct.Banks   = BankNumber;
        EraseInitStruct.NbPages = PageNbOfBank - EraseInitStruct.Page;

        result = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
        if (result != HAL_OK)
            goto __exit;

#ifdef FLASH_BANK_2
        if (BankNumber == FLASH_BANK_1)
        {
            BankNumber = FLASH_BANK_2;
        }
        else
        {
            BankNumber = FLASH_BANK_1;
        }

        EraseInitStruct.Banks   = BankNumber;
        EraseInitStruct.Page    = 0;
        EraseInitStruct.NbPages = NbOfPages - EraseInitStruct.NbPages;

        result = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
        if (result != HAL_OK)
            goto __exit;
#endif
    }
    else if (EraseInitStruct.Page < PageNbOfBank && (NbOfPages + EraseInitStruct.Page) <= PageNbOfBank)
    {
        EraseInitStruct.Banks   = BankNumber;
        EraseInitStruct.NbPages = NbOfPages;
        result = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
        if (result != HAL_OK)
            goto __exit;
    }
    else if (EraseInitStruct.Page >= PageNbOfBank && (NbOfPages + EraseInitStruct.Page <= PageNbOfBank * 2))
    {
        EraseInitStruct.Banks   = BankNumber;
        EraseInitStruct.Page    = EraseInitStruct.Page - PageNbOfBank;
        EraseInitStruct.NbPages = NbOfPages;
        result = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
        if (result != HAL_OK)
            goto __exit;
    }
    else
    {
        LOG_E(DBG_TAG,"ERROR: erase outrange flash size!\r\n");
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

#define STM32_FLASH_BLOCK_SIZE  (FLASH_PAGE_SIZE)
#define STM32_FLASH_PAGE_SIZE   (8)

#include "fal_drv_flash.c"


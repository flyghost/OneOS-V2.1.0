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
 * @file        drv_flash_f3.c
 *
 * @brief        This file provides flash read/write/erase functions for f0.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"

#include "drv_flash.h"

#define LOG_TAG "drv.flash"
#include <drv_log.h>
#define FLASH_PAGE_SIZE           ((uint16_t)0x800U)
/**
 ***********************************************************************************************************************
 * @brief           get_page: Gets the page of a given address
 *
 * @param[in]       Addr            Address of the FLASH Memory
 *
 * @return          Return the page number.
 * @retval          page            The page of a given address
 ***********************************************************************************************************************
 */
static uint32_t get_page(uint32_t addr)
{
    uint32_t page = 0;
    page          = OS_ALIGN_DOWN(addr, FLASH_PAGE_SIZE);
    return page;
}

/**
 ***********************************************************************************************************************
 * @brief           gd32_flash_read:Read data from flash,and this operation's units is word.
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
int gd32_flash_read(os_uint32_t addr, os_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > GD32_FLASH_END_ADDRESS)
    {
        LOG_EXT_E("read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return OS_EINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(os_uint8_t *)addr;
    }

    return size;
}

/*!
    \brief      program fmc word by word 
    \param[in]  none
    \param[out] none
    \retval     none
*/
int gd32_flash_write(os_uint32_t addr, const os_uint8_t *buf, size_t size)
{
    size_t i;
    size_t j;
    os_err_t    result     = 0;
    os_uint32_t write_data = 0;
    os_uint32_t temp_data = 0;
    /* unlock the flash program/erase controller */
    fmc_unlock();

    if (size < 1)
    {
        return OS_ERROR;
    }

    for (i = 0; i < size;)
    {
        if ((size - i) < 4)
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
            for (j = 0; j < 4; j++, i++)
            {
                temp_data  = *buf;
                write_data = (write_data) | (temp_data << 8 * j);
                buf++;
            }
        }

        /* write data */
        if (fmc_word_program(addr, write_data) == FMC_READY)
        {
            /* Check the written value */
            if (*(uint32_t *)addr != write_data)
            {
                LOG_EXT_E("ERROR: write data != read data\n");
                result = OS_ERROR;
                goto __exit;
            }
        }
        else
        {
            result = OS_ERROR;
            goto __exit;
        }
        
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        
        temp_data  = 0;
        write_data = 0;

        addr += 4;
    }

__exit:
    /* lock the main FMC after the program operation */
    fmc_lock();
    if (result != 0)
    {
        return result;
    }

    return size;
    
}


/*!
    \brief      erase fmc pages
    \param[in]  none
    \param[out] none
    \retval     none
*/
int gd32_flash_erase(os_uint32_t addr, size_t size)
{
    uint32_t erase_cnt;
    uint32_t page_num;
    uint32_t page_idx;
    page_num = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    
    page_idx = get_page(addr);
    
    /* erase the flash pages */
    for(erase_cnt = 0; erase_cnt < page_num; erase_cnt++){
        fmc_page_erase(page_idx + (FLASH_PAGE_SIZE * erase_cnt));
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    }

    /* lock the main FMC after the erase operation */
    fmc_lock();
    return size;
}


#define GD32_FLASH_BLOCK_SIZE  (FLASH_PAGE_SIZE)
#define GD32_FLASH_PAGE_SIZE   (4)

#include "fal_drv_flash.c"

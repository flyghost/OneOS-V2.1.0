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
 * @file        drv_flash_f2.c
 *
 * @brief        This file provides flash read/write/erase functions for f2.
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

#define FLASH_PAGE_SIZE           ((uint32_t)(2*1024))
#define FLASH_BANK0_PAGE_SIZE     ((uint32_t)(2*1024))
#define FLASH_BANK1_PAGE_SIZE     ((uint32_t)(4*1024))
#define FMC_BANK1_SART_ADDRESS    ((uint32_t)(FMC_BANK0_END_ADDRESS + 1))

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
    
    if (addr < FMC_BANK1_SART_ADDRESS)
    {
        page = OS_ALIGN_DOWN(addr, FLASH_BANK0_PAGE_SIZE);
    }
    else
    {
        page = OS_ALIGN_DOWN(addr, FLASH_BANK1_PAGE_SIZE);
    }
    
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
        LOG_E(LOG_TAG, "read outrange flash size! addr is (0x%p)", (void *)(addr + size));
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
 * @brief           gd32_flash_erase:Erase data of flash,and this operation's units is word.
 *
 * @param[in]       addr            flash address.
 * @param[in]       size            erase bytes size.
 *
 * @return          Return erase size or status.
 * @retval          size            erase bytes size.
 * @retval          Others          erase failed.
 ***********************************************************************************************************************
 */
int gd32_flash_erase(os_uint32_t addr, size_t size)
{
    uint32_t erase_cnt;
    uint32_t page_num;
    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t page_start_addr;
    
    start_addr = addr;
    end_addr = start_addr + size - 1;
    
    /* get page address */
    page_start_addr = get_page(start_addr);
        
    /* ensure flash be erased from first address of any page */
    if (page_start_addr != start_addr)
    {
        return 0;            
    }
    else
    {
        if (end_addr < FMC_BANK1_SART_ADDRESS)
        {                
            /* get page number */
            page_num = (size + FLASH_BANK0_PAGE_SIZE - 1) / FLASH_BANK0_PAGE_SIZE;

            /* get start address of page */
            page_start_addr = get_page(start_addr);

            /* unlock the flash program/erase controller */
            fmc_unlock();
 
            /* clear all pending flags */
            fmc_flag_clear(FMC_FLAG_BANK0_END);
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    
            /* erase the flash pages */
            for(erase_cnt = 0; erase_cnt < page_num; erase_cnt++)
            {
                fmc_page_erase(page_start_addr + (FLASH_BANK0_PAGE_SIZE * erase_cnt));
                fmc_flag_clear(FMC_FLAG_BANK0_END);
                fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
                fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
            }

            /* lock the main FMC after the erase operation */
            fmc_lock();    
        }
        else if (start_addr >= FMC_BANK1_SART_ADDRESS)
        {
            /* get page number */
            page_num = (size + FLASH_BANK1_PAGE_SIZE - 1) / FLASH_BANK1_PAGE_SIZE;
    
            /* get start address of page */
            page_start_addr = get_page(start_addr);
  
            /* unlock the flash program/erase controller */
            fmc_unlock();
 
            /* clear all pending flags */
            fmc_flag_clear(FMC_FLAG_BANK1_END);
            fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK1_PGERR);
    
            /* erase the flash pages */
            for(erase_cnt = 0; erase_cnt < page_num; erase_cnt++)
            {
                fmc_page_erase(page_start_addr + (FLASH_BANK1_PAGE_SIZE * erase_cnt));
                fmc_flag_clear(FMC_FLAG_BANK1_END);
                fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
                fmc_flag_clear(FMC_FLAG_BANK1_PGERR);
            }

            /* lock the main FMC after the erase operation */
            fmc_lock();            
        }
        else
        {
            /* unlock the flash program/erase controller */
            fmc_unlock();
            
            uint32_t bank0_size = FMC_BANK1_SART_ADDRESS - start_addr;  
            uint32_t bank1_size = end_addr - FMC_BANK1_SART_ADDRESS +1;       
            
             /* get page number of bank0 */
            page_num = (bank0_size + FLASH_BANK0_PAGE_SIZE - 1) / FLASH_BANK0_PAGE_SIZE;
            
            /* get start address of page in bank0 */
            page_start_addr = get_page(start_addr);

            /* clear all pending flags */
            fmc_flag_clear(FMC_FLAG_BANK0_END);
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    
            /* erase the flash pages */
            for(erase_cnt = 0; erase_cnt < page_num; erase_cnt++)
            {
                fmc_page_erase(page_start_addr + (FLASH_BANK0_PAGE_SIZE * erase_cnt));
                fmc_flag_clear(FMC_FLAG_BANK0_END);
                fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
                fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
            } 
  
             /* get page number of bank1 */
            page_num = (bank1_size + FLASH_BANK1_PAGE_SIZE - 1) / FLASH_BANK1_PAGE_SIZE;

            /* get start address of page in bank1 */
            page_start_addr = get_page(FMC_BANK1_SART_ADDRESS);
            
            /* clear all pending flags */
            fmc_flag_clear(FMC_FLAG_BANK1_END);
            fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK1_PGERR);
    
            /* erase the flash pages */
            for(erase_cnt = 0; erase_cnt < page_num; erase_cnt++)
            {
                fmc_page_erase(page_start_addr + (FLASH_BANK1_PAGE_SIZE * erase_cnt));
                fmc_flag_clear(FMC_FLAG_BANK1_END);
                fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
                fmc_flag_clear(FMC_FLAG_BANK1_PGERR);
            }

            /* lock the main FMC after the erase operation */
            fmc_lock();               
        }
    }
    
    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           gd32_flash_write:Write data of flash,and this operation's units is word.
 *
 * @param[in]       addr            flash address.
 * @param[in]       buf             buffer to store write data.
 * @param[in]       size            write bytes size.
 *
 * @return          Return write size or status.
 * @retval          size            write bytes size.
 * @retval          Others          read failed.
 ***********************************************************************************************************************
 */
int gd32_flash_write(os_uint32_t addr, const os_uint8_t *buf, size_t size)
{
    size_t i;
    size_t j;
    os_err_t    result     = 0;
    os_uint32_t write_data = 0;
    os_uint32_t temp_data = 0;
    fmc_state_enum state;
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
        state = fmc_word_program(addr, write_data);
       
        if (addr < FMC_BANK1_SART_ADDRESS)
        {            
            fmc_flag_clear(FMC_FLAG_BANK0_END);
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        }
        else
        {
            fmc_flag_clear(FMC_FLAG_BANK1_END);
            fmc_flag_clear(FMC_FLAG_BANK1_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK1_PGERR);           
        }
        
        if (state == FMC_READY)
        {
            /* Check the written value */
            if (*(uint32_t *)addr != write_data)
            {
                LOG_E(LOG_TAG, "ERROR: write data != read data\n");
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

#include "fal_drv_flash.c"

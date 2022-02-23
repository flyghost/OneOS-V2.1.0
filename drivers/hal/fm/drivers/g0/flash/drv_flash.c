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
 * @file        drv_flash_l1xx.c
 *
 * @brief       The file of flash drv for htl1xx
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_flash.h"

#define DBG_TAG "drv.flash"
#include <dlog.h>

#if defined(OS_USING_FAL)
#include "fal.h"
#endif

/* Flash operation verification code (for reliability, specific data can be customized by the user,
 * stored in ee or flash, read and assign to OperateKey before operating flash) 
 */
#define FLASHOPKEY  0x12ABF00F

os_uint8_t HAL_Flash_Erase_Sector( os_uint16_t SectorNum, os_uint32_t OperateKey)
{
    os_uint16_t  i;
    os_uint8_t   Result = 0;
    os_uint32_t *PFlash;

    PFlash = (os_uint32_t *)(os_uint32_t)(SectorNum*512);
    if( OperateKey == FLASHOPKEY )
    {
        DMA_GLOBALCTRL_DMAEN_Setable(DISABLE);
        RCC_PERCLK_SetableEx(FLSEPCLK, ENABLE);
    }
    FLASH_Erase_Sector( SectorNum*512 );
    DMA_GLOBALCTRL_DMAEN_Setable(ENABLE);
    RCC_PERCLK_SetableEx(FLSEPCLK, DISABLE);
    
    for( i=0;i<128;i++ )
    {
        if( PFlash[i] != 0xFFFFFFFF ) 
        {
            Result = 1;
            break;
        }
    }
    
    return Result;
}

os_uint8_t HAL_Flsah_Write_String( os_uint32_t prog_addr, os_uint8_t* prog_data, os_uint16_t Len, os_uint32_t OperateKey )
{
    os_uint16_t      i;
    os_uint8_t       Result = 0;
    os_uint8_t      *PFlash;
    
    if( OperateKey == FLASHOPKEY )
    {
        DMA_GLOBALCTRL_DMAEN_Setable(DISABLE);
        RCC_PERCLK_SetableEx(FLSEPCLK, ENABLE);
    }
    FLASH_Prog_ByteString( prog_addr, prog_data, Len);
     DMA_GLOBALCTRL_DMAEN_Setable(ENABLE);
    RCC_PERCLK_SetableEx(FLSEPCLK, DISABLE);


    PFlash = (uint8_t*)prog_addr;
    for( i=0;i<Len;i++ )
    {
        if( PFlash[i] != prog_data[i] ) 
        {
            Result = 1;
            break;
        }
    }   
    
    return Result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
int fm33_flash_read(os_uint32_t addr, os_uint8_t *buf, os_size_t size)
{
    os_size_t i;

    if ((addr + size) > FM_FLASH_END_ADDRESS)
    {
        os_kprintf("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return OS_EINVAL;
    }

    for(i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(os_uint8_t *) addr;
    }

    return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */

int fm33_flash_write(os_uint32_t addr, const uint8_t *buf, os_size_t size)
{
    os_err_t    result     = OS_EOK;

    if ((addr + size) > FM_FLASH_END_ADDRESS)
    {
        os_kprintf("ERROR: write outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (size < 1)
    {
        return OS_ERROR;
    }

    /* write data */
    LOG_D(DBG_TAG, "DMA is shutdown during flash program.");
    result = HAL_Flsah_Write_String(addr, (uint8_t *)buf, size, FLASHOPKEY);

    if (result != OS_EOK)
    {
        return result;
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int fm33_flash_erase(os_uint32_t addr, os_size_t size)
{
    os_uint16_t page_start = 0;
    os_uint16_t page_end   = 0;
    os_uint16_t page_cnt   = 0;

    if ((addr + size) > FM_FLASH_END_ADDRESS)
    {
        os_kprintf("ERROR: erase outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return OS_EINVAL;
    }

    page_start = addr / FM_FLASH_BLOCK_SIZE;
    page_end = (addr + size) / FM_FLASH_BLOCK_SIZE + (((addr + size) % FM_FLASH_BLOCK_SIZE) ? 1 : 0);
    
    LOG_D(DBG_TAG, "DMA is shutdown during flash erase.");
    for (page_cnt = 0; page_cnt < (page_end - page_start); page_cnt++)
    {
        HAL_Flash_Erase_Sector(page_start + page_cnt, FLASHOPKEY);
    }

    return size;
}

#if defined(OS_USING_FAL)

#include "fal_drv_flash.c"

#endif

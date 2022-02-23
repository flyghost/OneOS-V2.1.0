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
 * @file        drv_flash_f7.c
 *
 * @brief       This file provides flash read/write/erase functions for f7.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_flash.h"

#define DBG_TAG "drv.flash"
#include <drv_log.h>

#if defined(STM32F722xx) || defined(STM32F723xx)|| defined(STM32F732xx) || defined(STM32F733xx)
#define ADDR_FLASH_SECTOR_0 ((os_uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1 ((os_uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2 ((os_uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3 ((os_uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4 ((os_uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5 ((os_uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6 ((os_uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7 ((os_uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */

#elif  defined(STM32F730xx)
#define ADDR_FLASH_SECTOR_0 ((os_uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1 ((os_uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2 ((os_uint32_t)0x08008000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3 ((os_uint32_t)0x0800C000) /* Base @ of Sector 1, 16 Kbytes */

#elif  defined(STM32F750xx)
#define ADDR_FLASH_SECTOR_0  ((os_uint32_t)0x08000000) /* Base address of Sector 0, 32 Kbytes */
#define ADDR_FLASH_SECTOR_1  ((os_uint32_t)0x08008000) /* Base address of Sector 1, 32 Kbytes */

#elif  defined(STM32F756xx)||defined(STM32F74xxx)//F756xx and F74xxx
#define ADDR_FLASH_SECTOR_0  ((os_uint32_t)0x08000000) /* Base address of Sector 0, 32 Kbytes */
#define ADDR_FLASH_SECTOR_1  ((os_uint32_t)0x08008000) /* Base address of Sector 1, 32 Kbytes */
#define ADDR_FLASH_SECTOR_2  ((os_uint32_t)0x08010000) /* Base address of Sector 2, 32 Kbytes */
#define ADDR_FLASH_SECTOR_3  ((os_uint32_t)0x08018000) /* Base address of Sector 3, 32 Kbytes */
#define ADDR_FLASH_SECTOR_4  ((os_uint32_t)0x08020000) /* Base address of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5  ((os_uint32_t)0x08040000) /* Base address of Sector 5, 256 Kbytes */
#define ADDR_FLASH_SECTOR_6  ((os_uint32_t)0x08080000) /* Base address of Sector 6, 256 Kbytes */
#define ADDR_FLASH_SECTOR_7  ((os_uint32_t)0x080C0000) /* Base address of Sector 7, 256 Kbytes */

#else//STM32F76xxx/STM32F77xxx devices
#define ADDR_FLASH_SECTOR_0  ((os_uint32_t)0x08000000) /* Base address of Sector 0, 32 Kbytes */
#define ADDR_FLASH_SECTOR_1  ((os_uint32_t)0x08008000) /* Base address of Sector 1, 32 Kbytes */
#define ADDR_FLASH_SECTOR_2  ((os_uint32_t)0x08010000) /* Base address of Sector 2, 32 Kbytes */
#define ADDR_FLASH_SECTOR_3  ((os_uint32_t)0x08018000) /* Base address of Sector 3, 32 Kbytes */
#define ADDR_FLASH_SECTOR_4  ((os_uint32_t)0x08020000) /* Base address of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5  ((os_uint32_t)0x08040000) /* Base address of Sector 5, 256 Kbytes */
#define ADDR_FLASH_SECTOR_6  ((os_uint32_t)0x08080000) /* Base address of Sector 6, 256 Kbytes */
#define ADDR_FLASH_SECTOR_7  ((os_uint32_t)0x080C0000) /* Base address of Sector 7, 256 Kbytes */
#define ADDR_FLASH_SECTOR_8  ((os_uint32_t)0x08100000) /* Base address of Sector 8, 256 Kbytes */
#define ADDR_FLASH_SECTOR_9  ((os_uint32_t)0x08140000) /* Base address of Sector 9, 256 Kbytes */
#define ADDR_FLASH_SECTOR_10 ((os_uint32_t)0x08180000) /* Base address of Sector 10, 256 Kbytes */
#define ADDR_FLASH_SECTOR_11 ((os_uint32_t)0x081C0000) /* Base address of Sector 11, 256 Kbytes */
#endif

/**
 ***********************************************************************************************************************
 * @brief           Gets the sector of a given address.
 *
 * @param[in]       Address         a given flash address.
 *
 * @return          Return the sector of a given address.
 ***********************************************************************************************************************
 */
static os_uint32_t GetSector(os_uint32_t Address)
{
    uint32_t sector = 0;

#if defined(FLASH_OPTCR_nDBANK)
    FLASH_OBProgramInitTypeDef OBInit;
    uint32_t                   nbank = 0;

    // get duel bank ability:nDBANK(Bit29)
    HAL_FLASHEx_OBGetConfig(&OBInit);
    nbank = ((OBInit.USERConfig & 0x20000000U) >> 29);
    // 1:single bank mode
    if (1 == nbank)
    {
        if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
        {
            sector = FLASH_SECTOR_0;
        }
        else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
        {
            sector = FLASH_SECTOR_1;
        }
        else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
        {
            sector = FLASH_SECTOR_2;
        }
        else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
        {
            sector = FLASH_SECTOR_3;
        }
        else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
        {
            sector = FLASH_SECTOR_4;
        }
        else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
        {
            sector = FLASH_SECTOR_5;
        }
        else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
        {
            sector = FLASH_SECTOR_6;
        }
        else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
        {
            sector = FLASH_SECTOR_7;
        }
        else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
        {
            sector = FLASH_SECTOR_8;
        }
        else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
        {
            sector = FLASH_SECTOR_9;
        }
        else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
        {
            sector = FLASH_SECTOR_10;
        }
        else
        {
            sector = FLASH_SECTOR_11;
        }
    }
    else /* 0:dual bank mode */
    {
        LOG_E(DBG_TAG,"Not support dual bank mode yet!");
        OS_ASSERT(0);
    }
#else /* no dual bank ability */
    if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_SECTOR_0;
    }
#if (FLASH_SECTOR_TOTAL == 2)
    else if ((Address < STM32_FLASH_END_ADDRESS) && (Address >= ADDR_FLASH_SECTOR_1))
#else
    else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
#endif
    {
        sector = FLASH_SECTOR_1;
    }
#if defined(FLASH_SECTOR_2)
    else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_SECTOR_2;
    }
#endif
#if defined(FLASH_SECTOR_3)
#if (FLASH_SECTOR_TOTAL == 4)
    else if ((Address < STM32_FLASH_END_ADDRESS) && (Address >= ADDR_FLASH_SECTOR_3))
#else
    else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
#endif
    {
        sector = FLASH_SECTOR_3;
    }
#endif


#if defined(FLASH_SECTOR_4) 
    else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_SECTOR_4;
    }
#endif
#if defined(FLASH_SECTOR_5)
    else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_SECTOR_5;
    }
#endif
#if defined(FLASH_SECTOR_6)
    else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_SECTOR_6;
    }
#endif
#if defined(FLASH_SECTOR_7)
    else if ((Address < STM32_FLASH_END_ADDRESS) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_SECTOR_7;
    }
#endif

#endif
    return sector;
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
int stm32_flash_read(os_uint32_t addr, os_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return -1;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(os_uint8_t *)addr;
    }

    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           Write data to flash.This operation's units is word,and operation must after erase.
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
int stm32_flash_write(os_uint32_t addr, const os_uint8_t *buf, size_t size)
{
    os_err_t    result   = OS_EOK;
    os_uint32_t end_addr = addr + size;
    os_base_t   level;
    HAL_StatusTypeDef status;
    
    if ((end_addr) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"write outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (size < 1)
    {
        return OS_EINVAL;
    }

    /* Unlock the Flash to enable the flash control register access */
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGPERR | FLASH_FLAG_ERSERR);

    for (size_t i = 0; i < size; i++, addr++, buf++)
    {
        level = os_irq_lock();
        
        /* write data to flash */
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, (os_uint64_t)(*buf));
        
        os_irq_unlock(level);
        
        if (status == HAL_OK)
        {
            if (*(os_uint8_t *)addr != *buf)
            {
                result = OS_ERROR;
                break;
            }
        }
        else
        {
            result = OS_ERROR;
            break;
        }
    }

    HAL_FLASH_Lock();

    if (result != OS_EOK)
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
    os_err_t    result      = OS_EOK;
    os_uint32_t FirstSector = 0, NbOfSectors = 0;
    uint32_t    SECTORError = 0;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: erase outrange flash size! addr is (0x%p)\r\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;

    /* Unlock the Flash to enable the flash control register access */
    HAL_FLASH_Unlock();

    /* Get the 1st sector to erase */
    FirstSector = GetSector(addr);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = GetSector(addr + size - 1) - FirstSector + 1;
    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector       = FirstSector;
    EraseInitStruct.NbSectors    = NbOfSectors;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
    {
        result = OS_ERROR;
        goto __exit;
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

#if defined(STM32F722xx) || defined(STM32F732xx) || defined(STM32F733xx) || defined(STM32F730xx)
#define STM32_FLASH_BLOCK_SIZE (128 * 1024)
#else
#define STM32_FLASH_BLOCK_SIZE (256 * 1024)
#endif

#define STM32_FLASH_PAGE_SIZE   (8)

#include "fal_drv_flash.c"

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
 * @brief       This file provides flash init/read/write/erase functions for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "hc_flash.h"
#ifdef BSP_USING_ON_CHIP_FLASH
#include "drv_flash.h"

/**
 * Use Flash_WriteByte / Flash_SectorErase function from bootloader address,
 * because if the address of the two functions is larger than 32K, they will can't use
 */
#if BSP_TEXT_SECTION_ADDR > 0x00000000
typedef en_result_t(*PTR_WRITE)(uint32_t, uint8_t);
typedef en_result_t(*PTR_ERASE)(uint32_t);

PTR_WRITE pfun_write = (PTR_WRITE)0x1001; /* Bootloader's Flash_WriteByte function address is 0x1000 */
PTR_ERASE pfun_erase = (PTR_ERASE)0x2001; /* Bootloader's Flash_SectorErase function address is 0x2000 */
#endif

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
int hc32_flash_read(os_uint32_t addr, os_uint8_t *buf, os_size_t size)
{
    size_t i;

    if ((addr + size) > HC32_FLASH_END_ADDRESS)
    {
        printf("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
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

int hc32_flash_write(os_uint32_t addr, const uint8_t *buf, os_size_t size)
{
    size_t i;
    uint8_t *p = (uint8_t *)buf;

    if ((addr + size) > HC32_FLASH_END_ADDRESS)
    {
        printf("ERROR: write outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return OS_EINVAL;
    }

    for(i = 0; i < size; i++)
    {
#if BSP_TEXT_SECTION_ADDR > 0x00000000
        pfun_write(addr + i, p[i]);
#else
        Flash_WriteByte(addr + i, p[i]);
#endif
    }

    for(i = 0; i < size; i++)
    {
        if(p[i] != *((uint8_t *)(addr + i)))
        {
            return 0;
        }
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
int hc32_flash_erase(os_uint32_t addr, os_size_t size)
{
    os_uint16_t sector_start = 0;
    os_uint16_t sector_end = 0;
    os_uint16_t sector_cnt = 0;

    if ((addr + size) > HC32_FLASH_END_ADDRESS)
    {
        printf("ERROR: erase outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return OS_EINVAL;
    }

    addr = addr - addr % HC32_SECTOR_SIZE;

    sector_start = addr / HC32_SECTOR_SIZE;
    sector_end = (addr + size) / HC32_SECTOR_SIZE + (((addr + size) % HC32_SECTOR_SIZE) ? 1 : 0);

    for (sector_cnt = 0; sector_cnt < (sector_end - sector_start); sector_cnt++)
    {
#if BSP_TEXT_SECTION_ADDR > 0x00000000
        pfun_erase((sector_start + sector_cnt) * HC32_SECTOR_SIZE);
#else
        Flash_SectorErase((sector_start + sector_cnt) * HC32_SECTOR_SIZE);
#endif
    }

    return size;
}

#if defined(OS_USING_FAL)

static int hc32_flash_init(fal_flash_t *flash)
{
    os_uint32_t hclk_freq = 0;
    os_uint8_t freq_cfg = 0;

    hclk_freq = Sysctrl_GetHClkFreq();

    if (hclk_freq < 8000000)
    {
        freq_cfg = 1;
    }
    else if ((hclk_freq >= 8000000) && (hclk_freq < 16000000))
    {
        freq_cfg = 2;
    }
    else if ((hclk_freq >= 16000000) && (hclk_freq < 24000000))
    {
        freq_cfg = 4;
    }
    else if ((hclk_freq >= 24000000) && (hclk_freq < 32000000))
    {
        freq_cfg = 6;
    }
    else if ((hclk_freq >= 32000000) && (hclk_freq < 48000000))
    {
        freq_cfg = 8;
    }
    else
    {
        freq_cfg = 12;
    }

    while(Ok != Flash_Init(freq_cfg, TRUE))
    {
        ;
    }

    return OS_EOK;
}

#include "fal_drv_flash.c"

#endif
#endif /* BSP_USING_ON_CHIP_FLASH */

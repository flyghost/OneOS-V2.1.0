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
 * 2021-06-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifdef BSP_USING_ON_CHIP_FLASH

#include "hc32f4a0_clk.h"
#include "hc32f4a0_efm.h"
#include "drv_flash.h"

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
    size_t i, j;
    uint8_t *p = (uint8_t *)buf;
    uint32_t writedata = 0;
    uint8_t tempdata;

    if ((addr + size) > HC32_FLASH_END_ADDRESS)
    {
        printf("ERROR: write outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return OS_EINVAL;
    }

    if ( (addr % 4) != 0)
    {
        printf("write addr must be 4-byte alignment\n");
        return OS_EINVAL;
    }

    if(size < 1)
    {
        return OS_EINVAL;
    }

    EFM_Unlock();
    for(i = 0; i < size;)
    {
        if((size - i) < 4)
        {
            for(j = 0; (size - i) > 0; j++, i++)
            {
                tempdata = *p;
                writedata |= tempdata << (8 * j);
                p++;
            }
        }
        else
        {
            for(j = 0; j < 4; j++, i++)
            {
                tempdata = *p;
                writedata |= tempdata << (8 * j);
                p++;
            }
        }
        if(EFM_SingleProgram(addr, writedata) == Ok)
        {
            if(*(uint32_t *)addr != writedata)
            {
                EFM_Lock();
                printf("write error\n");
                return OS_ERROR;
            }
        }
        else
        {
            EFM_Lock();
            printf("write error\n");
            return OS_ERROR;
        }
        tempdata = 0;
        writedata = 0;
        addr += 4;
    }

    EFM_Lock();
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

    EFM_Unlock();
    for (sector_cnt = 0; sector_cnt < (sector_end - sector_start); sector_cnt++)
    {
        EFM_SectorErase((sector_start + sector_cnt) * HC32_SECTOR_SIZE);
    }
    EFM_Lock();

    return size;
}

#if defined(OS_USING_FAL)

static int hc32_flash_init(fal_flash_t *flash)
{
    os_uint32_t sysclk_freq = 0;
    os_uint32_t latency = 0;
    stc_clk_freq_t stc_clk_freq;

    CLK_GetClockFreq(&stc_clk_freq);
    sysclk_freq = stc_clk_freq.sysclkFreq;

    if (sysclk_freq <= 33000000)
    {
        latency = EFM_WAIT_CYCLE_0;
    }
    else if ((sysclk_freq > 33000000) && (sysclk_freq <= 66000000))
    {
        latency = EFM_WAIT_CYCLE_1;
    }
    else if ((sysclk_freq > 66000000) && (sysclk_freq <= 99000000))
    {
        latency = EFM_WAIT_CYCLE_2;
    }
    else if ((sysclk_freq > 99000000) && (sysclk_freq <= 132000000))
    {
        latency = EFM_WAIT_CYCLE_3;
    }
    else if ((sysclk_freq > 132000000) && (sysclk_freq <= 168000000))
    {
        latency = EFM_WAIT_CYCLE_4;
    }
    else if ((sysclk_freq > 168000000) && (sysclk_freq <= 200000000))
    {
        latency = EFM_WAIT_CYCLE_5;
    }

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetWaitCycle(latency);
    EFM_Lock();

    EFM_Cmd(Enable);

    /* Wait flash ready. */
    while(Set != EFM_GetFlagStatus(EFM_FSR_RDY0))
    {
        ;
    }

    return OS_EOK;
}

#include "fal_drv_flash.c"

#endif
#endif /* BSP_USING_ON_CHIP_FLASH */

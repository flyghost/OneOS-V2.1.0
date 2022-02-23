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

#if defined(OS_USING_FAL)
#include "fal.h"
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
int fm33_flash_read(os_uint32_t addr, os_uint8_t *buf, os_size_t size)
{
    size_t i;

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
int fm33_flash_write(os_uint32_t addr, const os_uint8_t *buf, os_size_t size)
{
    size_t      i, j;
    os_err_t    result     = 0;
    os_uint32_t write_data = 0, temp_data = 0;

    if ((addr + size) > FM_FLASH_END_ADDRESS)
    {
        os_kprintf("ERROR: write outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return OS_EINVAL;
    }

    if (addr % 4 != 0)
    {
        os_kprintf("write addr must be 4-byte alignment");
        return OS_EINVAL;
    }

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
        if (FL_FLASH_Program_Word(FLASH, addr, write_data) == FL_PASS)
        {
            /* Check the written value */
            if (*(os_uint32_t *)addr != write_data)
            {
                os_kprintf("ERROR: write data != read data\n");
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
    if (result != 0)
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

    page_start = addr / FM_FLASH_PAGE_SIZE;
    page_end = (addr + size) / FM_FLASH_PAGE_SIZE + (((addr + size) % FM_FLASH_PAGE_SIZE) ? 1 : 0);

    for (page_cnt = 0; page_cnt < (page_end - page_start); page_cnt++)
    {
        FL_FLASH_PageErase(FLASH, (page_start + page_cnt) * FM_FLASH_PAGE_SIZE);
    }

    return size;
}

#if defined(OS_USING_FAL)

#include "fal_drv_flash.c"

#endif

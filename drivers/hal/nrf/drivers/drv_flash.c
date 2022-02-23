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
 * @brief       This file provides operation functions declaration for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <nrfx.h>
#include "nrfx_nvmc.h"
#include <os_memory.h>
#include <string.h>
#include "ports/flash_info.c"

#ifdef BSP_USING_FLASH

#if defined(OS_USING_FAL)
#include "fal.h"
#endif

#define DBG_TAG "drv.flash"
#include <drv_log.h>




/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
    uint32_t page = 0;
    uint32_t page_size = nrfx_nvmc_flash_page_size_get();
    if (Addr < (NRF52_FLASH_START_ADDRESS + NRF52_FLASH_SIZE))
    {
        page = (Addr - NRF52_FLASH_START_ADDRESS) / page_size;
    }
    else
    {
        return 0xffffffff;
    }
    return page;
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
int nrf52_flash_read(uint32_t addr, uint8_t *buf, size_t size)
{

    size_t i;

    if ((addr + size) > NRF52_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"read outrange flash size! addr is (0x%p)", (void *)(addr + size));
        return -OS_EINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
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
int nrf52_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    if ((addr + size) > NRF52_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: write outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return -OS_EINVAL;
    }


    if (addr % 4 != 0)
    {
        LOG_E(DBG_TAG,"write addr should be 4-byte alignment");
        //4byte write
        //else byts
        return -OS_EINVAL;
    }

    if (size < 1)
    {
        return -OS_ERROR;
    }
    
    if (size % 4 != 0)
    {
        nrfx_nvmc_bytes_write(addr, buf, size);
        return size;
    }
    else
    {
        nrfx_nvmc_words_write(addr, buf, size / 4);
        return size;
    }

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
int nrf52_flash_erase(uint32_t addr, size_t size)
{
    nrfx_err_t result = OS_EOK;

    uint32_t FirstPage = 0, LastPages = 0;
    
    uint32_t page_size = nrfx_nvmc_flash_page_size_get();

    if ((addr + size) > NRF52_FLASH_END_ADDRESS)
    {
        LOG_E(DBG_TAG,"ERROR: erase outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
        return -OS_EINVAL;
    }

    FirstPage = GetPage(addr);
    LastPages = GetPage(addr + size - 1) - FirstPage + 1;

    for (int i = 0; i < LastPages ; i++)
    {
        result = nrfx_nvmc_page_erase((FirstPage + i) * page_size);
        if (NRFX_SUCCESS != result)
        {
            LOG_E(DBG_TAG,"ERROR: erase flash page %d ! error code  is (%x)\n", FirstPage + i, result);
            return -OS_EINVAL;
        }
    }
    LOG_D(DBG_TAG,"erase done: addr (0x%p), size %d", (void *)addr, LastPages * page_size);
    return size;
}

#if defined(OS_USING_FAL)

static int fal_flash_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info = flash->priv;
    
    int count = nrf52_flash_read(flash_info->start_addr + page_addr * flash->page_size, buff, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

static int fal_flash_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info = flash->priv;
    
    int count = nrf52_flash_write(flash_info->start_addr + page_addr * flash->page_size, buff, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

static int fal_flash_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info = flash->priv;
    
    int count =  nrf52_flash_erase(flash_info->start_addr + page_addr * flash->page_size, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

static int nrf52_flash_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal flash mem leak %s.\r\n", dev->name);
        return -1;
    }
    
    struct onchip_flash_info *flash_info = (struct onchip_flash_info *)dev->info;

    memcpy(fal_flash->name, dev->name, min(FAL_DEV_NAME_MAX - 1, strlen(dev->name)));
    
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(dev->name))] = 0;
    
    fal_flash->capacity   = flash_info->capacity;
    fal_flash->block_size = flash_info->block_size;
    fal_flash->page_size  = flash_info->page_size;
    
    fal_flash->ops.read_page   = fal_flash_read,
    fal_flash->ops.write_page  = fal_flash_write,
    fal_flash->ops.erase_block = fal_flash_erase,

    fal_flash->priv = flash_info;

    return fal_flash_register(fal_flash);
}

OS_DRIVER_INFO nrf52_flash_driver = 
{
    .name   = "Onchip_Flash_Type",
    .probe  = nrf52_flash_probe,
};

OS_DRIVER_DEFINE(nrf52_flash_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);


#endif
#endif /* BSP_USING_ON_CHIP_FLASH */

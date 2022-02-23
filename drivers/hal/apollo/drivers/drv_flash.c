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
 * @brief       This file implements flash driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include "am_mcu_apollo.h"
#include "flash.h"

static os_err_t os_flash_init(os_device_t dev)
{
    return OS_EOK;
}

static os_err_t os_flash_open(os_device_t dev, os_uint16_t oflag)
{
    return OS_EOK;
}

static os_err_t os_flash_close(os_device_t dev)
{
    return OS_EOK;
}

static os_err_t os_flash_control(os_device_t dev, int cmd, void *args)
{
    uint32_t ui32Critical, i;
    uint32_t ui32CurrentPage, ui32CurrentBlock;

    if (cmd == OS_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct os_device_blk_geometry *geometry;

        geometry = (struct os_device_blk_geometry *)args;
        if (geometry == OS_NULL)
            return OS_ERROR;

        geometry->bytes_per_sector = 8192;
        geometry->sector_count     = 8192;
        geometry->block_size       = 8192;
    }

    else if (cmd == OS_DEVICE_CTRL_BLK_ERASE)
    {
        struct rom_control_erase *erase;
        erase = (struct rom_control_erase *)args;

        /* Start a critical section. */
        ui32Critical = am_hal_interrupt_master_disable();

        if (erase->type == 0x01)
        {
            for (i = 0; i < erase->pagenums; i++)
            {
                /* Figure out what page and block we're working on. */
                ui32CurrentPage  = AM_HAL_FLASH_ADDR2PAGE(erase->addrstart);
                ui32CurrentBlock = AM_HAL_FLASH_ADDR2INST(erase->addrstart);

                am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY, ui32CurrentBlock, ui32CurrentPage);
                erase->addrstart += 8192;
            }
        }

        /* Exit the critical section. */
        am_hal_interrupt_master_set(ui32Critical);
    }

    return OS_EOK;
}

static os_size_t os_flash_read(os_device_t dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_memcpy((uint8_t *)buffer, (uint8_t *)pos, size);

    return size;
}

static os_size_t os_flash_write(os_device_t dev, os_off_t pos, const void *buffer, os_size_t size)
{
    uint32_t ui32Critical;
    uint32_t ui32WordsInBuffer;

    ui32WordsInBuffer = (size + 3) / 4;

    /* Start a critical section. */
    ui32Critical = am_hal_interrupt_master_disable();

    /* Program the flash page with the data we just received. */
    am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY, (uint32_t *)buffer, (uint32_t *)pos, ui32WordsInBuffer);

    /* Exit the critical section. */
    am_hal_interrupt_master_set(ui32Critical);

    return size;
}

int os_hw_rom_init(void)
{
    static struct os_device device;

    /* register device */
    device.type    = OS_DEVICE_TYPE_BLOCK;
    device.init    = os_flash_init;
    device.open    = os_flash_open;
    device.close   = os_flash_close;
    device.read    = os_flash_read;
    device.write   = os_flash_write;
    device.control = os_flash_control;

    /* no private */
    device.user_data = OS_NULL;

    /* register the device */
    os_device_register(&device, "rom", OS_DEVICE_FLAG_RDWR);

    return 0;
}
#ifdef OS_USING_COMPONENTS_INIT
INIT_DEVICE_EXPORT(os_hw_rom_init);
#endif

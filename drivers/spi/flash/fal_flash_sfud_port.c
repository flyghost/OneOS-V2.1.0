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
 * @file        fal_flash_sfud_port.c
 *
 * @brief       This file provides functions for fal flash sfud.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <sfud.h>
#include <spi_flash_sfud.h>
#include <fal/fal.h>
#include "driver.h"
#include <os_memory.h>
#include "string.h"

static sfud_flash *sfud_norflash0 = OS_NULL;

static int fal_sfud_init(fal_flash_t *flash)
{
    sfud_flash_t sfud_flash0 = NULL;
    sfud_flash0 = (sfud_flash_t)os_sfud_flash_find(OS_EXTERN_FLASH_BUS_NAME);

    if (NULL == sfud_flash0)
    {
        return -1;
    }

    sfud_norflash0 = sfud_flash0;
    return 0;
}

static int read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    if (sfud_read(sfud_norflash0, page_addr * OS_EXTERN_FLASH_PAGE_SIZE, page_nr * OS_EXTERN_FLASH_PAGE_SIZE, buff) != SFUD_SUCCESS)
    {
        return -1;
    }

    return 0;
}

static int write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    if (sfud_write(sfud_norflash0, page_addr * OS_EXTERN_FLASH_PAGE_SIZE, page_nr * OS_EXTERN_FLASH_PAGE_SIZE, buff) != SFUD_SUCCESS)
    {
        return -1;
    }

    return 0;
}

static int erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    if (sfud_erase(sfud_norflash0, page_addr * OS_EXTERN_FLASH_PAGE_SIZE, page_nr * OS_EXTERN_FLASH_PAGE_SIZE) != SFUD_SUCCESS)
    {
        return -1;
    }

    return 0;
}

int sfud_flash0_probe(void)
{
    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal flash mem leak %s.\r\n", OS_EXTERN_FLASH_NAME);
        return -1;
    }

    memcpy(fal_flash->name,
           OS_EXTERN_FLASH_NAME,
           min(FAL_DEV_NAME_MAX - 1, strlen(OS_EXTERN_FLASH_NAME)));
    
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(OS_EXTERN_FLASH_NAME))] = 0;
    
    fal_flash->capacity = OS_EXTERN_FLASH_SIZE;
    fal_flash->block_size = OS_EXTERN_FLASH_BLOCK_SIZE;
    fal_flash->page_size  = OS_EXTERN_FLASH_PAGE_SIZE;
    
    fal_flash->ops.read_page   = read;
    fal_flash->ops.write_page  = write;
    fal_flash->ops.erase_block = erase;

    return fal_flash_register(fal_flash);
}

static int sfud_flash0_init(void)
{
    fal_sfud_init(NULL);
    
    sfud_flash0_probe();
    
    return 0;
}

OS_DEVICE_INIT(sfud_flash0_init, OS_INIT_SUBLEVEL_MIDDLE);


#include "board.h"
#include <string.h>
#include <fal/fal.h>
#include <os_memory.h>


static char *fal_ram_base;

#define FLASH_NAME "ram_flash"



static int fal_ram_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    memcpy(buff, fal_ram_base + page_addr, page_nr);

    return 0;
}

static int fal_ram_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    memcpy(fal_ram_base + page_addr, buff, page_nr);

    return 0;
}

static int fal_ram_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    memset(fal_ram_base + page_addr, 0xff, page_nr);

    return 0;
}

int fal_ram_probe(void)
{
    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal flash mem leak %s.\r\n", FLASH_NAME);
        return -1;
    }

    memcpy(fal_flash->name,
           FLASH_NAME,
           min(FAL_DEV_NAME_MAX - 1, strlen(FLASH_NAME)));
    
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(FLASH_NAME))] = 0;
    
    fal_flash->capacity = OS_FAL_RAM_SIZE;
    fal_flash->block_size = 1;
    fal_flash->page_size  = 1;
    
    fal_flash->ops.read_page   = fal_ram_read;
    fal_flash->ops.write_page  = fal_ram_write;
    fal_flash->ops.erase_block = fal_ram_erase;

    return fal_flash_register(fal_flash);
}

static int fal_ram_init(void)
{
    fal_ram_base = os_calloc(1, OS_FAL_RAM_SIZE);
    OS_ASSERT(fal_ram_base != OS_NULL);

    memset(fal_ram_base, 0xff, OS_FAL_RAM_SIZE);
    
    fal_ram_probe();
    
    return 0;
}

OS_DEVICE_INIT(fal_ram_init, OS_INIT_SUBLEVEL_HIGH);

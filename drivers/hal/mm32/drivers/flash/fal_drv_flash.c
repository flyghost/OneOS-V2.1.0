#include "string.h"
#include <os_memory.h>
#include <oneos_config.h>
#include "dlog.h"

#ifdef BSP_USING_ONCHIP_FLASH
#include "drv_flash.h"

#if defined(OS_USING_FAL)
#include "fal.h"
#include "ports/flash_info.c"
#endif


static int mm32_fal_flash_read(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = mm32_flash_read(MM32_FLASH_START_ADRESS + page_addr * MM32_FLASH_PAGE_SIZE, buff, page_nr * MM32_FLASH_PAGE_SIZE);

    return (count == page_nr * MM32_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int mm32_fal_flash_write(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    int count = mm32_flash_write(MM32_FLASH_START_ADRESS + page_addr * MM32_FLASH_PAGE_SIZE, buff, page_nr * MM32_FLASH_PAGE_SIZE);

    return (count == page_nr * MM32_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int mm32_fal_flash_erase(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int count =  mm32_flash_erase(MM32_FLASH_START_ADRESS + page_addr * MM32_FLASH_PAGE_SIZE, page_nr * MM32_FLASH_PAGE_SIZE);

    return (count == page_nr * MM32_FLASH_PAGE_SIZE) ? 0 : -1;
}

static int mm32_flash_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
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
    
    fal_flash->ops.read_page   = mm32_fal_flash_read,
    fal_flash->ops.write_page  = mm32_fal_flash_write,
    fal_flash->ops.erase_block = mm32_fal_flash_erase,

    fal_flash->priv = flash_info;

    return fal_flash_register(fal_flash);
}

OS_DRIVER_INFO mm32_flash_driver = 
{
    .name   = "Onchip_Flash_Type",
    .probe  = mm32_flash_probe,
};

OS_DRIVER_DEFINE(mm32_flash_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);
#endif

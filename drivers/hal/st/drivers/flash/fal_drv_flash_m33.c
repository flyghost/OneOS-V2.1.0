#ifdef OS_USING_FAL

#include <fal/fal.h>
#include <os_memory.h>
#include <string.h>
#include "ports/flash_info.c"

static int stm32_flash_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info_p = flash->priv;
    
    int count = stm32_flash_read(flash_info_p->start_addr + page_addr * flash->page_size, buff, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

static int stm32_flash_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info_p = flash->priv;
    
    int count = stm32_flash_write(flash_info_p->start_addr + page_addr * flash->page_size, buff, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

static int stm32_flash_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info_p = flash->priv;
    
    int count =  stm32_flash_erase(flash_info_p->start_addr + page_addr * flash->page_size, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}
int stm32_flash_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal flash mem leak %s.\r\n", dev->name);
        return -1;
    }
    
    struct onchip_flash_info *flash_info = (struct onchip_flash_info *)dev->info;

    memcpy(fal_flash->name,
           dev->name,
           min(FAL_DEV_NAME_MAX - 1, strlen(dev->name)));
    
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(dev->name))] = 0;
    
    fal_flash->capacity = flash_info->capacity;
    fal_flash->block_size = flash_info->block_size;
    fal_flash->page_size  = flash_info->page_size;
    
    fal_flash->ops.read_page   = stm32_flash_read_page,
    fal_flash->ops.write_page  = stm32_flash_write_page,
    fal_flash->ops.erase_block = stm32_flash_erase_block,

    fal_flash->priv = flash_info;

    return fal_flash_register(fal_flash);
}

OS_DRIVER_INFO stm32_flash_driver = 
{
    .name   = "Onchip_Flash_Type",
    .probe  = stm32_flash_probe,
};

OS_DRIVER_DEFINE(stm32_flash_driver,DEVICE,OS_INIT_SUBLEVEL_HIGH);

#endif


#ifdef OS_USING_FAL

#include <fal/fal.h>
#include "ports/flash_info.c"

static int lpc_flash_init(fal_flash_t *flash)
{
    os_uint32_t status;
    
    LOG_I(DRV_EXT_TAG, "lpc55s69 onchip flash need config in 96/12Mhz to use!");
    
    status = FLASH_Init(&lpc_flashConfig);
    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "onchip_flash init failed!");
    }
    LOG_I(DRV_EXT_TAG, "onchip_flash init success!");
    LOG_I(DRV_EXT_TAG, "note: lpc55s69 flash has same limit!");
    LOG_I(DRV_EXT_TAG, "1. cannot direct read page which been erased! use FLASH_VerifyErase to check is erased or not!");
    LOG_I(DRV_EXT_TAG, "2. cannot write page which been written! use FLASH_VerifyErase to check is erased or not!");
    return 0;
}

static int lpc_flash_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info_p = flash->priv;
    int count = lpc_flash_read(flash_info_p->start_addr + page_addr * flash->page_size, buff, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

static int lpc_flash_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info_p = flash->priv;
    int count = lpc_flash_write(flash_info_p->start_addr + page_addr * flash->page_size, buff, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

static int lpc_flash_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    struct onchip_flash_info * flash_info_p = flash->priv;
    
    int count =  lpc_flash_erase(flash_info_p->start_addr + page_addr * flash->page_size, page_nr * flash->page_size);

    return (count == page_nr * flash->page_size) ? 0 : -1;
}

int lpc_flash_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    lpc_flash_init(NULL);
    
    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "fal flash mem leak %s.", dev->name);
        return OS_ERROR;
    }
    
    struct onchip_flash_info *flash_info = (struct onchip_flash_info *)dev->info;

    memcpy(fal_flash->name, dev->name, min(FAL_DEV_NAME_MAX - 1, strlen(dev->name)));
    
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(dev->name))] = 0;
    
    fal_flash->capacity = flash_info->capacity;
    fal_flash->block_size = flash_info->block_size;
    fal_flash->page_size  = flash_info->page_size;
    
    fal_flash->ops.read_page   = lpc_flash_read_page,
    fal_flash->ops.write_page  = lpc_flash_write_page,
    fal_flash->ops.erase_block = lpc_flash_erase_block,

    fal_flash->priv = flash_info;

    return fal_flash_register(fal_flash);
}

OS_DRIVER_INFO lpc_flash_driver = 
{
    .name   = "Onchip_Flash_Type",
    .probe  = lpc_flash_probe,
};

OS_DRIVER_DEFINE(lpc_flash_driver,DEVICE,OS_INIT_SUBLEVEL_HIGH);

#endif


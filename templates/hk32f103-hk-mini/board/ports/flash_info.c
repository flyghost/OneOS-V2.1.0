static struct onchip_flash_info onchip_flash = 
{
    .start_addr = HK32_FLASH_START_ADRESS,
    .capacity   = HK32_FLASH_SIZE,
    .block_size = 2 * 1024,
    .page_size  = 4,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "onchip_flash", onchip_flash);

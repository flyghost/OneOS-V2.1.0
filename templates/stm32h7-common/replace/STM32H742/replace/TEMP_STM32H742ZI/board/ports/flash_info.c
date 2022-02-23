static struct onchip_flash_info onchip_flash = 
{
    .start_addr = STM32_FLASH_START_ADRESS,
    .capacity   = STM32_FLASH_SIZE,
    .block_size = STM32_FLASH_BLOCK_SIZE,
    .page_size  = STM32_FLASH_PAGE_SIZE,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "onchip_flash", onchip_flash);

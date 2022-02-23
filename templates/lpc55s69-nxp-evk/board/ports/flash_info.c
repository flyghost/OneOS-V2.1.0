static struct onchip_flash_info onchip_flash = 
{
    .start_addr = LPC_FLASH_START_ADRESS,
    .capacity   = LPC_FLASH_SIZE,
    .block_size = LPC_FLASH_BLOCK_SIZE,
    .page_size  = LPC_FLASH_PAGE_SIZE,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "onchip_flash", onchip_flash);

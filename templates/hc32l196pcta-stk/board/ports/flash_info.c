static struct onchip_flash_info onchip_flash = 
{
    .start_addr = HC32_FLASH_START_ADRESS,
    .capacity   = HC32_FLASH_SIZE,
    .block_size = HC32_FLASH_BLOCK_SIZE,
    .page_size  = HC32_FLASH_BYTE_ALIGN_SIZE,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "onchip_flash", onchip_flash);

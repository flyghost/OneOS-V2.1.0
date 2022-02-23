static struct onchip_flash_info onchip_flash = 
{
    .start_addr = CM32_FLASH_START_ADRESS,
    .capacity   = CM32_FLASH_SIZE,
    .block_size = CM32_FLASH_BLOCK_SIZE,
    .page_size  = CM32_FLASH_BYTE_ALIGN_SIZE,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "onchip_flash", onchip_flash);

static struct onchip_flash_info onchip_flash = 
{
    .start_addr = FM_FLASH_START_ADRESS,
    .capacity   = FM_FLASH_SIZE,
    .block_size = FM_FLASH_BLOCK_SIZE,
    .page_size  = FM_FLASH_BYTE_ALIGN_SIZE,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "onchip_flash", onchip_flash);

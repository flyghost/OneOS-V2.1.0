static struct onchip_flash_info flash_0_info = 
{
    .start_addr = 0x08008000,
    .capacity   = 16384,
    .block_size = 16384,
    .page_size  = 2048,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "flash_0", flash_0_info);

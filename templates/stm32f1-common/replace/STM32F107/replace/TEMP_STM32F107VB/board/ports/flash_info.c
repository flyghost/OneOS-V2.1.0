static struct onchip_flash_info flash_0_info = 
{
    .start_addr = 0x08005000,
    .capacity   = 2048,
    .block_size = 2048,
    .page_size  = 1024,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "flash_0", flash_0_info);

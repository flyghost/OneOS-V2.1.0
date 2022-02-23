static struct onchip_flash_info flash_0_info = 
{
    .start_addr = 0x08000000,
    .capacity   = 0,
    .block_size = 0,
    .page_size  = 1024,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "flash_0", flash_0_info);


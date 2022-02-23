static struct onchip_flash_info flash_0_info = 
{
    .start_addr = (0x08000000),
    .capacity   = (128 * 1024),
    .block_size = (1024),
    .page_size  = (4),
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "flash_0", flash_0_info);

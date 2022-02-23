static struct onchip_flash_info flash_0_info = 
{
    .start_addr = 0x00000000,
    .capacity   = 0x80000,
    .block_size = 4096,
    .page_size  = 4096,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "onchip_flash", flash_0_info);

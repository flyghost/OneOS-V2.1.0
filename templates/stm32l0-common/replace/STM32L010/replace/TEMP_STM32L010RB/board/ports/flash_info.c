static struct onchip_flash_info flash_0_info = 
{
.start_addr = 0x08005000,
    .capacity = 2 * 1024,
    .block_size = 16 * 128,
    .page_size = 128,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "flash_0", flash_0_info);

//STM32L0x0:one sector contains 32 pages,one page is 128 bytes

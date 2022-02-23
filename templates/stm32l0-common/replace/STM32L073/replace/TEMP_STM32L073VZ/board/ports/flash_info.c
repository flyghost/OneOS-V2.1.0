static struct onchip_flash_info flash_0_info = 
{
.start_addr = 0x08005000,
    .capacity = 2 * 1024,
    .block_size = 16 * 128,
    .page_size = 128,
};
OS_HAL_DEVICE_DEFINE("Onchip_Flash_Type", "flash_0", flash_0_info);

//STM32L0x0:one sector contains 32 pages,one page is 128 bytes
//64K:sector:15;page_num:512;page_size:128;(STM32L0x1,STM32L0x2,STM32L0x3)
//128K:bank:2;sector:32;page_num:1024,page_size:128;(STM32L0x1,STM32L0x2,STM32L0x3)
//192K:bank:2;sector:48;page_num:1536,page_size:128;(STM32L0x1,STM32L0x2,STM32L0x3)
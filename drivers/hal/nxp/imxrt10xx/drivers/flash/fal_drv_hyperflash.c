#ifdef OS_USING_FAL

#include <fal/fal.h>
#include <os_memory.h>
#include <string.h>
#include "ports/flash_info.c"
#include "fsl_cache.h"
static int fal_hyperflash_init(void)
{
    status_t status;
    flexspi_hyper_flash_init();

    status = flexspi_nor_hyperflash_cfi(EXAMPLE_FLEXSPI);
    /* Get vendor ID. */
    if (status != kStatus_Success)
    {
        return status;
    }
    else
    {
        os_kprintf("Found the HyperFlash by CFI\r\n");
    }
    
    return 0;
}



static int hyper_flash_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + page_addr * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE * page_nr);
//    os_kprintf("dchche range:EXAMPLE_FLEXSPI:0x%x,FlexSPI_AMBA_BASE:0x%x\r\n",(EXAMPLE_FLEXSPI_AMBA_BASE + page_addr * FLASH_PAGE_SIZE),(FlexSPI_AMBA_BASE + page_addr*FLASH_PAGE_SIZE));
//#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
//    volatile bool ICacheEnableFlag = false;
//    /* Disable I cache. */
//    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
//    {
//        SCB_DisableICache();
//        ICacheEnableFlag = true;
//    }
//#endif /* __ICACHE_PRESENT */
    memcpy(buff, (void *)(FlexSPI_AMBA_BASE + page_addr*FLASH_PAGE_SIZE),FLASH_PAGE_SIZE * page_nr);
    
//#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
//    if (ICacheEnableFlag)
//    {
//        /* Enable I cache. */
//        SCB_EnableICache();
//        ICacheEnableFlag = false;
//    }
//#endif /* __ICACHE_PRESENT */
    return 0;
}

static int hyper_flash_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    bool ICacheEnableFlag;
    status_t status;
    for (uint32_t k = 0x00U; k < page_nr; k++)
    {
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
        volatile bool ICacheEnableFlag = false;
        /* Disable I cache. */
        if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
        {
            SCB_DisableICache();
            ICacheEnableFlag = true;
        }
#endif /* __ICACHE_PRESENT */

        status = flexspi_nor_flash_page_program(EXAMPLE_FLEXSPI,
                                                page_addr* FLASH_PAGE_SIZE + k * FLASH_PAGE_SIZE,
                                                (void *)buff);
        buff+=FLASH_PAGE_SIZE;
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
        if (ICacheEnableFlag)
        {
            /* Enable I cache. */
            SCB_EnableICache();
            ICacheEnableFlag = false;
        }
#endif /* __ICACHE_PRESENT */

        if (status != kStatus_Success)
        {
            os_kprintf("Page program failure !\r\n");
            return -1;
        }
    }
    return 0;
}

static int hyper_flash_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    struct onchip_flash_info *flash_info = flash->priv;
    status_t status;
    OS_ASSERT(page_nr);
    page_nr -=1;
    for (uint32_t j = 0x00U; j < (((page_nr * FLASH_PAGE_SIZE) / SECTOR_SIZE) +1); j++)
    {
        /* Disable I cache to avoid cache pre-fatch instruction with branch prediction from flash
           and application operate flash synchronously in multi-tasks. */
#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
        volatile bool ICacheEnableFlag = false;
        /* Disable I cache. */
        if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
        {
            SCB_DisableICache();
            ICacheEnableFlag = true;
        }
#endif /* __ICACHE_PRESENT */
        /* Erase sector. */
        status = flexspi_nor_flash_erase_sector(EXAMPLE_FLEXSPI, ((page_addr * FLASH_PAGE_SIZE) / SECTOR_SIZE +j) * SECTOR_SIZE);

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
        if (ICacheEnableFlag)
        {
            /* Enable I cache. */
            SCB_EnableICache();
            ICacheEnableFlag = false;
        }
#endif /* __ICACHE_PRESENT */

        if (status != kStatus_Success)
        {
            os_kprintf("Erase sector failure !\r\n");
            return -1;
        }
    }

    return 0;
}

static int hyper_flash_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    fal_hyperflash_init();

    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal flash mem leak %s.\r\n", dev->name);
        return -1;
    }
    
    struct onchip_flash_info *flash_info = (struct onchip_flash_info *)dev->info;

    memcpy(fal_flash->name, dev->name, min(FAL_DEV_NAME_MAX - 1, strlen(dev->name)));
    
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(dev->name))] = 0;
    
    fal_flash->capacity   = flash_info->capacity;
    fal_flash->block_size = flash_info->block_size;
    fal_flash->page_size  = flash_info->page_size;
    
    fal_flash->ops.read_page   = hyper_flash_read_page,
    fal_flash->ops.write_page  = hyper_flash_write_page,
    fal_flash->ops.erase_block = hyper_flash_erase_block,

    fal_flash->priv = flash_info;

    return fal_flash_register(fal_flash);
}

OS_DRIVER_INFO hyper_flash_driver = 
{
    .name   = "Hyper_Flash_Type",
    .probe  = hyper_flash_probe,
};

OS_DRIVER_DEFINE(hyper_flash_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);

#endif


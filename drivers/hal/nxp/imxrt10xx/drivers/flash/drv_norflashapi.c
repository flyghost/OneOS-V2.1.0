/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_adc.c
 *
 * @brief       This file implements adc driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <board.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>

#include <fal/fal.h>
#include <os_memory.h>
#include <string.h>
#include <fsl_cache.h>
#include "ports/flash_info.c"

#include "fsl_romapi.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.norflash"
#include <drv_log.h>

struct imxrt_rom_norflash
{
    os_uint32_t                 vendorID;
    os_uint32_t                 instance;
    flexspi_nor_config_t        norConfig;
    serial_nor_config_option_t  option;
    
    os_uint32_t                 persector_pagenum;
    os_uint32_t                 flash_addr;
};

static struct imxrt_rom_norflash nor_flash = {0};

static os_err_t _get_vendor_id(void)
{
    os_uint32_t status;
    os_uint32_t lut_seq[4];
    
    memset(lut_seq, 0, sizeof(lut_seq));
    
    // Read manufacturer ID
    lut_seq[0] = FSL_ROM_FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x9F, READ_SDR, FLEXSPI_1PAD, 4);
    ROM_FLEXSPI_NorFlash_UpdateLut(nor_flash.instance, NOR_CMD_LUT_SEQ_IDX_READID, (const uint32_t *)lut_seq, 1U);

    flexspi_xfer_t xfer;
    xfer.operation            = kFLEXSPIOperation_Read;
    xfer.seqId                = NOR_CMD_LUT_SEQ_IDX_READID;
    xfer.seqNum               = 1U;
    xfer.baseAddress          = 0U;
    xfer.isParallelModeEnable = false;
    xfer.rxBuffer             = &nor_flash.vendorID;
    xfer.rxSize               = 1U;

    status = ROM_FLEXSPI_NorFlash_CommandXfer(nor_flash.instance, &xfer);
    if ((nor_flash.vendorID != kSerialFlash_ISSI_ManufacturerID) && (nor_flash.vendorID != kSerialFlash_Adesto_ManufacturerID)
        && (nor_flash.vendorID != kSerialFlash_Winbond_ManufacturerID) && (nor_flash.vendorID != kSerialFlash_Cypress_ManufacturerID))
    {
        return OS_ENOMEM;
    }

    if (status != kStatus_Success)
    {
        return OS_ERROR;
    }
    else
    {
        return OS_EOK;
    }
}

static os_err_t fal_norflash_init(void)
{
    status_t status             = kStatus_Success;

    nor_flash.instance          = 0;
    nor_flash.option.option0.U  = 0xc0000007U;
    nor_flash.option.option1.U  = 0U;

    nor_flash.flash_addr        = FlexSPI_AMBA_BASE;
    
    /* Setup FLEXSPI NOR Configuration Block */
    status = ROM_FLEXSPI_NorFlash_GetConfig(nor_flash.instance, &nor_flash.norConfig, &nor_flash.option);
    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "ROM_FLEXSPI_NorFlash_GetConfig failed!");
        return OS_ERROR;
    }
    
    /* Initializes the FLEXSPI module for the other FLEXSPI APIs */
    status = ROM_FLEXSPI_NorFlash_Init(nor_flash.instance, &nor_flash.norConfig);
    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "ROM_FLEXSPI_NorFlash_Init failed!");
        return OS_ERROR;
    }
    
    ROM_FLEXSPI_NorFlash_ClearCache(nor_flash.instance);

    /*  Probe device presence by verifying Manufacturer ID */
    status = _get_vendor_id();
    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "_get_vendor_id failed!");
        return OS_ERROR;
    }

    nor_flash.persector_pagenum = nor_flash.norConfig.sectorSize / nor_flash.norConfig.pageSize;
    
    return OS_EOK;
}


static int nor_flash_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    os_uint32_t addr = 0;
    os_uint32_t size = 0;

    addr = nor_flash.instance + page_addr * nor_flash.norConfig.pageSize + nor_flash.flash_addr;
    size = page_nr * nor_flash.norConfig.pageSize;
        
    DCACHE_InvalidateByRange(addr, size);
    
    os_irq_disable();
    /* Verify programming by reading back from FLEXSPI memory directly */
    memcpy(buff, (os_uint8_t *)(addr), size);
    os_irq_enable();
    
    return OS_EOK;
}

static int nor_flash_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    status_t status;

    os_uint32_t i    = 0;
    os_uint32_t addr = 0;

    addr = nor_flash.instance + page_addr * nor_flash.norConfig.pageSize;

    for (i = 0; i < page_nr; i++)
    {
        os_irq_disable();
        /* Program user buffer into FLEXSPI NOR flash */
        status = ROM_FLEXSPI_NorFlash_ProgramPage(nor_flash.instance, &nor_flash.norConfig, addr, (const os_uint32_t *)buff);
        os_irq_enable();
        if (status != kStatus_Success)
        {
            LOG_E(DRV_EXT_TAG, "ROM_FLEXSPI_NorFlash_ProgramPage error %d!", status);
            return OS_ERROR;
        }

        addr += nor_flash.norConfig.pageSize;
        buff += nor_flash.norConfig.pageSize;
    }

    return OS_EOK;
}

static int nor_flash_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    status_t status;

    os_uint32_t addr        = 0;
    os_uint32_t size        = 0;
    os_uint32_t sector_num  = 0;;

    if (page_addr % nor_flash.persector_pagenum != 0)
    {
        LOG_E(DRV_EXT_TAG, "page_addr must be Several times of persector_pagenum %d!", nor_flash.persector_pagenum);
        return OS_EINVAL;
    }
    
    if (page_nr % nor_flash.persector_pagenum != 0)
    {
        LOG_E(DRV_EXT_TAG, "page_nr must be Several times of persector_pagenum %d!", nor_flash.persector_pagenum);
        return OS_EINVAL;
    }

    addr = nor_flash.instance + page_addr * nor_flash.norConfig.pageSize;
    size = page_nr * nor_flash.norConfig.pageSize;
    
    os_irq_disable();
    status = ROM_FLEXSPI_NorFlash_Erase(nor_flash.instance, &nor_flash.norConfig, addr, size);
    os_irq_enable();
    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "ROM_FLEXSPI_NorFlash_Erase error %d!", status);
        return OS_ERROR;
    }
    return OS_EOK;
}

static int nor_flash_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    fal_flash_t *fal_flash = os_calloc(1, sizeof(fal_flash_t));
    if (fal_flash == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "fal flash mem leak %s!", dev->name);
        return OS_EEMPTY;
    }

    if (fal_norflash_init() != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "fal_norflash_init failed!");
        os_free(fal_flash);
        return OS_ERROR;
    }
   
    struct onchip_flash_info *flash_info = (struct onchip_flash_info *)dev->info;

    flash_info->start_addr      = nor_flash.flash_addr;
    flash_info->block_size      = nor_flash.norConfig.sectorSize;
    flash_info->capacity        = nor_flash.norConfig.memConfig.sflashA1Size;
    flash_info->page_size       = nor_flash.norConfig.pageSize;
    
    memcpy(fal_flash->name, dev->name, min(FAL_DEV_NAME_MAX - 1, strlen(dev->name)));
    
    fal_flash->name[min(FAL_DEV_NAME_MAX - 1, strlen(dev->name))] = 0;
    
    fal_flash->capacity         = nor_flash.norConfig.memConfig.sflashA1Size;
    fal_flash->block_size       = nor_flash.norConfig.sectorSize;
    fal_flash->page_size        = nor_flash.norConfig.pageSize;
    
    fal_flash->ops.read_page    = nor_flash_read_page,
    fal_flash->ops.write_page   = nor_flash_write_page,
    fal_flash->ops.erase_block  = nor_flash_erase_block,
    fal_flash->priv = flash_info;
    
    fal_flash_register(fal_flash);
    
    return OS_EOK; 
}

OS_DRIVER_INFO nor_flash_driver = 
{
    .name   = "Nor_Flash_Type",
    .probe  = nor_flash_probe,
};

OS_DRIVER_DEFINE(nor_flash_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);




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
 * @file        fal_cfg.h
 *
 * @brief       Flash abstract layer partition definition
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <board.h>

/* bootloader partition name */
#define OS_BL_PART_NAME "bootloader"
#define OS_CFG_NAME     "cfg"
#define OS_APP_NAME     "app"
#define OS_DL_PART_NAME "download"
#define OS_DF_PART_NAME "diff_patch"
#define OS_BK_PART_NAME "backup"     /* backup */
#define OS_EF_PART_NAME "easyflash"  /* easyflash */
#define OS_RF_PART_NAME "wifi_image" /* wifi_image */
#define OS_FF_PART_NAME "font"       /* font */
#define OS_FS_PART_NAME "filesystem" /* filesystem */

#define OS_EEPROM_NAME     "eeprom"
#define OS_EEPROM_FLASH_NAME "at24cxx_eeprom"

/* fs: filesystem */
#define OS_EEPROM_PART_ADDR     (0)
#define OS_EEPROM_PART_SIZE     (256)

/* Flash device Configuration */
#define OS_ONCHIP_FLASH_NAME "onchip_flash"
#define OS_EXTERN_FLASH_NAME "nor_flash"

#define MCU_SRAM_BEGIN ((uint32_t)0x20000000)
#define MCU_SRAM_SIZE  (256 * 1024)
#define MCU_SRAM_END   (MCU_SRAM_BEGIN + MCU_SRAM_SIZE)

#define MCU_ROM_BEGIN ((uint32_t)0x00000000)
#define MCU_ROM_SIZE  ((1024) * 1024)
#define MCU_ROM_END   (MCU_ROM_BEGIN + MCU_ROM_SIZE)

/* bootloader */
#define MCU_BOOT_ENTRY MCU_ROM_BEGIN
#define MCU_BOOT_SIZE  (128 * 1024)
#define MCU_BOOT_END   (MCU_BOOT_ENTRY + MCU_BOOT_SIZE)

/* cfg */
#define MCU_CFG_ENTRY ((uint32_t)MCU_BOOT_END)
#define MCU_CFG_SIZE  (128 * 1024)
#define MCU_CFG_END   (MCU_CFG_ENTRY + MCU_CFG_SIZE)

/* app */
#define OS_APP_PART_ADDR     ((uint32_t)MCU_CFG_END)
#define OS_APP_PART_SIZE     (256 * 1024)
#define OS_APP_PART_END_ADDR (OS_APP_PART_ADDR + OS_APP_PART_SIZE)

#define USER_APP_ENTRY OS_APP_PART_ADDR

/* download */
#define OS_DL_PART_ADDR     ((uint32_t)OS_APP_PART_END_ADDR)
#define OS_DL_PART_SIZE     (512 * 1024)
#define OS_DL_PART_END_ADDR (OS_DL_PART_ADDR + OS_DL_PART_SIZE)

/* diff_patch */
#define OS_DF_PART_ADDR     ((uint32_t)MCU_SPI_FLASH_BEGIN)
#define OS_DF_PART_SIZE     (1024 * 1024)
#define OS_DF_PART_END_ADDR (OS_DF_PART_ADDR + OS_DF_PART_SIZE)

/* BK: backup */
#define OS_BK_PART_ADDR     ((uint32_t)OS_DF_PART_END_ADDR)
#define OS_BK_PART_SIZE     (1024 * 1024)
#define OS_BK_PART_END_ADDR (OS_BK_PART_ADDR + OS_BK_PART_SIZE)

/* ef: easyflash */
#define OS_EF_PART_ADDR     ((uint32_t)OS_BK_PART_END_ADDR)
#define OS_EF_PART_SIZE     (1024 * 1024)
#define OS_EF_PART_END_ADDR (OS_EF_PART_ADDR + OS_EF_PART_SIZE)

/* wf: wifi image */
#define OS_RF_PART_ADDR     ((uint32_t)OS_EF_PART_END_ADDR)
#define OS_RF_PART_SIZE     (1024 * 1024)
#define OS_RF_PART_END_ADDR (OS_RF_PART_ADDR + OS_RF_PART_SIZE)

/* ff: font */
#define OS_FF_PART_ADDR     ((uint32_t)OS_RF_PART_END_ADDR)
#define OS_FF_PART_SIZE     (1024 * 1024)
#define OS_FF_PART_END_ADDR (OS_FF_PART_ADDR + OS_FF_PART_SIZE)

/* fs: filesystem */
#define OS_FS_PART_ADDR     ((uint32_t)OS_FF_PART_END_ADDR)
#define OS_FS_PART_SIZE     (64 * 1024)
#define OS_FS_PART_END_ADDR (OS_FS_PART_ADDR + OS_FS_PART_SIZE)

/* partition table */
#define FAL_PART_TABLE                                                                                                 \
    {                                                                                                                  \
        {OS_BL_PART_NAME, OS_ONCHIP_FLASH_NAME,        (MCU_BOOT_ENTRY - MCU_ROM_BEGIN),    MCU_BOOT_SIZE, FAL_PART_INFO_FLAGS_LOCK}, \
        {    OS_CFG_NAME, OS_ONCHIP_FLASH_NAME,        (MCU_CFG_ENTRY  - MCU_ROM_BEGIN),     MCU_CFG_SIZE, FAL_PART_INFO_FLAGS_LOCK}, \
        {    OS_APP_NAME, OS_ONCHIP_FLASH_NAME,      (OS_APP_PART_ADDR - MCU_ROM_BEGIN), OS_APP_PART_SIZE, 0}, \
        {OS_DL_PART_NAME, OS_ONCHIP_FLASH_NAME,       (OS_DL_PART_ADDR - MCU_ROM_BEGIN),  OS_DL_PART_SIZE, 0}, \
				{OS_EEPROM_NAME, OS_EEPROM_FLASH_NAME,       OS_EEPROM_PART_ADDR,  OS_EEPROM_PART_SIZE, 0}, \
    }

#endif /* _FAL_CFG_H_ */

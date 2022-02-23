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
 * @file        spi_flash_nm25q.h
 *
 * @brief       this file implements related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SPI_FLASH_NM25Q_H__
#define __SPI_FLASH_NM25Q_H__

#include <board.h>
#include <drv_cfg.h>
#include <drv_spi.h>

#define NM25Q_FAL_NAME    "nor_flash"
#define NM25Q_DEV_NAME    "nm25q"
#define NM25Q_NAME        "nm25q_flash"

#define NM25Q_WriteEnable        0x06
#define NM25Q_WriteDisable       0x04
#define NM25Q_ReadStatusReg1     0x05
#define NM25Q_ReadStatusReg2     0x35
#define NM25Q_ReadStatusReg3     0x15
#define NM25Q_WriteStatusReg1    0x01
#define NM25Q_WriteStatusReg2    0x31
#define NM25Q_WriteStatusReg3    0x11
#define NM25Q_ReadData           0x03
#define NM25Q_FastReadData       0x0B
#define NM25Q_FastReadDual       0x3B
#define NM25Q_PageProgram        0x02
#define NM25Q_BlockErase         0xD8
#define NM25Q_SectorErase        0x20
#define NM25Q_ChipErase          0xC7
#define NM25Q_PowerDown          0xB9
#define NM25Q_ReleasePowerDown   0xAB
#define NM25Q_DeviceID           0xAB
#define NM25Q_ManufactDeviceID   0x90
#define NM25Q_JedecDeviceID      0x9F
#define NM25Q_Enable4ByteAddr    0xB7
#define NM25Q_Exit4ByteAddr      0xE9
#define NM25Q_Dummy_FF           0xFF
#define NM25Q_Dummy_00           0x00
#define NM25Q_MFID               0x52
#define NM25Q_TYPE08             0x13
#define NM25Q_TYPE16             0x14
#define NM25Q_TYPE32             0x15
#define NM25Q_TYPE64             0x16
#define NM25Q_TYPE128            0x17
#define NM25Q_TYPE256            0x18

struct chip_info
{
    os_uint8_t  id;
    os_uint32_t capacity;
    os_uint32_t page_size;
    os_uint32_t erase_size;
    os_uint8_t  addr_flag;
};

struct spi_flash_info
{
    struct os_device              flash_device;
    struct os_spi_device         *spi_device;
    struct chip_info              chip_data;
};

#endif /*__SPI_FLASH_NM25Q_H__*/

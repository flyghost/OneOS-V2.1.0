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
 * @file        fal_cfg.c
 *
 * @brief       Flash abstract layer partition definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#define OS_EE_PART BSP_AT24CXX_I2C_BUS_NAME
#define OS_EE_TYPE "at24cxx_eeprom"

static const fal_part_info_t fal_part_info[] = 
{
    /*       part,          flash,       addr,       size,                       lock */

    {"bootloader", "onchip_flash", 0x00000000, 0x00018000,   FAL_PART_INFO_FLAGS_LOCKED},
    {       "cfg", "onchip_flash", 0x00018000, 0x00008000,   FAL_PART_INFO_FLAGS_LOCKED},
    {       "app", "onchip_flash", 0x00020000, 0x00048000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {  "download", "onchip_flash", 0x00068000, 0x00018000, FAL_PART_INFO_FLAGS_UNLOCKED},

#ifdef BSP_USING_NM25Q
    {"diff_patch", "nor_flash"   , 0x00000000, 0x00100000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {    "backup", "nor_flash"   , 0x00100000, 0x00100000, FAL_PART_INFO_FLAGS_UNLOCKED},
    { "easyflash", "nor_flash"   , 0x00200000, 0x00080000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {"wifi_image", "nor_flash"   , 0x00280000, 0x00080000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {      "font", "nor_flash"   , 0x00300000, 0x00700000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {"filesystem", "nor_flash"   , 0x00a00000, 0x00600000, FAL_PART_INFO_FLAGS_UNLOCKED},
#endif

#ifdef BSP_USING_I2C_AT24CXX
    {OS_EE_PART,  OS_EE_TYPE     , 0x00000000, 0x00000100, FAL_PART_INFO_FLAGS_UNLOCKED},
#endif
};


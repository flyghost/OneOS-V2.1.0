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


static const fal_part_info_t fal_part_info[] = 
{
    /*       part,          flash,       addr,       size,                       lock */
#ifdef OS_USE_BOOTLOADER
    {"bootloader", "onchip_flash", 0x00000000, 0x00020000,   FAL_PART_INFO_FLAGS_LOCKED},
    {       "cfg", "onchip_flash", 0x00020000, 0x00020000,   FAL_PART_INFO_FLAGS_LOCKED},
    {       "app", "onchip_flash", 0x00040000, 0x00040000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {  "download", "onchip_flash", 0x00080000, 0x00080000, FAL_PART_INFO_FLAGS_UNLOCKED},
#else
    {       "app", "onchip_flash", 0x00000000, 0x00100000, FAL_PART_INFO_FLAGS_UNLOCKED},
#endif
#ifdef OS_EXTERN_FLASH_NAME
    {"diff_patch", "nor_flash"   , 0x00000000, 0x00100000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {    "backup", "nor_flash"   , 0x00100000, 0x00100000, FAL_PART_INFO_FLAGS_UNLOCKED},
    { "easyflash", "nor_flash"   , 0x00200000, 0x00080000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {"wifi_image", "nor_flash"   , 0x00280000, 0x00080000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {      "font", "nor_flash"   , 0x00300000, 0x00700000, FAL_PART_INFO_FLAGS_UNLOCKED},
    {"filesystem", "nor_flash"   , 0x00a00000, 0x00600000, FAL_PART_INFO_FLAGS_UNLOCKED},
#endif
};


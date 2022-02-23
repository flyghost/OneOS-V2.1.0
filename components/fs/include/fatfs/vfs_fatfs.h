/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
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
 * @file        vfs_fatfs.h
 *
 * @brief       Header file for FAT filesystem.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-20   OneOS Team      Modify some code to compliant with the posix syntax and optimize some feature.
 ***********************************************************************************************************************
 */

#ifndef __VFS_FATFS_H__
#define __VFS_FATFS_H__

#ifdef __cplusplus
extern "C" {
#endif

int fat_init(void);

#ifdef __cplusplus
}
#endif

#endif

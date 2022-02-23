/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        vfs_yaffs.h
 *
 * @brief       Header file of the yaffs.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First version
 ***********************************************************************************************************************
 */

#ifndef __VFS_YAFFS_H__
#define __VFS_YAFFS_H__

int vfs_yaffs_init(void);

void *yaffs_nand_install(const char *dev_name);

#endif

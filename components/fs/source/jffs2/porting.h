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
 * @file        porting.h
 *
 * @brief       The header file for porting jffs2 file system.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-29   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef _PORTING_H 
#define _PORTING_H

#include "jffs2_config.h"
/* The following should be same with os_sys_stat.h */
//#define JFFS2_S_IFMT  0x000003FF
//#define JFFS2_S_IFDIR (1<<0)
//#define JFFS2_S_IFREG (1<<3)
#define JFFS2_S_IFMT	 S_IFMT
#define JFFS2_S_IFDIR	 S_IFDIR
#define JFFS2_S_IFREG	 S_IFREG

struct jffs2_fs_info
{
    unsigned sector_size; /* A erasing block size*/
    unsigned nr_blocks;   /* Number of blocks in flash */
    unsigned free_size;
};

extern cyg_fileops jffs2_fileops;
extern cyg_fileops jffs2_dirops;
extern struct cyg_fstab_entry jffs2_fste;

time_t jffs2_get_timestamp(void);
void jffs2_get_info_from_sb(void * data, struct jffs2_fs_info * info);
int jffs2_porting_stat(cyg_mtab_entry * mte, cyg_dir dir, const char *name, void * stat_buf);

#endif

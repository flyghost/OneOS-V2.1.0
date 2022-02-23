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
 * @file        porting.c
 *
 * @brief       This file implement functions for porting jffs2 file system.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-29   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <linux/kernel.h>
#include "nodelist.h"

#include "porting.h"

/**
 ***********************************************************************************************************************
 * @brief           This function return time_t, not implemented yet. 
 *
 * @param[]         None
 *
 * @return          Return present time in time_t format.
 ***********************************************************************************************************************
 */
time_t jffs2_get_timestamp(void)
{
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will fetch the partition table on specified buffer.
 *
 * @param[in]       data            The data pointer point to super_block struct.
 * @param[out]      info            return struct jffs2_fs_info info.
 *
 * @return          None
 ***********************************************************************************************************************
 */ 
void jffs2_get_info_from_sb(void * data, struct jffs2_fs_info * info)
{
	struct jffs2_fs_info;
	struct super_block *jffs2_sb;
	struct jffs2_sb_info *c;

	jffs2_sb = (struct super_block *)(data);
	c = JFFS2_SB_INFO(jffs2_sb);
	
	info->sector_size = c->sector_size; 
	info->nr_blocks = c->nr_blocks;
	info->free_size = c->free_size + c->dirty_size;	
}

/**
 ***********************************************************************************************************************
 * @brief           This function get struct jffs2_stat info.
 *
 * @param[in]       mte           The pointer to cyg_mtab_entry.
 * @param[in]       dir           The dir pointer.
 * @param[in]       name          The full path name of file.
 * @param[out]      stat_buf      The pointer to struct jffs2_stat.
 *
 * @return          0 on successful or error code on failed.
 ***********************************************************************************************************************
 */ 
int jffs2_porting_stat(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
		      void * stat_buf)
{
	return jffs2_fste.stat(mte, mte->root, name, (struct jffs2_stat *)stat_buf);	
}

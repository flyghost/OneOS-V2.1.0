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
 * @file        statfs.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SYS_STATFS_H__
#define __SYS_STATFS_H__

struct statfs
{
    unsigned long f_bsize;      /* Block size. */
    unsigned long f_blocks;     /* Total data blocks in file system. */
    unsigned long f_bfree;      /* Free blocks in file system. */
};

extern int statfs (const char *path, struct statfs *buf);

#endif /* __SYS_STATFS_H__ */


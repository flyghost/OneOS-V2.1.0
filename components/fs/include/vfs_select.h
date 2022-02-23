/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        vfs_select.h
 *
 * @brief       Header file for select operation.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_SELECT_H__
#define __VFS_SELECT_H__

#include <oneos_config.h>
#include <sys/time.h>
#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_IO_MULTIPLEXING
extern int vfs_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
#endif

#ifdef __cplusplus
}
#endif

#endif 


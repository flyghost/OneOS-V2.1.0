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
 * @file        fcntl.h
 *
 * @brief       Define some macros and open,fcntl function prototype.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-13   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SYS_FCNTL_H__
#define __SYS_FCNTL_H__

#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2

#define O_APPEND        0x0008
#define O_CREAT         0x0200
#define O_TRUNC         0x0400
#define O_EXCL          0x0800
#define O_SYNC          0x2000
#define O_NONBLOCK      0x4000
#define O_NOCTTY        0x8000
#define O_BINARY        0x10000
#define O_DIRECTORY     0x200000

#define O_ACCMODE       (O_RDONLY|O_WRONLY|O_RDWR)

#define F_DUPFD         0
#define F_GETFD         1
#define F_SETFD         2
#define F_GETFL         3
#define F_SETFL         4
#define F_GETOWN        5
#define F_SETOWN        6
#define F_GETLK         7
#define F_SETLK         8
#define F_SETLKW        9

extern int open(const char *path, int oflag, ...);
extern int fcntl(int fd, int flag, ...);

#endif /* __SYS_FCNTL_H__ */


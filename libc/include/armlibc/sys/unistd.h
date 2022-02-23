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
 * @file        unistd.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-13   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SYS_UNISTD_H__
#define __SYS_UNISTD_H__

#include <sys/types.h>

extern int   chdir(const char *path);
extern char *getcwd(char *buf, size_t size);
extern int   read(int fd, void *buf, size_t nbyte);
extern int   close(int fildes);
extern int   write(int fd, const void *buf, size_t nbyte);
extern int   unlink(const char *path);
extern off_t lseek(int fildes, off_t offset, int whence);
extern int   fsync(int fd);
extern int   access(const char *pathname, int mode);
extern int   rmdir(const char *pathname);

#endif /* __SYS_UNISTD_H__ */


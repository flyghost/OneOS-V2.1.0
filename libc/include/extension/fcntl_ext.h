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
 * @file        libc_fcntl.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __LIBC_FCNTL_H__
#define __LIBC_FCNTL_H__

#include <oneos_config.h>

#if defined(__GNUC__) && defined(OS_USING_NEWLIB_ADAPTER)

#include <fcntl.h>

#ifndef O_NONBLOCK
#define O_NONBLOCK      0x4000
#endif

#ifndef F_GETFL
#define F_GETFL         3
#endif

#ifndef F_SETFL
#define F_SETFL         4
#endif

#ifndef O_DIRECTORY
#define O_DIRECTORY     0x200000
#endif

#ifndef O_BINARY
#ifdef  _O_BINARY
#define O_BINARY        _O_BINARY
#else
#define O_BINARY        0
#endif
#endif /* Not define _BINARY */

#endif /* defined(__GNUC__) && defined(OS_USING_NEWLIB_ADAPTER) */

#endif /* __LIBC_FCNTL_H__ */


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
 * @file        libc_ext.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __OS_LIBC_H__
#define __OS_LIBC_H__

/* Definitions for libc if toolchain has no these definitions. */

#include "extension/errno_ext.h"
#include "extension/fcntl_ext.h"
#include "extension/signal_ext.h"
#include "extension/stdio_ext.h"
#include "extension/string_ext.h"

/* TODO: Temp define Null */
#define EXPORT_SYMBOL(symbol)

#endif /* __OS_LIBC_H__ */


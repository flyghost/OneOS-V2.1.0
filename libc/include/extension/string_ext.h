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
 * @file        string_ext.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __STRING_EXT_H__
#define __STRING_EXT_H__

#include <sys/types.h>
#include <os_stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* These define is define for extending as follow functions under ARMCC or ARMCLANG */
#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern size_t  strnlen(const char *s, size_t maxlen);
extern char   *strdup(const char *s);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STRING_EXT_H__ */


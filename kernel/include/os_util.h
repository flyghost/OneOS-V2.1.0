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
 * @file        os_util.h
 *
 * @brief       This file provides a part of external interface declaration of kernel utility functions. These 
 *              functions are optimized. 
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __OS_UTIL_H__
#define __OS_UTIL_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern os_int32_t os_vsnprintf(char *buf, os_size_t size, const char *fmt, va_list args);
extern os_int32_t os_snprintf(char *buf, os_size_t size, const char *fmt, ...);
extern void       os_kprintf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __OS_UTIL_H__ */


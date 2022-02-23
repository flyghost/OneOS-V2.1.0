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
 * This file are derived from material that is
 * Copyright (c) 2014-2019, PALANDesign Hannover, Germany, MIT License
 * @author      Marco Paland (info@paland.com)
 *
 * @file        at_printf.h
 *
 * @brief       Tiny snprintf implementation, optimized for speed on
 *              embedded systems with a very limited resources.
 *              Use this instead of bloated standard/newlib printf.
 *              These routines are thread safe and reentrant.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-18   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __AT_PRINTF_H__
#define __AT_PRINTF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>

/**
 * Tiny snprintf/vsnprintf implementation
 * \param buffer A pointer to the buffer where to store the formatted string
 * \param count The maximum number of characters to store in the buffer, including a terminating null character
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that COULD have been written into the buffer, not counting the terminating
 *         null character. A value equal or larger than count indicates truncation. Only when the returned value
 *         is non-negative and less than count, the string has been completely written.
 */
int at_snprintf(char *buffer, size_t count, const char *format, ...);
int at_vsnprintf(char *buffer, size_t count, const char *format, va_list va);

#ifdef __cplusplus
}
#endif

#endif /* __AT_PRINTF_H__ */

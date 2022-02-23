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
 * @file        cmiot_hal_uart.h
 *
 * @brief       The hal uart header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_LOG_H__
#define __CMIOT_LOG_H__

#include "cmiot_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LEVEL_ERROR    0
#define LOG_LEVEL_WARNNING 1
#define LOG_LEVEL_INFO     2
#define LOG_LEVEL_DEBUG    3

#define LOG_LEVEL LOG_LEVEL_ERROR

#if (LOG_LEVEL >= LOG_LEVEL_ERROR)
#define CMIOT_ERROR(tag, format, ...)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        cmiot_kprintf("%s" format, tag, ##__VA_ARGS__);                                                                \
    } while (0)
#else
#define CMIOT_ERROR(tag, format, ...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_WARNNING)
#define CMIOT_WARN(tag, format, ...)                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        cmiot_kprintf("%s" format, tag, ##__VA_ARGS__);                                                                \
    } while (0)
#else
#define CMIOT_WARN(tag, format, ...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_INFO)
#define CMIOT_INFO(tag, format, ...)                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        cmiot_kprintf("%s" format, tag, ##__VA_ARGS__);                                                                \
    } while (0)
#else
#define CMIOT_INFO(tag, format, ...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_DEBUG)
#define CMIOT_DEBUG(tag, format, ...)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        cmiot_kprintf("%s" format, tag, ##__VA_ARGS__);                                                                \
    } while (0)
#else
#define CMIOT_DEBUG(tag, format, ...)
#endif

void cmiot_kprintf(const cmiot_char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif

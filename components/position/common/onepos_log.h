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
 * @file        onepos_log.h
 *
 * @brief       log micro definitions
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-21   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#ifndef __ONEPOS_LOG_H__
#define __ONEPOS_LOG_H__

#include <oneos_config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !defined(ONEPOS_LOG_TAG)
#define ONEPOS_LOG_TAG                       "ONEPOS"
#endif

#ifdef OS_USING_DLOG
#include <dlog.h>

/* Debug contorol by dlog config */
#define ONEPOS_LOG_E(fmt, ...)  LOG_E(ONEPOS_LOG_TAG, fmt, ##__VA_ARGS__)

#define ONEPOS_LOG_W(fmt, ...)  LOG_W(ONEPOS_LOG_TAG, fmt, ##__VA_ARGS__)

#define ONEPOS_LOG_I(fmt, ...)  LOG_I(ONEPOS_LOG_TAG, fmt, ##__VA_ARGS__)

#define ONEPOS_LOG_D(fmt, ...)  LOG_D(ONEPOS_LOG_TAG, fmt, ##__VA_ARGS__)

#else /* Not define OS_USING_DLOG, using kernel printf */
#include <os_util.h>

#if !defined(ONEPOS_LOG_LVL)
#define ONEPOS_LOG_LVL                       ONEPOS_LOG_INFO
#endif

#define ONEPOS_LOG_ERROR                    (3)   /* Error conditions */
#define ONEPOS_LOG_WARNING                  (4)   /* Warning conditions */
#define ONEPOS_LOG_INFO                     (6)   /* Informational */
#define ONEPOS_LOG_DEBUG                    (7)   /* Debug-level messages */

#if (ONEPOS_LOG_ERROR <= ONEPOS_LOG_LVL)
#define ONEPOS_LOG_E(fmt, ...) os_kprintf("[ERROR] [%s] [%s][%d] " fmt "\r\n", ONEPOS_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ONEPOS_LOG_E(fmt, ...)
#endif

#if (ONEPOS_LOG_WARNING <= ONEPOS_LOG_LVL)
#define ONEPOS_LOG_W(fmt, ...)  os_kprintf("[WARN] [%s] [%s][%d] " fmt "\r\n", ONEPOS_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ONEPOS_LOG_W(fmt, ...)
#endif

#if (ONEPOS_LOG_INFO <= ONEPOS_LOG_LVL)
#define ONEPOS_LOG_I(fmt, ...)  os_kprintf("[INFO] [%s] [%s][%d] " fmt "\r\n", ONEPOS_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ONEPOS_LOG_I(fmt, ...)
#endif

#if (ONEPOS_LOG_DEBUG <= ONEPOS_LOG_LVL)
#define ONEPOS_LOG_D(fmt, ...) os_kprintf("[DEBUG] [%s] [%s][%d] " fmt "\r\n", ONEPOS_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ONEPOS_LOG_D(fmt, ...)
#endif

#endif /* OS_USING_DLOG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ONEPOS_LOG_H__ */

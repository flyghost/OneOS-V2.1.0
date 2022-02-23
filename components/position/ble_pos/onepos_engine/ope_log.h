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
 * @file        ope_log.h
 * 
 * @brief       Print control
 * 
 * @details     The control system prints information
 * 
 * @revision
 * Date         Author          Notes
 * 2021-04-29   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __OPE_LOG_H__
#define __OPE_LOG_H__


#include <oneos_config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if !defined(OPE_LOG_TAG)
#define OPE_LOG_TAG                       "OPE_LOG"
#endif

#ifdef OS_USING_OPE_LOG
#include <dlog.h>

/* Debug contorol by dlog config */
#define OPE_LOG_E(fmt, ...)  LOG_E(OPE_LOG_TAG, fmt, ##__VA_ARGS__)

#define OPE_LOG_W(fmt, ...)  LOG_W(OPE_LOG_TAG, fmt, ##__VA_ARGS__)

#define OPE_LOG_I(fmt, ...)  LOG_I(OPE_LOG_TAG, fmt, ##__VA_ARGS__)

#define OPE_LOG_D(fmt, ...)  LOG_D(OPE_LOG_TAG, fmt, ##__VA_ARGS__)


#else /* Not define OS_USING_OPE_LOG, using kernel printf */
#include <os_util.h>


#if !defined(OPE_LOG_LVL)
#define OPE_LOG_LVL                       OPE_LOG_WARNING
#endif

#define OPE_LOG_ERROR                    (3)   /* Error conditions */
#define OPE_LOG_WARNING                  (4)   /* Warning conditions */
#define OPE_LOG_INFO                     (6)   /* Informational */
#define OPE_LOG_DEBUG                    (7)   /* Debug-level messages */

#if (OPE_LOG_ERROR <= OPE_LOG_LVL)
#define OPE_LOG_E(fmt, ...) os_kprintf("[ERROR] [%s] [%s][%d] " fmt "\r\n", OPE_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define OPE_LOG_E(fmt, ...)
#endif

#if (OPE_LOG_WARNING <= OPE_LOG_LVL)
#define OPE_LOG_W(fmt, ...)  os_kprintf("[WARN] [%s] [%s][%d] " fmt "\r\n", OPE_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define OPE_LOG_W(fmt, ...)
#endif

#if (OPE_LOG_INFO <= OPE_LOG_LVL)
#define OPE_LOG_I(fmt, ...)  os_kprintf("[INFO] [%s] [%s][%d] " fmt "\r\n", OPE_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define OPE_LOG_I(fmt, ...)
#endif

#if (OPE_LOG_DEBUG <= OPE_LOG_LVL)
#define OPE_LOG_D(fmt, ...) os_kprintf("[DEBUG] [%s] [%s][%d] " fmt "\r\n", OPE_LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define OPE_LOG_D(fmt, ...)
#endif

#endif /* OS_USING_OPE_LOG */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __OPE_LOG_H__ */

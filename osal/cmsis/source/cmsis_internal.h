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
 * @file        cmsis_internal.h
 *
 * @brief       internal head file for CMSIS APIs adapter
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-26   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __CMSIS_INTERNAL_H__
#define __CMSIS_INTERNAL_H__

#include <oneos_cmsis.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @def         SYS_MALLOC_CTRL_BLK
 *
 * @brief       System malloc memory for control block flag
 ***********************************************************************************************************************
 */
#define SYS_MALLOC_CTRL_BLK         0x10

/**
 ***********************************************************************************************************************
 * @def         SYS_MALLOC_MEM
 *
 * @brief       System malloc memory flag
 ***********************************************************************************************************************
 */
#define SYS_MALLOC_MEM              0x02

#define IdInvalid          0x00U
#define IdThread           0x01U
#define IdTimer            0x02U
#define IdEventFlags       0x03U
#define IdMutex            0x04U
#define IdSemaphore        0x05U
#define IdMemoryPool       0x06U
#define IdMessage          0x07U
#define IdMessageQueue     0x08U

#ifdef __cplusplus
}
#endif
#endif


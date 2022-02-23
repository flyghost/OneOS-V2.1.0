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
 * @file        iot_platform_types_oneos.h
 * 
 * @brief       OneOS definitions of platform layer types. 
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-08-25   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#ifndef IOT_PLATFORM_TYPES_TEMPLATE_H_
#define IOT_PLATFORM_TYPES_TEMPLATE_H_

#include <os_sem.h>
#include <os_mutex.h>
#include <os_timer.h>

/**
 * @brief Set this to the target system's mutex type.
 */
typedef os_mutex_t * _IotSystemMutex_t;


/**
 * @brief Set this to the target system's semaphore type.
 */
typedef os_sem_t * _IotSystemSemaphore_t;


/**
 * @brief Set this to the target system's timer type.
 */
typedef os_timer_t * _IotSystemTimer_t;


/**
 * @brief The format for remote server host and port on this system.
 */
typedef struct IotNetworkServerInfo * _IotNetworkServerInfo_t;

/**
 * @brief The format for network credentials on this system.
 */
typedef struct IotNetworkCredentials * _IotNetworkCredentials_t;

/**
 * @brief The handle of a network connection on this system.
 */
typedef struct _networkConnection * _IotNetworkConnection_t;

#endif /* ifndef IOT_PLATFORM_TYPES_TEMPLATE_H_ */

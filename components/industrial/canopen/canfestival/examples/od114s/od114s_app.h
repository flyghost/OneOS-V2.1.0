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
 * @file        od114s_app.h
 *
 * @brief
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __CIA402M_APP_h__
#define __CIA402M_APP_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <oneos_config.h>
#ifdef OS_USING_CANFESTIVAL_EXAMPLE_OD114S
#include "cf_data.h"

#define OD114S_NODEID 4

#define PDO_TRANSMISSION_TYPE 1
#define CONTROLLER_NODEID     1

#define PRODUCER_HEARTBEAT_TIME 500
#define CONSUMER_HEARTBEAT_TIME 1000

#endif

#ifdef __cplusplus
};
#endif

#endif

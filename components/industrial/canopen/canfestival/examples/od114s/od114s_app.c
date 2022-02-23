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
 * @file        od114s_app.c
 *
 * @brief       This file implement od114s driver
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <drv_cfg.h>
#include <oneos_config.h>
#ifdef OS_USING_CANFESTIVAL_EXAMPLE_OD114S
#include "cf_canfestival.h"
#include "od114s_app.h"
#include "od114s_od.h"

#include "dlog.h"
#define TAG "od114s"

#ifndef CANFESTIVAL_DEVICE_NAME
#define CANFESTIVAL_DEVICE_NAME "can1"
#endif
#ifndef CANFESTIVAL_DEVICE_BAUD
#define CANFESTIVAL_DEVICE_BAUD "1000000"
#endif
#ifndef OD114S_NODEID
#define OD114S_NODEID 4
#endif

static s_BOARD sg_can = {CANFESTIVAL_DEVICE_NAME, CANFESTIVAL_DEVICE_BAUD};

int canopen_app_init(void)
{
    CO_Data *d;
    CAN_PORT canport;
    canopen_init();/* must init first */
    canport = canopen_portopen(&sg_can);
    OS_ASSERT(canport);
    d                  = &od114s_od_Data;
    d->heartbeatError  = canfestival_heartbeatError;
    d->initialisation  = canfestival_initialisation;
    d->preOperational  = canfestival_preOperational;
    d->operational     = canfestival_operational;
    d->stopped         = canfestival_stopped;
    d->post_sync       = canfestival_post_sync;
    d->post_TPDO       = canfestival_post_TPDO;
    d->storeODSubIndex = (storeODSubIndex_t)canfestival_storeODSubIndex;
    d->post_emcy       = (post_emcy_t)canfestival_post_emcy;
    canopen_node_init(canport,d,OD114S_NODEID);
    
    CanOpen_TimerInit();
    CanOpen_StartTimerLoop(NULL);
    return 0;
}
OS_APP_INIT(canopen_app_init, OS_INIT_SUBLEVEL_LOW);
#endif

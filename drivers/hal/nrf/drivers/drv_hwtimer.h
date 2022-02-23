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
 * @file        drv_hwtimer.h
 *
 * @brief       This file implements hwtimer driver for nrf.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_HWTIMER_H__
#define __DRV_HWTIMER_H__

#include <board.h>
#include <os_task.h>
#ifdef OS_USING_CLOCKSOURCE
#include "clocksource.h"
#endif

#ifdef OS_USING_CLOCKEVENT
#include "clockevent.h"
#endif

#include "nrfx_timer.h"

struct nrf5_timer_info {
    nrfx_timer_t timer_periph;
    nrf_timer_cc_channel_t channel;
    nrf_timer_short_mask_t short_mask;
};

#endif

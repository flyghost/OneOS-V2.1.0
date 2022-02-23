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
 * @file        drv_lptim.c
 *
 * @brief       This file implements low power timer driver for fm33.
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_task.h>
#include <os_memory.h>
#include <timer/clockevent.h>

#include "drv_lptim.h"

static os_list_node_t fm_lptimer_list = OS_LIST_INIT(fm_lptimer_list);

void LPTIM_IRQHandler()
{
    struct fm33_lptimer *lptimer = OS_NULL;

    os_list_for_each_entry(lptimer, &fm_lptimer_list, struct fm33_lptimer, list)
    {
        if(FL_LPTIM32_IsEnabledIT_Update(lptimer->info->instance) && 
            FL_LPTIM32_IsActiveFlag_Update(lptimer->info->instance))
        {
            os_clockevent_isr((os_clockevent_t *)lptimer);
            FL_LPTIM32_ClearFlag_Update(lptimer->info->instance);
        }
    }
}

static void fm33_lptimer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct fm33_lptimer *lptimer = OS_NULL;

    FL_LPTIM32_InitTypeDef     defaultInitStruct;

    OS_ASSERT(prescaler == 0);
    OS_ASSERT(count != 0);

    OS_UNREFERENCE(prescaler);

    lptimer = (struct fm33_lptimer *)ce;

    defaultInitStruct.clockSource           = FL_RCC_LPTIM32_CLK_SOURCE_LSCLK;
    defaultInitStruct.prescalerClockSource  = FL_LPTIM32_CLK_SOURCE_INTERNAL;
    defaultInitStruct.prescaler             = FL_LPTIM32_PSC_DIV128;
    defaultInitStruct.autoReload            = count;
    defaultInitStruct.mode                  = FL_LPTIM32_OPERATION_MODE_NORMAL;
    defaultInitStruct.onePulseMode          = FL_LPTIM32_ONE_PULSE_MODE_SINGLE;
    defaultInitStruct.countEdge             = FL_LPTIM32_ETR_COUNT_EDGE_RISING;
    defaultInitStruct.triggerEdge           = FL_LPTIM32_ETR_TRIGGER_EDGE_RISING;

    FL_LPTIM32_Init(lptimer->info->instance, &defaultInitStruct );

    NVIC_EnableIRQ(lptimer->info->irqn);

    FL_LPTIM32_Enable(lptimer->info->instance);
}

static void fm33_lptimer_stop(os_clockevent_t *ce)
{
    struct fm33_lptimer *lptimer = OS_NULL;

    lptimer = (struct fm33_lptimer *)ce;

    FL_LPTIM32_Disable(lptimer->info->instance);
}

os_uint64_t fm33_lptimer_read(void *clock)
{
    struct fm33_lptimer *lptimer = OS_NULL;

    OS_UNREFERENCE(clock);

    lptimer = (struct fm33_lptimer *)clock;

    return (os_uint64_t)FL_LPTIM32_ReadCounter(lptimer->info->instance);
}

static const struct os_clockevent_ops fm_lptim_ops =
{
    .start = fm33_lptimer_start,
    .stop  = fm33_lptimer_stop,
    .read  = fm33_lptimer_read,
};

/**
 ***********************************************************************************************************************
 * @brief           fm33_tim_probe:probe timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int fm33_lptim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct fm33_lptimer *lptimer = OS_NULL;

    lptimer = os_calloc(1, sizeof(struct fm33_lptimer));
    OS_ASSERT(lptimer);

    lptimer->info = (struct fm33_lptimer_info *)dev->info;


    lptimer->freq = 32000 / 128;

    lptimer->ce.rating  = 50;
    lptimer->ce.freq    = lptimer->freq;
    lptimer->ce.mask    = 0xfffful;
    
    lptimer->ce.prescaler_mask = 0;
    lptimer->ce.prescaler_bits = 0;

    lptimer->ce.count_mask = 0xffffffffull;
    lptimer->ce.count_bits = 32;

    lptimer->ce.feature  = OS_CLOCKEVENT_FEATURE_PERIOD;

    lptimer->ce.min_nsec = NSEC_PER_SEC / lptimer->ce.freq;

    lptimer->ce.ops     = &fm_lptim_ops;
    os_clockevent_register(dev->name, &lptimer->ce);

    os_list_add(&fm_lptimer_list, &lptimer->list);

    return OS_EOK;
}

OS_DRIVER_INFO fm33_lptim_driver = {
    .name   = "LPTIM_Type",
    .probe  = fm33_lptim_probe,
};

OS_DRIVER_DEFINE(fm33_lptim_driver, PREV, OS_INIT_SUBLEVEL_HIGH);


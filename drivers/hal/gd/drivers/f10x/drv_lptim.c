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
 * @brief       This file implements low power timer driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_task.h>
#include <os_irq.h>
#include <os_memory.h>
#include <timer/timer.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.lptimer"
#include <drv_log.h>


struct stm32_lptimer {
    os_clockevent_t      ce;
    
    LPTIM_HandleTypeDef *handle;
    
    os_uint32_t freq;
    
    os_list_node_t list;
};

static os_list_node_t stm32_lptimer_list = OS_LIST_INIT(stm32_lptimer_list);

void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
    struct stm32_lptimer *timer;
    
    os_list_for_each_entry(timer, &stm32_lptimer_list, struct stm32_lptimer, list)
    {
        if (timer->handle == hlptim)
        {
            os_clockevent_isr((os_clockevent_t *)timer);
            return;
        }
    }
}

static void stm32_lptimer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct stm32_lptimer *timer;
    LPTIM_HandleTypeDef *lptim;

    OS_ASSERT(prescaler == 0);
    OS_ASSERT(count != 0);

    timer = (struct stm32_lptimer *)ce;
    lptim = timer->handle;

    HAL_LPTIM_Counter_Start_IT(lptim, count);
}

static void stm32_lptimer_stop(os_clockevent_t *ce)
{
    struct stm32_lptimer *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct stm32_lptimer *)ce;

    HAL_LPTIM_Counter_Stop_IT(timer->handle);
}

os_uint64_t stm32_lptimer_read(void *clock)
{
    struct stm32_lptimer *timer;

    timer = (struct stm32_lptimer *)clock;

    return HAL_LPTIM_ReadCounter(timer->handle);
}

static const struct os_clockevent_ops stm32_lptim_ops =
{
    .start = stm32_lptimer_start,
    .stop  = stm32_lptimer_stop,
    .read  = stm32_lptimer_read,
};

/**
 ***********************************************************************************************************************
 * @brief           stm32_tim_probe:probe timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int stm32_lptim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_lptimer *timer;
    LPTIM_HandleTypeDef  *lptim;

    timer = os_calloc(1, sizeof(struct stm32_lptimer));
    OS_ASSERT(timer);

    lptim = (LPTIM_HandleTypeDef *)dev->info;
    
    timer->handle = lptim;

    timer->freq = 32000 / 32;

    timer->ce.rating  = 50;
    timer->ce.freq    = timer->freq;
    timer->ce.mask    = 0xfffful;
    
    timer->ce.prescaler_mask = 0;
    timer->ce.prescaler_bits = 0;
    
    timer->ce.count_mask = 0xfffful;
    timer->ce.count_bits = 16;

    timer->ce.feature  = OS_CLOCKEVENT_FEATURE_PERIOD;

    timer->ce.min_nsec = 10 * NSEC_PER_SEC / timer->ce.freq;
    
    timer->ce.ops     = &stm32_lptim_ops;
    os_clockevent_register(dev->name, &timer->ce);

    os_list_add(&stm32_lptimer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO stm32_lptim_driver = {
    .name   = "LPTIM_HandleTypeDef",
    .probe  = stm32_lptim_probe,
};

OS_DRIVER_DEFINE(stm32_lptim_driver, "1");


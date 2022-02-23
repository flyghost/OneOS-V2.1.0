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
#include <os_memory.h>
#include <timer/timer.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.lptimer"
#include <drv_log.h>

#include <fm33g0xx_lptim.h>

#define LPTIME_FRE  (32*1000/128)

struct fm_lptimer {
    os_clockevent_t      ce;
    
    TIM_HandleTypeDef *handle;
    
    os_uint32_t freq;
    
    os_list_node_t list;
};

static os_list_node_t fm_lptimer_list = OS_LIST_INIT(fm_lptimer_list);


void LPTIM_IRQHandler(void)
{
    LPTIM_LPTIF_COMPIF_Clr();
    LPTIM_LPTIF_OVIF_Clr();
    LPTIM_LPTIF_TRIGIF_Clr();
}

/* 32kHz时钟,使用128分频，一次最长低功耗时间:0xffff*1/(32*1000/128) = 262.14 s */
void LPTIMER_COMM_TIMER_DEMO_init(os_uint64_t count)
{
    LPTIM_InitTypeDef init_para;

    //使能LPTIMER的外设时钟
    RCC_PERCLK_SetableEx( LPTFCLK, ENABLE );	
    RCC_PERCLK_SetableEx( LPTRCLK, ENABLE );
    init_para.LPTIM_TMODE = LPTIM_LPTCFG_TMODE_PWMIM;
    init_para.LPTIM_MODE = LPTIM_LPTCFG_MODE_SINGLE;

    init_para.LPTIM_CLK_SEL = LPTIM_LPTCFG_CLKSEL_LSCLK;
    init_para.LPTIM_CLK_DIV = LPTIM_LPTCFG_DIVSEL_128;

    init_para.LPTIM_compare_value = count;
    init_para.LPTIM_target_value = 0xffff;

    LPTIM_Init(&init_para);

    NVIC_DisableIRQ(LPTIM_IRQn);
    NVIC_SetPriority(LPTIM_IRQn, 2);
    NVIC_EnableIRQ(LPTIM_IRQn);		
}


void LPTIMER_COMM_TIMER_DEMO_start(void)
{	
    LPTIM_LPTIE_COMPIE_Setable(ENABLE);     /* 使用比较中断唤醒低功耗 */
    LPTIM_LPTCTRL_LPTEN_Setable(ENABLE);
}

static void fm_lptimer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    OS_ASSERT(prescaler == 0);
    OS_ASSERT(count != 0);


    LPTIMER_COMM_TIMER_DEMO_init(count);
    LPTIMER_COMM_TIMER_DEMO_start();
}

void LPTIMER_COMM_TIMER_stop(void)
{
    LPTIM_LPTIE_COMPIE_Setable(DISABLE);
    LPTIM_LPTCTRL_LPTEN_Setable(DISABLE);
}

static void fm_lptimer_stop(os_clockevent_t *ce)
{
    LPTIMER_COMM_TIMER_stop();
}

os_uint64_t fm_lptimer_read(void *clock)
{
    return (os_uint64_t)LPTIM_LPTCNT_Read();
}

static const struct os_clockevent_ops fm_lptim_ops =
{
    .start = fm_lptimer_start,
    .stop  = fm_lptimer_stop,
    .read  = fm_lptimer_read,
};

/**
 ***********************************************************************************************************************
 * @brief           fm_tim_probe:probe timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int fm_lptim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct fm_lptimer *timer;
    TIM_HandleTypeDef  *lptim;

    timer = os_calloc(1, sizeof(struct fm_lptimer));
    OS_ASSERT(timer);

    lptim = (TIM_HandleTypeDef *)dev->info;
    
    timer->handle = lptim;

    timer->freq = LPTIME_FRE;

    timer->ce.rating  = 50;
    timer->ce.freq    = timer->freq;
    timer->ce.mask    = 0xfffful;
    
    timer->ce.prescaler_mask = 0;
    timer->ce.prescaler_bits = 0;
    
    timer->ce.count_mask = 0xfffful;
    timer->ce.count_bits = 16;

    timer->ce.feature  = OS_CLOCKEVENT_FEATURE_ONESHOT;

    timer->ce.min_nsec = NSEC_PER_SEC / timer->ce.freq;
    
    timer->ce.ops     = &fm_lptim_ops;
    os_clockevent_register(dev->name, &timer->ce);

    os_list_add(&fm_lptimer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO fm_lptim_driver = {
    .name   = "LPTIM_HandleTypeDef",
    .probe  = fm_lptim_probe,
};

OS_DRIVER_DEFINE(fm_lptim_driver, PREV, OS_INIT_SUBLEVEL_HIGH);



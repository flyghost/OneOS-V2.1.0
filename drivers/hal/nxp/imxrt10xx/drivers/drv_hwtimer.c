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
 * @file        drv_hwtimer.c
 *
 * @brief       This file implements hwtimer driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_task.h>

#define LOG_TAG             "drv.hwtimer"
#include <drv_log.h>

#include <board.h>
#include <os_memory.h>
#include "drv_hwtimer.h"
#include "fsl_gpt.h"

#if defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL
#error "Please don't define 'FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL'!"
#endif

/* Select IPG Clock as PERCLK_CLK clock source */
#define EXAMPLE_GPT_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for PERCLK_CLK clock source */
#define EXAMPLE_GPT_CLOCK_DIVIDER_SELECT (5U)
/* Get source clock for GPT driver (GPT prescaler = 6) */
#define EXAMPLE_GPT_CLK_FREQ (CLOCK_GetFreq(kCLOCK_IpgClk) / (EXAMPLE_GPT_CLOCK_DIVIDER_SELECT + 1U))

struct imxrt_gpt
{
    os_clockevent_t         ce;
    struct nxp_gpt_info    *info;
    os_list_node_t          list;
};

static os_list_node_t imxrt_gpt_list = OS_LIST_INIT(imxrt_gpt_list);

static void gpt_irq_callback(struct imxrt_gpt *imxrt_gpt)
{
    GPT_Type *base = imxrt_gpt->info->gpt_base;

    if (GPT_GetStatusFlags(base, kGPT_OutputCompare1Flag) != 0)
    {
        GPT_ClearStatusFlags(base, kGPT_OutputCompare1Flag);
        os_clockevent_isr((os_clockevent_t *)imxrt_gpt);
    }

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F, Cortex-M7, Cortex-M7F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U || __CORTEX_M == 7U)
    __DSB();
#endif
}

#define GPT_IRQHandler_DEFINE(__index)                                        \
void GPT##__index##_IRQHandler(void)                                          \
{                                                                               \
    struct imxrt_gpt *imxrt_gpt;                                                \
                                                                                \
    os_list_for_each_entry(imxrt_gpt, &imxrt_gpt_list, struct imxrt_gpt, list)  \
    {                                                                           \
        if (imxrt_gpt->info->gpt_base == GPT##__index)                        \
        {                                                                       \
            break;                                                              \
        }                                                                       \
    }                                                                           \
                                                                                \
    if (imxrt_gpt->info->gpt_base == GPT##__index)                            \
        gpt_irq_callback(imxrt_gpt);                                            \
}

GPT_IRQHandler_DEFINE(1);
GPT_IRQHandler_DEFINE(2);

static os_uint64_t imxr_timer_read(void *clock)
{
    struct imxrt_gpt *timer;

    timer = (struct imxrt_gpt *)clock;

    return GPT_GetCurrentTimerCount(timer->info->gpt_base);
}

static void imxr_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct imxrt_gpt *timer;
    GPT_Type *gpt_base;

    OS_ASSERT(ce != OS_NULL);
    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer = (struct imxrt_gpt *)ce;
    
    gpt_base = timer->info->gpt_base;

    OS_ASSERT(timer != OS_NULL);

    /* period */
    //gpt_base->CR |= GPT_CR_FRR_MASK;

    GPT_SetOutputCompareValue(gpt_base, kGPT_OutputCompare_Channel1, count);

    GPT_EnableInterrupts(gpt_base, kGPT_OutputCompare1InterruptEnable);

    GPT_StartTimer(gpt_base);
}

static void imxr_timer_stop(os_clockevent_t *ce)
{
    struct imxrt_gpt *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct imxrt_gpt *)ce;

    GPT_StopTimer(timer->info->gpt_base);
}

static const struct os_clockevent_ops imxrt_tim_ops =
{
    .start = imxr_timer_start,
    .stop  = imxr_timer_stop,
    .read  = imxr_timer_read,
};

static os_uint32_t imxrt_clock_freq(struct nxp_gpt_info *info)
{
    GPT_Type *gpt_base = info->gpt_base;

#ifdef GPT1_PERIPHERAL
    if (gpt_base == GPT1_PERIPHERAL)
        return GPT1_CLOCK_SOURCE / info->config->divider;
#endif

#ifdef GPT2_PERIPHERAL
    if (gpt_base == GPT2_PERIPHERAL)
        return GPT2_CLOCK_SOURCE / info->config->divider;
#endif

    return 0;
}

static int imxrt_gpt_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct nxp_gpt_info *info = (struct nxp_gpt_info *)dev->info;
    struct imxrt_gpt *imxrt_gpt = os_calloc(1, sizeof(struct imxrt_gpt));

    OS_ASSERT(imxrt_gpt);

    imxrt_gpt->info = info;

    os_clockevent_t *ce = &imxrt_gpt->ce;
   
    ce->rating  = 320;
    ce->freq    = imxrt_clock_freq(info);
    ce->mask    = 0xffffffffull;
    
    ce->prescaler_mask = 0xfffful;
    ce->prescaler_bits = 16;

    ce->count_mask = 0xfffffffful;
    ce->count_bits = 32;

    ce->feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

    ce->min_nsec = NSEC_PER_SEC / ce->freq;
    
    ce->ops     = &imxrt_tim_ops;
    os_clockevent_register(dev->name, ce);

    level = os_irq_lock();
    os_list_add_tail(&imxrt_gpt_list, &imxrt_gpt->list);
    os_irq_unlock(level);
    
    return result;
}

OS_DRIVER_INFO imxrt_gpt_driver = {
    .name   = "GPT_Type",
    .probe  = imxrt_gpt_probe,
};

OS_DRIVER_DEFINE(imxrt_gpt_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);


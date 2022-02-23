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
 * @file        clocksource_cortexm.c
 *
 * @brief       This file provides functions for cputime_cortexm init.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_task.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>
#include <timer/clocksource_cortexm.h>

#if defined(OS_USING_DWT_FOR_CLOCKSOURCE) && defined(DWT)

/* dwt for os_clocksource */
static os_clocksource_t cortexm_dwt_clocksource;

static os_uint64_t cortexm_dwt_read(void *clock)
{
    return DWT->CYCCNT;
}

void cortexm_dwt_init(void)
{
    os_uint32_t DWT_CNT = 0;
    os_uint32_t t_DWT;
    
    /* check support bit */
    if ((DWT->CTRL & (1UL << DWT_CTRL_NOCYCCNT_Pos)) == 0)
    {
        /* enable trace*/
        CoreDebug->DEMCR |= (1UL << CoreDebug_DEMCR_TRCENA_Pos);

#ifdef ARCH_ARM_CORTEX_M7
        /* unlock (CM7) */
        DWT->LAR = 0xC5ACCE55;
#endif
        DWT->CYCCNT = 0;

        /* whether cycle counter not enabled */
        if ((DWT->CTRL & (1UL << DWT_CTRL_CYCCNTENA_Pos)) == 0)
        {
            /* enable cycle counter */
            DWT->CTRL |= (1UL << DWT_CTRL_CYCCNTENA_Pos);
        }
        DWT_CNT = DWT->CYCCNT;
        
        /* sure dwt is runing */
        for (t_DWT = 0x10; t_DWT; t_DWT--);
        
        /* dwt not runing */
        if (DWT_CNT == DWT->CYCCNT)
          return;
        
        cortexm_dwt_clocksource.rating  = 320;
        cortexm_dwt_clocksource.freq    = SystemCoreClock;
        cortexm_dwt_clocksource.mask    = 0xffffffffull;
        cortexm_dwt_clocksource.read    = cortexm_dwt_read;
        os_clocksource_register("cortexm_dwt", &cortexm_dwt_clocksource);
    }
}

#endif /* OS_USING_DWT_FOR_CLOCKSOURCE */

#if defined(OS_USING_SYSTICK_FOR_CLOCKSOURCE) || defined(OS_USING_SYSTICK_FOR_CLOCKEVENT)

typedef struct
{
  volatile os_uint32_t CTRL;    /* Offset: 0x000 (R/W)  systick Control and Status Register */
  volatile os_uint32_t LOAD;    /* Offset: 0x004 (R/W)  systick Reload Value Register */
  volatile os_uint32_t VAL;     /* Offset: 0x008 (R/W)  systick Current Value Register */
  volatile os_uint32_t CALIB;   /* Offset: 0x00C (R/ )  systick Calibration Register */
} os_systick_t;

#define systick ((os_systick_t *)SysTick_BASE)   /* systick configuration struct */

#define SYSTICK_CTRL_TICKINT_MASK           (1 << 1)
#define SYSTICK_CTRL_ENABLE_MASK            (1 << 0)

static os_uint64_t cortexm_systick_read(void *clock)
{    
    return systick->LOAD - systick->VAL;
}

#endif

#ifdef OS_USING_SYSTICK_FOR_CLOCKSOURCE

static os_clocksource_t cortexm_systick_clocksource;

void cortexm_systick_clocksource_init(void)
{
    systick->LOAD  = 0x00fffffful;  /* set reload register */
    systick->VAL   = 0UL;           /* Load the systick Counter Value */
    systick->CTRL &= ~SYSTICK_CTRL_TICKINT_MASK;
    systick->CTRL |=  SYSTICK_CTRL_ENABLE_MASK;

    cortexm_systick_clocksource.rating  = 240;
    cortexm_systick_clocksource.freq    = SystemCoreClock;
    cortexm_systick_clocksource.mask    = 0x00ffffffull;
    cortexm_systick_clocksource.read    = cortexm_systick_read;
    
    os_clocksource_register("cortexm_systick", &cortexm_systick_clocksource);
}

#endif /* OS_USING_SYSTICK_FOR_CLOCKSOURCE */

#ifdef OS_USING_SYSTICK_FOR_CLOCKEVENT

static os_clockevent_t cortexm_systick_clockevent;

void cortexm_systick_clockevent_isr(void)
{
    os_clockevent_isr(&cortexm_systick_clockevent);
}

static void cortexm_systick_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{    
    systick->LOAD  = count;     /* set reload register */
    systick->VAL   = 0UL;       /* Load the systick Counter Value */
    systick->CTRL |= SYSTICK_CTRL_TICKINT_MASK | SYSTICK_CTRL_ENABLE_MASK;
}

static void cortexm_systick_stop(os_clockevent_t *ce)
{
    systick->CTRL &= ~(SYSTICK_CTRL_TICKINT_MASK | SYSTICK_CTRL_ENABLE_MASK);
}

static const struct os_clockevent_ops cortexm_systick_clockevent_ops =
{
    .start = cortexm_systick_start,
    .stop  = cortexm_systick_stop,
    .read  = cortexm_systick_read,
};

void cortexm_systick_clockevent_init(void)
{
    cortexm_systick_clockevent.rating  = 240;
    cortexm_systick_clockevent.freq    = SystemCoreClock;
    cortexm_systick_clockevent.mask    = 0x00ffffffull;

    cortexm_systick_clockevent.prescaler_mask = 1;
    cortexm_systick_clockevent.prescaler_bits = 0;

    cortexm_systick_clockevent.count_mask = 0x00ffffffull;
    cortexm_systick_clockevent.count_bits = 24;

    cortexm_systick_clockevent.feature  = OS_CLOCKEVENT_FEATURE_PERIOD;

    cortexm_systick_clockevent.min_nsec = NSEC_PER_SEC / SystemCoreClock;

    cortexm_systick_clockevent.ops     = &cortexm_systick_clockevent_ops;
    
    os_clockevent_register("cortexm_systick", &cortexm_systick_clockevent);
}

#endif /* OS_USING_SYSTICK_FOR_CLOCKEVENT */

#ifdef OS_USING_TICKLESS_LPMGR

#include <lpmgr.h>

static os_uint32_t gs_sys_load = 0;
static os_uint32_t gs_sys_ctrl = 0;

static int systick_suspend(void *priv, os_uint8_t mode)
{
    if (mode == SYS_SLEEP_MODE_LIGHT)
    {
        return OS_EOK;
    }

    gs_sys_load = SysTick->LOAD;
    gs_sys_ctrl = SysTick->CTRL;

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    
    return OS_EOK;
}

static void systick_resume(void *priv, os_uint8_t mode)
{
    if (mode == SYS_SLEEP_MODE_LIGHT)
    {
        return;
    }
    
    SysTick->LOAD  = gs_sys_load;
    SysTick->CTRL  = gs_sys_ctrl;
}

static const struct os_lpmgr_device_ops systick_lpmgr_ops =
{
    .suspend = systick_suspend,
    .resume  = systick_resume,
};

static os_err_t systick_lpmgr_init(void)
{
    return os_lpmgr_device_register_high_prio(OS_NULL, &systick_lpmgr_ops);
}

OS_CMPOENT_INIT(systick_lpmgr_init, OS_INIT_SUBLEVEL_HIGH);

#endif


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
 * @file        board.c
 *
 * @brief       The 7231n board init of OneOS.
 *
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-18   OneOS Team      First version
 ***********************************************************************************************************************
 */

#include "os_hw.h"
#include "os_task.h"
#include "os_types.h"
#include "os_memory.h"
#include "os_sem.h"
#include "os_object.h"
#include "shell.h"

#include "board.h"
#include "interrupt.h"
#include "driver_pub.h"
#include "drv_uart.h"
#include "include.h"
#include "func_pub.h"
#include "gpio_pub.h"

#include "icu.h"
#include "arm_arch.h"
#include "intc.h"
#include "portmacro.h"


enum wdg_status {
    WDG_STATUS_STOP,
    WDG_STATUS_REBOOT,
    WDG_STATUS_WATCH,
    WDG_STATUS_MAX,
};

struct wdg_context {
    enum wdg_status wdg_flag;
    int threshold_in_tick;
    int consumed_in_tick;
    os_tick_t last_fresh_in_tick; /* add time to dealing with power save mode */
};

static struct wdg_context g_wdg_context = { WDG_STATUS_STOP, 0, 0 };

extern void os_clk_init(void);
extern void bk_reboot(void);


void os_hw_board_init(void)
{
#if defined(RT_USING_HEAP)
    /* init memory system */
#if(CFG_SOC_NAME == SOC_BK7221U)
    rt_system_heap_init(RT_HW_SDRAM_BEGIN, RT_HW_SDRAM_END);
	rt_sdram_heap_init(); 
#endif
    os_memheap_init(&tcm_heap, "TCM", RT_HW_TCM_BEGIN, RT_HW_TCM_END-RT_HW_TCM_BEGIN); 
#endif

/* Heap initialization */
#if defined(OS_USING_HEAP)
		os_system_heap_init((void *)RT_HW_TCM_BEGIN, (void *)RT_HW_TCM_END);
#endif

    
    /* init hardware */
    driver_init();

    /* interrupt init */
    rt_hw_interrupt_init();

    /* init system tick */
    os_clk_init();
	portENABLE_INTERRUPTS();

	os_board_auto_init();

}


#define WDT_DEV_NAME "wdt"
/**
 * reset cpu by dog's time-out
 */
void beken_hw_cpu_reset(void)
{
    bk_reboot();

    while (1);
}

SH_CMD_EXPORT(bk_cpu_reboot,beken_hw_cpu_reset, "bk cpu reboot cmd.");


#ifdef BEKEN_USING_WLAN
static int auto_func_init(void)
{
    func_init_basic();
    func_init_extended();  
    return 0;
}
OS_PREV_INIT(auto_func_init);
#endif

extern void cp15_enable_alignfault(void);
static int auto_enable_alignfault(void)
{
    cp15_enable_alignfault();
    return 0;
}
OS_BOARD_INIT(auto_enable_alignfault);


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
 * @file        drv_ost.c
 *
 * @brief       This file provides function for osticks.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <drv_clock.h>
#include <drv_ost.h>
#include <os_clock.h>
#include <ingenic_ost.h>

void os_hw_ost_handler(void)
{
    os_tick_increase();
    REG_OSTFR = 0;
}

int ostick_config(unsigned int tick_per_second)
{
    struct clk *clk;

    clk = clk_get("sys_ost");

    clk_enable(clk);

    ingenic_ost_config(tick_per_second);

    clk_put(clk);

    return 0;
}


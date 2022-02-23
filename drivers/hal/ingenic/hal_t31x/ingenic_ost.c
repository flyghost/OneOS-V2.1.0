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
 * @brief       This file provides ostick function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <ingenic_ost.h>
#include <ingenic_clock.h>

int ingenic_ost_config(unsigned int tick_per_second)
{
    uint32_t cnt, div;

    div = OST_DIV16;
    cnt = BOARD_EXTAL_CLK / 16;

    REG_OSTECR = 0x3;

    REG_OSTCR = 0x01;
    REG_OST1CNT = 0;

    REG_OST1DFR   = (cnt / tick_per_second - 1);

    REG_OSTCCR = div;

    REG_OSTMR = 0;

    REG_OSTESR = 0x01;

    return 0;
}


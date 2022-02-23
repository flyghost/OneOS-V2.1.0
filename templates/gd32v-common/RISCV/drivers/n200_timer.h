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
 * @file        n200_timer.h
 *
 * @brief       This file provides macro definition for timer.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef N200_TIMER_H
#define N200_TIMER_H

#define TIMER_MSIP          0xFFC
#define TIMER_MSIP_size     0x4
#define TIMER_MTIMECMP      0x8
#define TIMER_MTIMECMP_size 0x8
#define TIMER_MTIME         0x0
#define TIMER_MTIME_size    0x8

#define TIMER_CTRL_ADDR   0xd1000000
#define TIMER_REG(offset) _REG32(TIMER_CTRL_ADDR, offset)
#define TIMER_FREQ        ((uint32_t)SystemCoreClock / 4)    // units HZ

#endif

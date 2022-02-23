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
 * @file        ingenic_clock.h
 *
 * @brief       This file provides micro definition for clock.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#ifndef DRV_CLOCK_H_
#define DRV_CLOCK_H_

#include <ingenic_clock.h>
#include <stdint.h>
#include <os_stddef.h>

int mips_clock_init(void);
extern void *os_memcpy(void *dst, const void *src, os_size_t count);
extern os_int32_t os_strcmp(const char *str1, const char *str2);
struct clk *clk_get(const char *id);
int clk_enable(struct clk *clk);
int clk_is_enabled(struct clk *clk);
void clk_disable(struct clk *clk);
uint32_t clk_get_rate(struct clk *clk);
void clk_put(struct clk *clk);
int clk_set_rate(struct clk *clk, uint32_t rate);

#endif

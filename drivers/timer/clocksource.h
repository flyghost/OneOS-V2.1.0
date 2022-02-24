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
 * @file        cputime.h
 *
 * \@brief      This file provides struct definition and cputime functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef CLOCKSOURCE_H__
#define CLOCKSOURCE_H__

#include <driver.h>
#include <os_types.h>
#include <timer/timer.h>

typedef struct os_clocksource os_clocksource_t;

#define OS_CLOCKSOURCE_NAME_LENGTH  (16)

struct os_clocksource {
    os_device_t parent;
    
    char name[OS_CLOCKSOURCE_NAME_LENGTH];

    /**
     * @brief 每次新的clocksource注册进来，都会触发clocksource_select函数被调用，它按照rating值选择最好的clocksource
     * 
     */
    os_uint32_t rating;
    os_uint32_t freq;
    
    /**
     * @brief 时钟源的计数值用类型cycle_t表示，该类型其实就是无符号64位数。
     * 但是时钟源实际的计数范围可能小于64位，mask的作用就是表示真实的计数范围。
     * 对于一个32位的时钟源硬件，mask等于0xffffffff，即32个bit 1。这种方法的好处是即使时钟源发生溢出，一样可以通过简单的计算得出实际经历的cycle数。
     * 
     * 例如，假设上次读时钟源得到的计数值是cycle_last，这次读取的计数值是cycle_now，则经历的cycle数等于：(cycle_now - cycle_last) & mask
     * 对于一个32位的时钟源，假设cycle_last=0xffffffff，cycle_now=0，即发生了溢出，通过上面的等式依然可以得到经历了一次cycle。
     * 
     */
    os_uint64_t mask;
    os_uint64_t min_nsec;       // 一个硬件counter计数表示多少纳秒
    /**
     * @brief 表示两次读取之间所允许的最大时间间隔。
     * 
     * 当两次读取间隔超过mask+1时就无法正确计算出经历的cycle数，因此该数值需要小于mask+1个cycle所对应的纳秒数。
     * 另一个限制因素来自于clocksource_cyc2ns()函数。该函数用于将cycle数换算成纳秒。由于函数返回值是有符号64位数(s64)，两次读取间隔的cycle数不能使最终的计算结果溢出。
     * 
     * 如果小于32位的计数器，则max_nsec = NSEC_PER_SEC * cs->mask / cs->freq
     * 
     */
    os_uint64_t max_nsec;
    
    /**
     * @brief mult 和 shift 用于将时钟周期数值转换成纳秒值
     * 
     * mult 值越大，精度越高，但是为了计算过程不发生溢出，mult值又不能取得过大
     * 
     * 为此内核假设cycle计数值被转换后的最大时间值：10分钟（600秒），
     * 主要的考虑是CPU进入IDLE状态后，时间信息不会被更新，
     * 只要在10分钟内退出IDLE，clocksource的cycle计数值就可以被正确地转换为相应的时间，然后系统的时间信息可以被正确地更新。
     * 
     */
    os_uint32_t mult;       /* time(ns) = count * mult >> shift */
    os_uint32_t shift;

    // mult_t 和 shift_t 用于将纳秒值转化你成时钟周期数值
    os_uint32_t mult_t;     /* count = time(ns) * mult_t >> shift_t */
    os_uint32_t shift_t;

    os_uint64_t last_update_count;          // 上一次计数值
    os_uint64_t last_update_nsec;           // 上一次计算的纳秒值

    os_uint64_t (*read)(void *clock);   // 读取时钟的tick值

    os_list_node_t list;
};

os_clocksource_t *os_clocksource_best(void);

os_uint64_t os_clocksource_gettime(void);

void os_clocksource_update(void);
void os_clocksource_ndelay(os_uint64_t nsec);
void os_clocksource_register(const char *name, os_clocksource_t *cs);

#endif  /* CLOCKSOURCE_H__ */


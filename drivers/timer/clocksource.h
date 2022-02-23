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

    os_uint32_t rating;
    os_uint32_t freq;
    
    os_uint64_t mask;
    os_uint64_t min_nsec;
    os_uint64_t max_nsec;
    
    os_uint32_t mult;       /* time(ns) = count * mult >> shift */
    os_uint32_t shift;

    os_uint32_t mult_t;     /* count = time(ns) * mult_t >> shift_t */
    os_uint32_t shift_t;

    os_uint64_t last_update_count;
    os_uint64_t last_update_nsec;

    os_uint64_t (*read)(void *clock);

    os_list_node_t list;
};

os_clocksource_t *os_clocksource_best(void);

os_uint64_t os_clocksource_gettime(void);

void os_clocksource_update(void);
void os_clocksource_ndelay(os_uint64_t nsec);
void os_clocksource_register(const char *name, os_clocksource_t *cs);

#endif  /* CLOCKSOURCE_H__ */


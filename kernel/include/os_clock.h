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
 * @file        os_clock.h
 *
 * @brief       The header file for system clock.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __OS_CLOCK_H__
#define __OS_CLOCK_H__

#include <os_types.h>
#include <oneos_config.h>

#ifdef __cplusplus
extern "C" {
#endif

extern os_tick_t os_tick_get(void);
extern void      os_tick_set(os_tick_t tick);
extern void      os_tick_increase(void);
extern os_tick_t os_tick_from_ms(os_uint32_t ms);

#ifdef OS_USING_TICKLESS_LPMGR

extern os_tick_t os_tickless_get_sleep_ticks(void);
extern void      os_tickless_update(os_tick_t sleep_ticks);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __OS_CLOCK_H__ */


/*
// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "port/oc_clock.h"
#include "oneos_config.h"
#include "os_clock.h"
#include "os_task.h"
// #if defined(OS_USING_RTC)
// #include <device.h>
// #include <rtc/rtc.h>
// #endif

static uint32_t prev_time = 0;
static uint32_t high_time = 0;

// uint64_t start_ticks_from_1970 = 0;

void
oc_clock_init(void)
{
// #if defined(OS_USING_RTC)
//     time_t tim;
//     os_device_t *device;

//     device = os_device_find("rtc");
//     OS_ASSERT(device != OS_NULL);

//     os_device_control(device, OS_DEVICE_CTRL_RTC_GET_TIME, &tim);
//     start_ticks_from_1970 = (uint64_t)tim * OS_TICK_PER_SECOND;
//     os_kprintf("ticks from 1970-0-0 is 0x%llx\r\n", start_ticks_from_1970);
// #endif  
}

oc_clock_time_t
oc_clock_time(void)
{
  uint32_t time = os_tick_get();

  if (time < prev_time) {
      high_time++;
  }

  prev_time = time;

  return (uint64_t)high_time << 32 | time;
  // return (uint64_t)high_time << 32 | time + start_ticks_from_1970;
}

unsigned long
oc_clock_seconds(void)
{
  int tick = os_tick_get();
  return tick / OS_TICK_PER_SECOND;
}

void
oc_clock_wait(oc_clock_time_t t)
{
  os_task_tsleep(t);
}

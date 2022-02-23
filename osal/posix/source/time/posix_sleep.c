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
 * @file        posix_sleep.c
 *
 * @brief       This file implements the posix sleep functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdlib.h>
#include <unistd.h>
#include <oneos_config.h>
#include <os_clock.h>
#include <os_task.h>

unsigned int sleep(unsigned int seconds)
{
    os_tick_t delta_tick;

    delta_tick = os_tick_get();
    
    os_task_tsleep(seconds * OS_TICK_PER_SECOND);
    
    delta_tick = os_tick_get() - delta_tick;

    return (seconds - delta_tick / OS_TICK_PER_SECOND);
}

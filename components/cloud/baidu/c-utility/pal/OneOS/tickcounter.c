// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"

#include <stdint.h>
#include <time.h>
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

//#include <os_kernel.h>
#include <os_clock.h>

#define INVALID_TIME_VALUE (time_t)(-1)
typedef struct TICK_COUNTER_INSTANCE_TAG
{
    uint32_t os_tick_count;

} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE *result = (TICK_COUNTER_INSTANCE *)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result == NULL)
    {
        LogError("tickcounter failed: time return INVALID_TIME.");
    }
    else
    {
        result->os_tick_count = os_tick_get();
    }
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
        free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t *current_ms)
{
    int result;

    if (tick_counter == NULL || current_ms == NULL)
    {
        LogError("tickcounter failed: Invalid Arguments.");
        result = __FAILURE__;
    }
    else
    {
        *current_ms =
            (tickcounter_ms_t)(((uint32_t)(os_tick_get() - tick_counter->os_tick_count)) * 1000.0 / OS_TICK_PER_SECOND);
        result = 0;
    }
    return result;
}

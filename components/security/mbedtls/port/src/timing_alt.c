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
 * @file        thread_alt.c
 * 
 * @brief       Implementation of the functions when MBEDTLS_THREADING_ALT defined for mbedtls. 
 * 
 * @details
 * 
 * @revision
 * Date         Author          Notes
 * 2020-08-25   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_TIMING_ALT)
#include <sys/time.h>
#include <os_clock.h>
#include "timing_alt.h"

unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val, int reset)
{
    struct mbedtls_timing_hr_time now;
    now.timer_ms = ((uint64_t)os_tick_get() * (1000 / os_tick_from_ms(1000)));

    if (reset)
    {
        val->timer_ms = now.timer_ms;
    }

    return (unsigned long)(now.timer_ms - val->timer_ms);
}

/*
 * Set delays to watch
 */
void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *)data;

    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;

    if (fin_ms != 0)
    {
        (void)mbedtls_timing_get_timer(&ctx->timer, 1);
    }
}

/*
 * Get number of delays expired
 */
int mbedtls_timing_get_delay( void *data )
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;
    unsigned long elapsed_ms;

    if( ctx->fin_ms == 0 )
        return( -1 );

    elapsed_ms = mbedtls_timing_get_timer( &ctx->timer, 0 );

    if( elapsed_ms >= ctx->fin_ms )
        return( 2 );

    if( elapsed_ms >= ctx->int_ms )
        return( 1 );

    return( 0 );
}

unsigned long mbedtls_timing_hardclock(void)
{
#ifdef OS_USING_RTC
    static int            hardclock_init = 0;
    static struct timeval tv_init;
    struct timeval        tv_cur;

    if (hardclock_init == 0)
    {
        gettimeofday(&tv_init, NULL);
        hardclock_init = 1;
    }

    gettimeofday(&tv_cur, NULL);
    return ((tv_cur.tv_sec - tv_init.tv_sec) * 1000000 + (tv_cur.tv_usec - tv_init.tv_usec));
#else
    static os_tick_t fisrt_tick = 0;
    if (!fisrt_tick)
        fisrt_tick = os_tick_get();
    os_tick_t tick = os_tick_get();
    tick -= fisrt_tick;
    return tick ? tick * (1000000 / OS_TICK_PER_SECOND) : 1;
#endif
}

#endif

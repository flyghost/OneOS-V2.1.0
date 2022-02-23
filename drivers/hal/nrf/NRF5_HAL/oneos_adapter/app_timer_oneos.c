/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(APP_TIMER)

#include "os_timer.h"
#include "app_timer.h"
#include <stdlib.h>
#include <string.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "app_error.h"

#define RTC1_IRQ_PRI            APP_TIMER_CONFIG_IRQ_PRIORITY               /**< Priority of the RTC1 interrupt (used for checking for timeouts and executing timeout handlers). */
#define SWI_IRQ_PRI             APP_TIMER_CONFIG_IRQ_PRIORITY               /**< Priority of the SWI  interrupt (used for updating the timer list). */

// The current design assumes that both interrupt handlers run at the same interrupt level.
// If this is to be changed, protection must be added to prevent them from interrupting each other
// (e.g. by using guard/trigger flags).
STATIC_ASSERT(RTC1_IRQ_PRI == SWI_IRQ_PRI);

#define MAX_RTC_COUNTER_VAL     0x00FFFFFF                                  /**< Maximum value of the RTC counter. */

#define RTC_COMPARE_OFFSET_MIN  3                                           /**< Minimum offset between the current RTC counter value and the Capture Compare register. Although the nRF51 Series User Specification recommends this value to be 2, we use 3 to be safer.*/

#define MAX_RTC_TASKS_DELAY     47                                          /**< Maximum delay until an RTC task is executed. */

#ifdef EGU_PRESENT
#define SWI_PART(_id) CONCAT_2(SWI,_id)
#define EGU_PART(_id) CONCAT_2(_EGU,_id)
#define SWI_IRQ_n(_id) CONCAT_3(SWI_PART(_id), EGU_PART(_id),_IRQn)
#define SWI_IRQ_Handler_n(_id) CONCAT_3(SWI_PART(_id), EGU_PART(_id),_IRQHandler)
#else //EGU_PRESENT
#define SWI_IRQ_n(_id) CONCAT_3(SWI,_id,_IRQn)
#define SWI_IRQ_Handler_n(_id) CONCAT_3(SWI,_id,_IRQHandler)
#endif

#define SWI_IRQn SWI_IRQ_n(APP_TIMER_CONFIG_SWI_NUMBER)
#define SWI_IRQHandler SWI_IRQ_Handler_n(APP_TIMER_CONFIG_SWI_NUMBER)

/**@brief This structure keeps information about osTimer.*/
typedef struct
{
    void *argument;
    os_timer_t *osHandle;
    app_timer_timeout_handler_t func;
    /**
     * This member is to make sure that timer function is only called if timer is running.
     * FreeRTOS may have timer running even after stop function is called,
     * because it processes commands in Timer task and stopping function only puts command into the queue. */
    bool active;
    bool single_shot;
} app_timer_info_t;


/* Check if app_timer_t variable type can held our app_timer_info_t structure */
STATIC_ASSERT(sizeof(app_timer_info_t) <= sizeof(app_timer_t));


/**@brief Function for returning the current value of the RTC1 counter.
 *
 * @return     Current value of the RTC1 counter.
 */
static __INLINE uint32_t rtc1_counter_get(void)
{
    return NRF_RTC1->COUNTER;
}

/**@brief Function for computing the difference between two RTC1 counter values.
 *
 * @return     Number of ticks elapsed from ticks_old to ticks_now.
 */
static __INLINE uint32_t ticks_diff_get(uint32_t ticks_now, uint32_t ticks_old)
{
    return ((ticks_now - ticks_old) & MAX_RTC_COUNTER_VAL);
}

/**
 * @brief Internal callback function for the system timer
 *
 * Internal function that is called from the system timer.
 * It gets our parameter from timer data and sends it to user function.
 * @param[in] xTimer Timer handler
 */
static void app_timer_callback(app_timer_info_t *pinfo)
{
    ASSERT(pinfo->func != NULL);

    if (pinfo->active)
    {
        pinfo->active = (pinfo->single_shot) ? false : true;
        pinfo->func(pinfo->argument);
    }
}

uint32_t app_timer_init(void)
{
    return NRF_SUCCESS;
}

uint32_t app_timer_create(app_timer_id_t const *p_timer_id,
                          app_timer_mode_t mode,
                          app_timer_timeout_handler_t timeout_handler)
{
    app_timer_info_t *pinfo = (app_timer_info_t *)(*p_timer_id);
    uint32_t err_code = NRF_SUCCESS;

    if ((timeout_handler == NULL) || (p_timer_id == NULL))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (pinfo->active)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (pinfo->osHandle == NULL)
    {
        /* New timer is created */ 
        memset(pinfo, 0, sizeof(app_timer_info_t));

        pinfo->single_shot = (mode == APP_TIMER_MODE_SINGLE_SHOT);
        pinfo->func = timeout_handler;
        pinfo->osHandle = os_timer_create(" ", (void (*)(void *))app_timer_callback, pinfo, 1000,
                                          ((mode == APP_TIMER_MODE_SINGLE_SHOT) ? OS_TIMER_FLAG_ONE_SHOT : OS_TIMER_FLAG_PERIODIC));

        if (pinfo->osHandle == NULL)
            err_code = NRF_ERROR_NULL;
    }
    else
    {
        /* Timer cannot be reinitialized using FreeRTOS API */
        return NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t app_timer_start(app_timer_id_t timer_id, uint32_t timeout_ticks, void *p_context)
{
    app_timer_info_t *pinfo = (app_timer_info_t *)(timer_id);
    os_timer_t *hTimer = pinfo->osHandle;

    if (hTimer == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (pinfo->active)
    {
        // Timer already running - exit silently
        return NRF_SUCCESS;
    }

    pinfo->argument = p_context;

    os_tick_t init_tick = timeout_ticks;
    
    pinfo->active = false;

    os_timer_stop(hTimer);
    os_timer_set_timeout_ticks(hTimer, init_tick);
    os_timer_start(hTimer);

    pinfo->active = true;
    return NRF_SUCCESS;
}

uint32_t app_timer_stop(app_timer_id_t timer_id)
{
    app_timer_info_t *pinfo = (app_timer_info_t *)(timer_id);
    os_timer_t *hTimer = pinfo->osHandle;
    if (hTimer == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    os_timer_stop(hTimer);

    pinfo->active = false;
    return NRF_SUCCESS;
}

uint32_t app_timer_cnt_get(void)
{
    return rtc1_counter_get();
}

uint32_t app_timer_cnt_diff_compute(uint32_t   ticks_to,
                                    uint32_t   ticks_from)
{
    return ticks_diff_get(ticks_to, ticks_from);
}

#endif //NRF_MODULE_ENABLED(APP_TIMER)

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
 * @file        cmsis_kernel.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 event function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-02   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_version.h>
#include <os_clock.h>
#include <os_tick.h>
#include <string.h>
#include <board.h>
#include <arch_interrupt.h>

#include "cmsis_internal.h"

/**
 ***********************************************************************************************************************
 * @def         API_VERSION
 *
 * @brief       CMSIS API version (2.1.2)
 ***********************************************************************************************************************
 */
#define API_VERSION             20010002

/**
 ***********************************************************************************************************************
 * @def         KERNEL_Id
 *
 * @brief       OneOS Kernel identification string
 ***********************************************************************************************************************
 */
#define KERNEL_Id               "OneOS"

static osKernelState_t kernel_state = osKernelInactive;

#ifdef SysTick

static volatile uint8_t          blocked;
static uint8_t                   pendSV;

__STATIC_INLINE uint8_t GetPendSV (void)
{
    return ((uint8_t)((SCB->ICSR & (SCB_ICSR_PENDSVSET_Msk)) >> 24));
}

__STATIC_INLINE void ClrPendSV (void)
{
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;
}

__STATIC_INLINE void SetPendSV (void)
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

static void KernelBlock (void)
{

    OS_Tick_Disable();

    blocked = 1U;
    __DSB();

    if (GetPendSV() != 0U)
    {
        ClrPendSV();
        pendSV = 1U;
    }
}

static void KernelUnblock (void)
{

    blocked = 0U;
    __DSB();

    if (pendSV != 0U)
    {
        pendSV = 0U;
        SetPendSV();
    }

    OS_Tick_Enable();
}

#endif  /*SysTick*/

osStatus_t osKernelInitialize(void)
{

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }
    /* oneos has init all kernel module at init phase, so just set variable here */
    kernel_state = osKernelReady;

    return osOK;
}

static uint32_t os_version_show()
{
    os_uint32_t version_size;
    os_uint16_t os_version;
    os_uint16_t os_subversion;
    os_uint16_t os_reversion;
    os_uint32_t os_kernel_version;

    char version_name[20];

    version_size = sizeof(ONEOS_VERSION);

    strncpy(version_name, ONEOS_VERSION, version_size);

    /*ASCII exchange to dec*/
    os_version    = version_name[7]  - 48;
    os_subversion = version_name[9]  - 48;
    os_reversion  = version_name[11] - 48;

    os_kernel_version =  (((os_uint32_t)os_version      * 10000000UL)     | \
                          ((os_uint32_t)os_subversion   *    10000UL)     | \
                          ((os_uint32_t)os_reversion    *        1UL));

    return (uint32_t)os_kernel_version;

}

osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
    if ((OS_NULL == version) || (OS_NULL == id_buf) || id_size < sizeof(KERNEL_Id))
    {
        return osErrorParameter;
    }

    version->api    = API_VERSION;
    version->kernel = os_version_show();

    id_size = sizeof(KERNEL_Id);
    strncpy(id_buf, KERNEL_Id, id_size);

    return osOK;
}

osKernelState_t osKernelGetState(void)
{
    return kernel_state;
}

osStatus_t osKernelStart(void)
{
    osStatus_t state;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    /* oneos has start schedule at init phase, so just set variable here */
    if (osKernelReady == kernel_state)
    {
        kernel_state = osKernelRunning;

        state = osOK;
    }
    else
    {
        state = osError;
    }

    return state;
}

int32_t osKernelLock(void)
{
    int32_t lock;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    switch (kernel_state)
    {
    case osKernelRunning:
        os_schedule_lock();
        kernel_state = osKernelLocked;
        lock = 0;
        break;
    case osKernelLocked:
        lock = 1;
        break;
    default:
        lock = (int32_t)osError;
        break;
    }

    return lock;
}

int32_t osKernelUnlock(void)
{
    int32_t lock;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    switch (kernel_state)
    {
    case osKernelRunning:
        lock = 0;
        break;
    case osKernelLocked:
        kernel_state = osKernelRunning;
        os_schedule_unlock();
        lock = 1;
        break;
    default:
        lock = (int32_t)osError;
        break;
    }
    return lock;
}

int32_t osKernelRestoreLock(int32_t lock)
{
    int32_t lock_new;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    switch (kernel_state)
    {
    case osKernelRunning:
    case osKernelLocked:
        switch (lock)
        {
        case 0:
            kernel_state = osKernelRunning;
            if (os_is_schedule_locked())
            {
                os_schedule_unlock();
            }
            lock_new = 0;
            break;
        case 1:
            kernel_state = osKernelLocked;
            if (!os_is_schedule_locked())
            {
                os_schedule_lock();
            }
            lock_new = 1;
            break;
        default:
            lock_new = (int32_t)osError;
        break;
        }
        break;
    default:
        lock_new = (int32_t)osError;
        break;
    }

    return lock_new;
}

uint32_t osKernelGetTickCount(void)
{
    return (uint32_t)os_tick_get();
}

uint32_t osKernelGetTickFreq(void)
{

    return OS_TICK_PER_SECOND;
}


#ifdef SysTick

uint32_t osKernelGetSysTimerCount(void)
{
    os_ubase_t  irqmask ;
    uint32_t  val;
    os_tick_t ticks;

    irqmask = os_irq_lock();

    ticks = os_tick_get();
    val   = OS_Tick_GetCount();

    if (OS_Tick_GetOverflow() != 0U)
    {
        val = OS_Tick_GetCount();
        ticks++;
    }

    val += ticks * OS_Tick_GetInterval();
    os_irq_unlock(irqmask);

    return (val);
}

#endif    /*SysTick*/


uint32_t osKernelGetSysTimerFreq(void)
{
    return SystemCoreClock;
}

#ifdef SysTick

uint32_t osKernelSuspend (void)
{

    os_tick_t              min_tick = 0;

	if (kernel_state != osKernelRunning)
	{
		return 0U;
	}

    KernelBlock();

#ifdef OS_USING_TICKLESS_LPMGR

    min_tick = os_tickless_get_sleep_ticks();

#endif

    kernel_state = osKernelSuspended;

    return (min_tick);
}
#endif /* SysTick */

#ifdef SysTick

void osKernelResume (uint32_t sleep_ticks)
{
    if (kernel_state != osKernelSuspended)
    {
        return;
    }

#ifdef OS_USING_TICKLESS_LPMGR

    os_tickless_update(sleep_ticks);

#endif

    kernel_state = osKernelRunning;

    KernelUnblock();

    return;
}
#endif /* SysTick */

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
 * @file        os_spinlock.h
 *
 * @brief       Header file for spinlock interface.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-08   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __OS_SPINLOCK_H__
#define __OS_SPINLOCK_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_util.h>

#ifdef OS_USING_SMP
#include <os_arch_spinlock.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct os_spinlock
{
#ifdef OS_USING_SMP
	os_arch_spinlock_t  lock;
#endif

#ifdef OS_USING_SPINLOCK_CHECK
	os_int32_t          owner_cpu;
	void               *owner;
#endif

#if !defined(OS_USING_SMP) && !defined(OS_USING_SPINLOCK_CHECK)
	os_uint8_t          padding;
#endif
};
typedef struct os_spinlock os_spinlock_t;


#ifdef OS_USING_SMP
#define SPIN_LOCK_INIT                                              \
		.lock = SPINLOCK_ARCH_UNLOCKED,
#else
#define SPIN_LOCK_INIT
#endif


#ifdef OS_USING_SPINLOCK_CHECK
#define SPINLOCK_DEBUG_INIT                                         \
		.owner_cpu  = -1,                                           \
		.owner      = OS_NULL,
#else
#define SPINLOCK_DEBUG_INIT
#endif

#if !defined(OS_USING_SMP) && !defined(OS_USING_SPINLOCK_CHECK)
#define	SPINLOCK_PADDING_INIT                                       \
		.padding = 0,
#else
#define	SPINLOCK_PADDING_INIT
#endif

#define OS_DEFINE_SPINLOCK(var) struct os_spinlock var =            \
{                                                                   \
    SPIN_LOCK_INIT                                                  \
    SPINLOCK_DEBUG_INIT 				                            \
    SPINLOCK_PADDING_INIT                                           \
}

extern void os_spin_lock_init(os_spinlock_t *lock);
extern void os_spin_lock(os_spinlock_t *lock);
extern void os_spin_unlock(os_spinlock_t *lock);
extern void os_spin_lock_irqsave(os_spinlock_t *lock, os_ubase_t *irqsave);
extern void os_spin_unlock_irqrestore(os_spinlock_t *lock, os_ubase_t irqsave);

#ifdef __cplusplus
}
#endif

#endif /* __OS_SPINLOCK_H__ */


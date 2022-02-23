/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        drv_lptim.h
 *
 * \@brief		 This file implements low power timer driver for stm32.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_PMTIMER_H__
#define __DRV_PMTIMER_H__

#include <os_task.h>

os_uint32_t lptim_get_countfreq(void);
os_uint32_t lptim_get_tick_max(void);
os_uint32_t lptim_get_current_tick(void);

os_err_t lptim_start(os_uint32_t reload);
void     lptim_stop(void);

#endif /* __DRV_PMTIMER_H__ */

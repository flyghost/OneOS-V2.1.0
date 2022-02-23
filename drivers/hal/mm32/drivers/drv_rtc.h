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
 * @file        drv_rtc.h
 *
 * @brief        This file provides functions declaration for rtc driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
 
#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__

#include <device.h>
#include "types.h"

struct mm32_rtc_info 
{
    void *      hrtc;
    os_uint32_t rtc_clk;
    void(*rcc_init_func)(os_uint32_t x, FunctionalState y);
};


#endif /* __DRV_USART_H__ */
 
 
 

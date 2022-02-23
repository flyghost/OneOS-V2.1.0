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
 * @file        drv_pwm.h
 *
 * @brief       This file provides functions declaration for STM32 pwm driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_PWM_H__
#define __DRV_PWM_H__

#include <board.h>
#include <drv_hwtimer.h>
#include <os_task.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stm32_pwm
{
    struct os_pwm_device pwm;
    struct stm32_timer *tim;
    os_uint32_t freq;
    os_uint8_t  channel;
    
    os_uint32_t tim_mult;   
    os_uint32_t tim_shift;
};

os_err_t stm32_pwm_register(const char *name, struct stm32_timer *tim);

#ifdef __cplusplus
}
#endif
\

#endif

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
 * @file        drv_pwm.c
 *
 * @brief       This file implements PWM driver for nrf.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************/
 
#ifndef __DRV_PWM_H__
#define __DRV_PWM_H__

#ifdef NRF52832_XXAA

#ifdef BSP_USING_PWM0
#define BSP_USING_PWM0_CH0 17
#define BSP_USING_PWM0_CH1 18
#define BSP_USING_PWM0_CH2 19
#define BSP_USING_PWM0_CH3 20
#endif
#ifdef BSP_USING_PWM1
#define BSP_USING_PWM1_CH0 17
#define BSP_USING_PWM1_CH1 18
#define BSP_USING_PWM1_CH2 19
#define BSP_USING_PWM1_CH3 20
#endif
#ifdef BSP_USING_PWM2
#define BSP_USING_PWM2_CH0 17
#define BSP_USING_PWM2_CH1 18
#define BSP_USING_PWM2_CH2 19
#define BSP_USING_PWM2_CH3 20
#endif

#elif NRF52840_XXAA

#ifdef BSP_USING_PWM0
#define BSP_USING_PWM0_CH0 17
#define BSP_USING_PWM0_CH1 18
#define BSP_USING_PWM0_CH2 19
#define BSP_USING_PWM0_CH3 20
#endif
#ifdef BSP_USING_PWM1
#define BSP_USING_PWM1_CH0 17
#define BSP_USING_PWM1_CH1 18
#define BSP_USING_PWM1_CH2 19
#define BSP_USING_PWM1_CH3 20
#endif
#ifdef BSP_USING_PWM2
#define BSP_USING_PWM2_CH0 17
#define BSP_USING_PWM2_CH1 18
#define BSP_USING_PWM2_CH2 19
#define BSP_USING_PWM2_CH3 20
#endif

#endif

void nrf52_pwm_init(void);

#endif

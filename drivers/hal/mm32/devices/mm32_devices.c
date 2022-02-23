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
 * @file        mm32_devices.c
 *
 * @brief       This file implements device info for mm32; 
 *              peripherals files in board folder; 
 *              peripherals_demo provide some config demo of peripherals files;
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "board.h"
#include "drv_cfg.h"
#include "drv_common.h"
#include "mm32_hal.h"

#include "oneos_config.h"
#include "drv_uart.h"
#include "drv_hwtimer.h"
#include "drv_adc.h"

/***
The following files are configured with default parameters, 
and you can add peripheral information under the project 
without selecting specific peripherals.
such as: select BSP_USING_UART but not not BSP_USING_UART1/BSP_USING_UART2

#ifdef BSP_USING_UART
#include "dev_uart.c"
#endif
#ifdef BSP_USING_ADC
#include "dev_adc.c"
#endif
#ifdef BSP_USING_I2C
#include "dev_i2c.c"
#endif
#ifdef BSP_USING_ONCHIP_RTC
#include "dev_rtc.c"
#endif
#ifdef BSP_USING_TIM
#include "dev_tim.c"
#endif
***/

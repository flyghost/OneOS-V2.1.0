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
 * @file        drv_common.h
 *
 * @brief       This file provides _Error_Handler() declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_COMMON_H__
#define __DRV_COMMON_H__

#include <board.h>
#include "fsl_device_registers.h"

#include "board_config.h"

#include "fsl_cache.h"
#include "fsl_clock.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$ARM_LIB_STACK$$ZI$$Base;
extern int Image$$ARM_LIB_STACK$$ZI$$Limit;
extern uint32_t __Vectors[];

#define COTEX_M_VECTORS_BASE  __Vectors
#define COTEX_M_STACK_END     Image$$ARM_LIB_STACK$$ZI$$Limit
#elif defined(__ICCARM__) || defined(__ICCRX__)
#error: not support iar
#elif defined(__GNUC__)
extern uint32_t __isr_vector[];
extern int _estack;

#define COTEX_M_VECTORS_BASE  __isr_vector
#define COTEX_M_STACK_END     _estack
#endif

#ifdef BSP_USING_GPIO
#include "fsl_gpio.h"
#include "drv_gpio.h"
#endif

#ifdef BSP_USING_LPUART
#include "fsl_lpuart.h"
#include "fsl_lpuart_edma.h"
#include "drv_uart.h"
#endif

#ifdef BSP_USING_CAN
#include "fsl_flexcan.h"
#include "drv_can.h"
#endif

#ifdef BSP_USING_GPT
#include "fsl_gpt.h"
#include "drv_hwtimer.h"
#endif

#ifdef BSP_USING_LCDIF
#include "fsl_elcdif.h"
#include "drv_lcd.h"
#endif

#ifdef BSP_USING_ADC
#include "fsl_adc.h"
#include "drv_adc.h"
#endif

#ifdef BSP_USING_ADC_ETC
#include "fsl_adc_etc.h"
#include "drv_adc.h"
#endif

#ifdef BSP_USING_RTC
#include "fsl_snvs_hp.h"
#include "drv_rtc.h"
#endif

#ifdef BSP_USING_RTC_LP
#include "fsl_snvs_lp.h"
#include "drv_rtc_lp.h"
#endif

#ifdef BSP_USING_WDOG
#include "fsl_wdog.h"
#include "drv_wdg.h"
#endif

#ifdef BSP_USING_RTWDOG
#include "fsl_rtwdog.h"
#include "drv_rtwdg.h"
#endif

#ifdef BSP_USING_PWM
#include "fsl_pwm.h"
#include "drv_pwm.h"
#endif

#ifdef BSP_USING_LPSPI
#include "fsl_lpspi.h"
#include "fsl_lpspi_edma.h"
#include "drv_spi.h"
#endif

#endif

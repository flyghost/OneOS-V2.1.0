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
 * @file        drv_adc.h
 *
 * @brief       This file implements adc driver declaration for mm32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#include "mm32_hal.h"
#include "mm32_it.h"

#include <os_types.h>

#define ADC_PIN_SET(PORTx, index, time)  \
{.gpio = GPIO_Pin_##index, .port = GPIO##PORTx, .gpio_clk = RCC_AHBENR_GPIO##PORTx, .channel = ADC_Channel_##index, .sample_time = time}

typedef void (*RCC_PeriphClockCmd_ADC)(u32 ahb_periph, FunctionalState state);

struct mm32_adc_pin
{
    os_uint16_t             gpio;
    GPIO_TypeDef           *port;
    os_uint32_t             gpio_clk;
    os_uint32_t             channel;
    os_uint32_t             sample_time;
};

struct mm32_adc_info
{
    ADC_TypeDef                *hadc;
    ADC_InitTypeDef             init_struct;
    os_uint32_t                 adc_rcc_clk;
    RCC_PeriphClockCmd_ADC      adc_rcc_clkcmd;
    RCC_PeriphClockCmd_ADC      gpio_rcc_clkcmd;
    os_int32_t                  ref_low;
    os_int32_t                  ref_high;
    const struct mm32_adc_pin  *pin;
    os_uint32_t                 pin_num;
};

#endif /* __DRV_ADC_H__ */





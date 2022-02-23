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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_adc.h"

#ifdef BSP_USING_ADC
static const struct fm33_adc_info adc_info =
{
    .clk = 
        {
            .operationSource                  = FL_ADC_CLK_SOURCE_ADCCLK,
            .clockSource                      = FL_CMU_ADC_CLK_SOURCE_RCHF,
            .clockPrescaler                   = FL_CMU_ADC_PSC_DIV8,
            .APBClockPrescaler                = FL_ADC_APBCLK_PSC_DIV1,
            .referenceSource                  = FL_ADC_REF_SOURCE_VDDA,
            .bitWidth                         = FL_ADC_BIT_WIDTH_12B,
        },
    .init = 
        {
            .conversionMode                   = FL_ADC_CONV_MODE_SINGLE,
            .autoMode                         = FL_ADC_SINGLE_CONV_MODE_AUTO,
            .waitMode                         = FL_ENABLE,
            .overrunMode                      = FL_ENABLE,
            .scanDirection                    = FL_ADC_SEQ_SCAN_DIR_FORWARD,
            .externalTrigConv                 = FL_ADC_TRIGGER_EDGE_NONE,
            .triggerSource                    = FL_ADC_TRGI_LUT0,
            .fastChannelTime                  = FL_ADC_FAST_CH_SAMPLING_TIME_2_ADCCLK,
            .lowChannelTime                   = FL_ADC_SLOW_CH_SAMPLING_TIME_192_ADCCLK,
            .oversamplingMode                 = FL_ENABLE,
            .overSampingMultiplier            = FL_ADC_OVERSAMPLING_MUL_8X,
            .oversamplingShift                = FL_ADC_OVERSAMPLING_SHIFT_3B,
        },
};
OS_HAL_DEVICE_DEFINE("ADC_Type", "adc", adc_info);
#endif


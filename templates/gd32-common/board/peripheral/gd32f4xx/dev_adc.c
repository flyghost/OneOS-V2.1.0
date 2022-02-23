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

#ifdef BSP_USING_ADC0
struct gd32_adc_info adc0_info = {"adc", ADC0};
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc", adc0_info);
#endif

#ifdef BSP_USING_ADC1
struct gd32_adc_info adc1_info = {"adc1", ADC1};
OS_HAL_DEVICE_DEFINE("ADC_HandleTypeDef", "adc1", adc1_info);
#endif


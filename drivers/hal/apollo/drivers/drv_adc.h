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
 * @brief       This file provides operation functions declaration for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ADC_H_
#define __ADC_H_

#include <os_types.h>

int        os_hw_adc_init(void);
os_uint8_t am_adc_data_get(os_uint8_t channel, os_int16_t *buff, os_uint16_t size);
void       am_adc_start(os_uint8_t channel);
void       am_adc_stop(os_uint8_t channel);

#endif /* __ADC_H_ */

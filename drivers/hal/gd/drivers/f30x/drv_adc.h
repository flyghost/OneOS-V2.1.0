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
 * @file        drv_wdt.h
 *
 * @brief       The head file of watchdog driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_ADC__
#define __DRV_ADC__

#include <drv_cfg.h>

typedef struct __ADC_HandleTypeDef
{
  os_uint32_t        channel;
}ADC_HandleTypeDef;


/* gd32 config class */
struct gd32_adc_info
{
    const char *name;
    os_uint32_t adc_periph;
};

#endif


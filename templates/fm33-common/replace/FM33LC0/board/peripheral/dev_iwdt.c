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
 * @file        dev_usart.c
 *
 * @brief       This file define the information of iwdt device
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "drv_iwdt.h"

#ifdef BSP_USING_WDT
struct fm33_iwdg_info iwdt_info = {
    .instance           = IWDT,

    .init = {
        .overflowPeriod = FL_IWDT_PERIOD_125MS,
        .iwdtWindows    = 0,
     },
};
OS_HAL_DEVICE_DEFINE("IWDG_Type", "iwdg", iwdt_info);
#endif


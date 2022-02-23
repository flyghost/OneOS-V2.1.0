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
 * @file        dev_lptim.c
 *
 * @brief       This file implements lptim driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_lptim.h>

#ifdef BSP_USING_LPTIM0
static const struct hc32_lptim_info lptim0_info =
{
    .name = "lptim0",
    .base = M0P_LPTIMER0,
    .prescale = 6,
};
OS_HAL_DEVICE_DEFINE("LPTIMER_Type", "lptim0", lptim0_info);
#endif

#ifdef BSP_USING_LPTIM1
static const struct hc32_lptim_info lptim1_info =
{
    .name = "lptim1",
    .base = M0P_LPTIMER1,
    .prescale = 6,
};
OS_HAL_DEVICE_DEFINE("LPTIMER_Type", "lptim1", lptim1_info);
#endif

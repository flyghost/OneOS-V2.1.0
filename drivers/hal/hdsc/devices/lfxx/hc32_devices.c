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
 * @file        hc32_devices.c
 *
 * @brief       This file implements devices for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_cfg.h"
#include "drv_common.h"

#ifdef BSP_USING_LPUART
#include "dev_lpuart.c"
#endif

#ifdef BSP_USING_UART
#include "dev_usart.c"
#endif

#ifdef OS_USING_DAC
#include "dev_dac.c"
#endif

#ifdef OS_USING_RTC
#include "dev_rtc.c"
#endif

#ifdef OS_USING_FAL
#include "ports/flash_info.c"
#endif

#ifdef OS_USING_WDG
#include "dev_wdg.c"
#endif

#ifdef OS_USING_CLOCKEVENT
#include "dev_lptim.c"
#endif

#ifdef OS_USING_TIMER_DRIVER
#include "dev_hwtimer.c"
#endif

#ifdef OS_USING_SPI
#include "dev_spi.c"
#endif

#ifdef OS_USING_I2C
#include "dev_i2c.c"
#endif

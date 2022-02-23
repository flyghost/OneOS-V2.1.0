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
 * @file        drv_lcd.h
 *
 * @brief       This file implements lcd driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_LCD_H__
#define __DRV_LCD_H__

#include <os_task.h>
#include <device.h>

#include "peripherals.h"

#define LCDIF_config LCDIF_rgbConfig
struct nxp_lcdif_info {
    LCDIF_Type *uart_base;
    const elcdif_rgb_mode_config_t *config;
};


#endif

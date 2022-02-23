/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        drv_uart.h
 *
 * \@brief       This file provides operation functions declaration for IWDT.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_IWDT_H_
#define __DRV_IWDT_H_

#include "fm33lc0xx.h"
#include "fm33lc0xx_fl_iwdt.h"

struct fm33_iwdg_info
{
    IWDT_Type             *instance;
    FL_IWDT_InitTypeDef    init;
};

struct fm33_iwdg
{
    os_watchdog_t          wdg;
    struct fm33_iwdg_info *info;
};

#endif /* __DRV_IWDT_H_ */

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
 * @file        hk32_devicess.c
 *
 * @brief       This file implements driver cfg
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_cfg.h"
#include "drv_common.h"
#include "dev_usart.c"
#include "dev_timer.c"

#ifdef  OS_USING_FAL
#include "ports/flash_info.c"
#endif

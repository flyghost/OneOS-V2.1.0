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
 * @file        drv_lpmgr.c
 *
 * @brief       This file implements low power manager for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>
#include <hc_lpm.h>

static os_err_t lpm_sleep(lpmgr_sleep_mode_e mode)
{

    switch (mode)
    {
    case SYS_SLEEP_MODE_NONE:
        break;

    case SYS_SLEEP_MODE_IDLE:
        break;

    case SYS_SLEEP_MODE_LIGHT:
        Lpm_GotoSleep(FALSE);
        break;

    case SYS_SLEEP_MODE_DEEP:
        Lpm_GotoDeepSleep(FALSE);
        break;

    default:
        OS_ASSERT(0);
    }
    return OS_EOK;
}

static int drv_lpmgr_hw_init(void)
{
    /* system lpmgr module */
    os_lpmgr_init(lpm_sleep);

    return 0;
}

OS_PREV_INIT(drv_lpmgr_hw_init, OS_INIT_SUBLEVEL_MIDDLE);

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
 * @file        remote_control_atk_rx.c
 *
 * @brief       remote_control_atk_rx
 *
 * @details     remote_control_atk_rx
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_clock.h>
#include <shell.h>
#include <string.h>
#include <infrared/infrared.h>
#include "remote_control_atk.h"

#define DBG_EXT_TAG "remote_control_atk"
#define DBG_EXT_LVL DBG_EXT_DEBUG
#include <dlog.h>

static struct os_infrared_device remote_control_atk_dev;

static int remote_control_atk_init(void)
{
    memset(&remote_control_atk_dev, 0, sizeof(remote_control_atk_dev));

#ifdef BSP_USING_RMT_CTL_ATK_TX
    remote_control_atk_dev.tx_pin = BSP_USING_RMT_CTL_ATK_TX_PIN;
#else
    remote_control_atk_dev.tx_pin = OS_INFRARED_INVALIDE_PIN;
#endif

#ifdef BSP_USING_RMT_CTL_ATK_RX
    remote_control_atk_dev.rx_pin = BSP_USING_RMT_CTL_ATK_RX_PIN;
#else
    remote_control_atk_dev.rx_pin = OS_INFRARED_INVALIDE_PIN;
#endif

    os_infrared_register_device("atk_rmt", &remote_control_atk_dev);

    return 0;
}

OS_INIT_EXPORT(remote_control_atk_init, "4", OS_INIT_SUBLEVEL_LOW);


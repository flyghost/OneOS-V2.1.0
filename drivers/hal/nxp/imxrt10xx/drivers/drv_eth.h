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
 * @file        drv_rtc.c
 *
 * @brief       This file implements rtc driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_ETH_H__
#define DRV_ETH_H__

#include "fsl_phy.h"
#include "fsl_enet_mdio.h"
#include "fsl_phyksz8081.h"

struct nxp_eth_info
{
    ENET_Type              *base;
    uint32_t                phyAddr;
    const phy_operations_t *ops;

    enet_buffer_config_t   *buffConfig;
    os_base_t               reset_pin;
    os_base_t               int_pin;
};

#endif

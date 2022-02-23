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
 * @file        drv_can.h
 *
 * @brief       This file implements adc driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2021-8-27   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_CAN_H__
#define DRV_CAN_H__

#include <os_types.h>
#include "fsl_flexcan.h"
#include "peripherals.h"

#define CAN_INFO_INIT(_IMXRT_CAN_, __index)                                                    \
    _IMXRT_CAN_->clk_src = USART##__index##_CLOCK_SOURCE;                                       \
    _IMXRT_CAN_->usart_handle = &USART##__index##_handle;                                       \
    _IMXRT_CAN_->usart_handle->callback = lpc_usart_transfer_callback;                          \
    _IMXRT_CAN_->usart_handle->userData = _NXP_USART_;                                          \
    

struct nxp_can_info
{
    CAN_Type               *can_base;
    const flexcan_config_t *config;
};



#endif /* DRV_CAN_H__ */

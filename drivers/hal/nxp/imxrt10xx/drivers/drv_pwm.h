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
 * @file        drv_pwm.h
 *
 * @brief       This file implements pwm driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_PWM_H__
#define DRV_PWM_H__

#include "peripherals.h"

#ifdef PWM1_PERIPHERAL
#define PWM1_config PWM1_SM0_config
#endif

#ifdef PWM2_PERIPHERAL
#define PWM2_config PWM2_SM0_config
#endif

#ifdef PWM3_PERIPHERAL
#define PWM3_config PWM3_SM0_config
#endif

#ifdef PWM4_PERIPHERAL
#define PWM4_config PWM4_SM0_config
#endif

#define IMXRT_PWM_INFO_GET(_PWM_, _PWM_SM_)                                             \
    _PWM_->sm_clk_src_hz                        = _PWM_SM_##_SM_CLK_SOURCE_FREQ_HZ;         \
    _PWM_->sm_clk_count_src_hz                  = _PWM_SM_##_COUNTER_CLK_SOURCE_FREQ_HZ;    \
    _PWM_->sm_clk_count_hz                      = _PWM_SM_##_COUNTER_FREQ_HZ;               \
    _PWM_->sm_config                            = &_PWM_SM_##_config;                       \
    _PWM_->sm_channel_cfg                       = _PWM_SM_##_pwm_function_config;           \
    _PWM_->pwm_submodules[_PWM_SM_].subModule   = _PWM_SM_;                                 \
    _PWM_->pwm_submodules[_PWM_SM_].numOfChnls  = sizeof(_PWM_SM_##_pwm_function_config)/sizeof(pwm_signal_param_t); \

struct nxp_pwm_info
{
    PWM_Type *base;
    const pwm_config_t *config;
};


#endif

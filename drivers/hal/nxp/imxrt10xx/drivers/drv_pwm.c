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
 * @file        drv_pwm.c
 *
 * @brief       This file implements PWM driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>
#include <sys/time.h>
#include <os_clock.h>
#include <rtc/rtc.h>

#include "fsl_common.h"
#include "fsl_xbara.h"
#include "fsl_pwm.h"
#include "drv_pwm.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.pwm"
#include <drv_log.h>

#define IMXRT_PWM_SUBMODULE_NUM             4
#define IMXRT_PWM_SUBMODULE_CHANNEL_NUM     12

struct imxrt_pwm_submodule
{
    pwm_submodule_t             subModule;
    os_uint8_t                  numOfChnls;
};

struct imxrt_pwm_use_param
{
    pwm_channels_t              channel;
    pwm_module_control_t        module_control;
    struct imxrt_pwm_submodule *pwm_submodule;
    pwm_signal_param_t         *channel_cfg;
    pwm_mode_t                  mode;
};

struct os_imxrt_pwm
{
    struct os_pwm_device        pwm;
    struct nxp_pwm_info        *info;
    
    os_uint32_t                 sm_clk_src_hz;
    os_uint32_t                 sm_clk_count_src_hz;
    os_uint32_t                 sm_clk_count_hz_max;
    os_uint32_t                 sm_clk_count_hz;
    pwm_config_t               *sm_config;
    pwm_signal_param_t         *sm_channel_cfg;
    pwm_mode_t                  mode;
        
    struct imxrt_pwm_submodule  pwm_submodules[IMXRT_PWM_SUBMODULE_NUM];

    os_uint32_t                 max_value;

    os_uint32_t                 mult;
    os_uint32_t                 shift;
};

static os_err_t get_channel_use_param(struct os_imxrt_pwm *imxrt_pwm, struct imxrt_pwm_use_param *param, os_uint32_t channel)
{
    os_uint8_t                  i;
    os_uint8_t                  numOfChnls;
    os_uint8_t                  submodule_index;

    if (channel > IMXRT_PWM_SUBMODULE_CHANNEL_NUM)
    {
        LOG_E(DRV_EXT_TAG, "cahnnel %d over max channel %d", channel, IMXRT_PWM_SUBMODULE_CHANNEL_NUM);
        return OS_ERROR;
    }

    submodule_index = channel / 3;
    
    if (imxrt_pwm->pwm_submodules[submodule_index].subModule == 0xFF)
    {
        LOG_E(DRV_EXT_TAG, "pwm submodule %d not support! please config!", submodule_index);
        return OS_ERROR;
    }

    switch(imxrt_pwm->pwm_submodules[submodule_index].subModule)
    {
    case kPWM_Module_0:
        param->module_control = kPWM_Control_Module_0;
    break;
    case kPWM_Module_1:
        param->module_control = kPWM_Control_Module_1;
    break;
    case kPWM_Module_2:
        param->module_control = kPWM_Control_Module_2;
    break;
    case kPWM_Module_3:
        param->module_control = kPWM_Control_Module_3;
    break;
    default:
    break;
    }

    param->mode = imxrt_pwm->mode;
    param->pwm_submodule = &imxrt_pwm->pwm_submodules[submodule_index];

    numOfChnls = imxrt_pwm->pwm_submodules[submodule_index].numOfChnls;

    switch(channel % 3)
    {
    case 0:
        param->channel = kPWM_PwmA;
    break;
    case 1:
        param->channel = kPWM_PwmB;
    break;
    case 2:
        param->channel = kPWM_PwmX;
    break;
    default:
    break;
    }
    
    for (i = 0;i < numOfChnls;i++)
    {
        if (imxrt_pwm->sm_channel_cfg[i].pwmChannel == param->channel)
        {
            param->channel_cfg = &imxrt_pwm->sm_channel_cfg[i];
            break;
        }
    }

    if (i == numOfChnls)
    {
        LOG_E(DRV_EXT_TAG, "pwm channel %d not support! please config!", channel % 3);
        return OS_ERROR;
    }
    
    return OS_EOK;
}


static os_err_t imxrt_pwm_enabled(struct os_pwm_device *dev, os_uint32_t channel, os_bool_t enable)
{
    struct imxrt_pwm_use_param param;

    struct os_imxrt_pwm *imxrt_pwm = (struct os_imxrt_pwm *)dev;

    if (get_channel_use_param(imxrt_pwm, &param, channel) != OS_EOK)
    {
        return OS_ERROR;
    }
    
    if (enable)
    {
        PWM_StartTimer(imxrt_pwm->info->base, param.module_control);
    }
    else
    {
        PWM_StopTimer(imxrt_pwm->info->base, param.module_control);
    }
    
    return OS_EOK;
}

static os_err_t imxrt_pwm_set_period(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t nsec)
{
    os_uint32_t                 i = 0;
    os_uint32_t                 period_hz;
    os_uint32_t                 caculate;
    struct imxrt_pwm_use_param  param;

    struct os_imxrt_pwm *imxrt_pwm = (struct os_imxrt_pwm *)dev;

    if (get_channel_use_param(imxrt_pwm, &param, channel) != OS_EOK)
    {
        return OS_ERROR;
    }

    caculate = (1000000000.0 / imxrt_pwm->sm_clk_src_hz) * imxrt_pwm->max_value * 128;
    if (caculate < nsec)
    {
        LOG_E(DRV_EXT_TAG, "current src clk support period is %d!", caculate);
        return OS_ERROR;
    }
    
    PWM_StopTimer(imxrt_pwm->info->base, param.module_control);
    
    for (i = 0;i < 8;i++)
    {
        caculate = 1000000000.0 / (imxrt_pwm->sm_clk_src_hz / (1 << i)) * imxrt_pwm->max_value;
        if (caculate >= nsec)
        {
            break;
        }
    }
    
    imxrt_pwm->sm_config->prescale = i;
    imxrt_pwm->sm_clk_count_hz_max = imxrt_pwm->sm_clk_count_src_hz / (1 << i) / imxrt_pwm->max_value;
    
    PWM_Init(imxrt_pwm->info->base, param.pwm_submodule->subModule, imxrt_pwm->sm_config);

    imxrt_pwm->sm_clk_count_hz = 1000000000 / nsec;

    calc_mult_shift(&imxrt_pwm->mult, &imxrt_pwm->shift, NSEC_PER_SEC, imxrt_pwm->sm_clk_count_src_hz / (1 << i), imxrt_pwm->max_value * (1 << i) / imxrt_pwm->sm_clk_count_src_hz);

    PWM_SetPwmLdok(imxrt_pwm->info->base, param.module_control, true);

    PWM_SetupPwm(imxrt_pwm->info->base, param.pwm_submodule->subModule, param.channel_cfg, 1, imxrt_pwm->mode, imxrt_pwm->sm_clk_count_hz, imxrt_pwm->sm_clk_count_src_hz);

    PWM_SetPwmLdok(imxrt_pwm->info->base, param.module_control, true);

    return OS_EOK;
}

static os_err_t imxrt_pwm_set_pulse(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t nsec)
{
    os_uint8_t duty;
    os_uint32_t caculate;
    
    struct imxrt_pwm_use_param  param;

    struct os_imxrt_pwm *imxrt_pwm = (struct os_imxrt_pwm *)dev;

    if (get_channel_use_param(imxrt_pwm, &param, channel) != OS_EOK)
    {
        return OS_ERROR;
    }

    caculate = 1000000000 / imxrt_pwm->sm_clk_count_hz;
    if (nsec > caculate)
    {
        LOG_E(DRV_EXT_TAG, "pwm pulse value over range!");
        return OS_ERROR;
    }

    duty = (float)nsec / caculate * 100;
    
    PWM_UpdatePwmDutycycle(imxrt_pwm->info->base, param.pwm_submodule->subModule, param.channel, param.mode, duty);

    PWM_SetPwmLdok(imxrt_pwm->info->base, param.module_control, true);
    
    return OS_EOK;
}

static const struct os_pwm_ops imxrt_pwm_ops =
{
    .enabled = imxrt_pwm_enabled,
    .set_period = imxrt_pwm_set_period,
    .set_pulse = imxrt_pwm_set_pulse,
    .control  = OS_NULL,
};

static os_err_t imxrt_pwm_init(struct os_imxrt_pwm *imxrt_pwm)
{
    os_uint8_t i  = 0;

    XBARA_Init(XBARA1);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1234Fault2);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1234Fault3);

    for(i = 0;i < IMXRT_PWM_SUBMODULE_NUM;i++)
    {
        imxrt_pwm->pwm_submodules[i].subModule = 0xFF;
    }
    
    switch(BSP_PWM_MODE)
    {
    case 0:
        imxrt_pwm->mode = kPWM_CenterAligned;
    break;
    case 1:
        imxrt_pwm->mode = kPWM_SignedCenterAligned;
    break;
    case 2:
        imxrt_pwm->mode = kPWM_EdgeAligned;
    break;
    case 3:
        imxrt_pwm->mode = kPWM_SignedEdgeAligned;
    break;
    default:
    break;
    }
    
    switch((os_uint32_t)imxrt_pwm->info->base)
    {
    case (os_uint32_t)PWM1:
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1Fault0);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm1Fault1);
    
#ifdef PWM1_SM0
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM1_SM0)
#endif

#ifdef PWM1_SM1
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM1_SM1)
#endif

#ifdef PWM1_SM2
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM1_SM2)
#endif

#ifdef PWM1_SM3
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM1_SM3)
#endif
    break;
    case (os_uint32_t)PWM2:
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm2Fault0);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm2Fault1);
        
#ifdef PWM2_SM0
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM2_SM0)
#endif

#ifdef PWM2_SM1
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM2_SM1)
#endif

#ifdef PWM2_SM2
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM2_SM2)
#endif

#ifdef PWM2_SM3
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM2_SM3)
#endif
    break;
    case (os_uint32_t)PWM3:
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm3Fault0);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm3Fault1);
    
#ifdef PWM3_SM0
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM3_SM0)
#endif

#ifdef PWM3_SM1
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM3_SM1)
#endif

#ifdef PWM3_SM2
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM3_SM2)
#endif

#ifdef PWM3_SM3
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM3_SM3)
#endif
    break;
    case (os_uint32_t)PWM4:
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm4Fault0);
        XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicHigh, kXBARA1_OutputFlexpwm4Fault1);
    
#ifdef PWM4_SM0
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM4_SM0)
#endif

#ifdef PWM4_SM1
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM4_SM1)
#endif

#ifdef PWM4_SM2
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM4_SM2)
#endif

#ifdef PWM4_SM3
        IMXRT_PWM_INFO_GET(imxrt_pwm, PWM4_SM3)
#endif
    break;
    default:
    break;
    }
    
    imxrt_pwm->max_value = 0xFFFF;
    imxrt_pwm->sm_clk_count_hz_max = imxrt_pwm->sm_clk_count_src_hz / imxrt_pwm->max_value;
    
    return OS_EOK;
}

os_err_t imxrt_pwm_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct os_imxrt_pwm *imxrt_pwm = os_calloc(1, sizeof(struct os_imxrt_pwm));
    OS_ASSERT(imxrt_pwm);
    
    imxrt_pwm->info = (struct nxp_pwm_info *)dev->info;
    
    imxrt_pwm_init(imxrt_pwm);
    
    imxrt_pwm->pwm.ops = &imxrt_pwm_ops;
    
    calc_mult_shift(&imxrt_pwm->mult, &imxrt_pwm->shift, NSEC_PER_SEC, imxrt_pwm->sm_clk_count_src_hz, imxrt_pwm->max_value / imxrt_pwm->sm_clk_count_src_hz);
   
    os_device_pwm_register(&imxrt_pwm->pwm, dev->name);
    
    return OS_EOK;
}

OS_DRIVER_INFO imxrt_pwm_driver = {
    .name   = "PWM_Type",
    .probe  = imxrt_pwm_probe,
};

OS_DRIVER_DEFINE(imxrt_pwm_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);


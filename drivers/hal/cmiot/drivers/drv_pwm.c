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
 * @brief       This file implements pwm driver for cm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <string.h>
#include <os_memory.h>
#include "drv_pwm.h"

#define DRV_EXT_LVL    DBG_EXT_INFO
#define DRV_EXT_TAG    "drv.pwm"
#include <drv_log.h>

static struct cm32_pwm cm32_pwm_obj[] =
{
#ifdef BSP_USING_PWM1
    {
        .tim_handle              = TIM1,
        .name                    = "pwm1",
    },
#endif

#ifdef BSP_USING_PWM2
    {
        .tim_handle              = TIM2,
        .name                    = "pwm2",
    },
#endif

#ifdef BSP_USING_PWM3
    {
        .tim_handle              = TIM3,
        .name                    = "pwm3",
    },
#endif

#ifdef BSP_USING_PWM4
    {
        .tim_handle              = TIM4,
        .name                    = "pwm4",
    },
#endif
};

OCInitType TIM_OCInitStructure;

static os_err_t cm32_pwm_enabled(struct os_pwm_device *dev, os_uint32_t channel, os_bool_t enable)
{
    struct cm32_pwm *pwm;

    OS_ASSERT(dev != OS_NULL);

    pwm = os_container_of(dev, struct cm32_pwm, parent);

    if (!enable)
    {
        if (channel == 1)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
            TIM_InitOc1(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_EnableOc1Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_DISABLE);
        }
        else if (channel == 2)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
            TIM_InitOc2(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_ConfigOc2Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_DISABLE);
        }
        else if (channel == 3)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
            TIM_InitOc3(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_ConfigOc3Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_DISABLE);
        }
        else if (channel == 4)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
            TIM_InitOc4(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_ConfigOc4Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_DISABLE);
        }
        else
        {
            return OS_ERROR;
        }
    }
    else
    {
        if (channel == 1)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
            TIM_InitOc1(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_EnableOc1Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_ENABLE);
        }
        else if (channel == 2)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
            TIM_InitOc2(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_ConfigOc2Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_ENABLE);
        }
        else if (channel == 3)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
            TIM_InitOc3(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_ConfigOc3Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_ENABLE);
        }
        else if (channel == 4)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
            TIM_InitOc4(pwm->tim_handle, &TIM_OCInitStructure);
            TIM_ConfigOc4Preload(pwm->tim_handle, TIM_OC_PRE_LOAD_ENABLE);
        }
        else
        {
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static os_err_t cm32_pwm_set_period(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t nsec)
{
    struct cm32_pwm *pwm;
    int period = (int)(nsec * 6 / 1000);

    OS_ASSERT(dev != OS_NULL);

    pwm = os_container_of(dev, struct cm32_pwm, parent);

    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    uint16_t PrescalerValue = 0;
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)(SystemCoreClock / 12000000) - 1;
    /* Time base configuration */
    TIM_TimeBaseStructure.Period    = period;
    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(pwm->tim_handle, &TIM_TimeBaseStructure);

    return OS_EOK;
}

static os_err_t cm32_pwm_set_pulse(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t buffer)
{
    struct cm32_pwm *pwm;
    int pulse = (int)(buffer * 6 / 1000);

    OS_ASSERT(dev != OS_NULL);

    pwm = os_container_of(dev, struct cm32_pwm, parent);

    TIM_OCInitStructure.OcMode      = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.Pulse       = pulse;
    TIM_OCInitStructure.OcPolarity  = TIM_OC_POLARITY_HIGH;

    if (channel == 1)
    {
        if (pwm->tim_handle->CCEN && TIM_CCEN_CC1EN > 0)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
        }
        else
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
        }
    }
    else if (channel == 2)
    {
        if (pwm->tim_handle->CCEN && TIM_CCEN_CC2EN > 0)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
        }
        else
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
        }
    }
    else if (channel == 3)
    {
        if (pwm->tim_handle->CCEN && TIM_CCEN_CC3EN > 0)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
        }
        else
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
        }
    }
    else if (channel == 4)
    {
        if (pwm->tim_handle->CCEN && TIM_CCEN_CC4EN > 0)
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
        }
        else
        {
            TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
        }
    }
    else
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static const struct os_pwm_ops cm32_pwm_ops =
{
    .enabled    = cm32_pwm_enabled,
    .set_period = cm32_pwm_set_period,
    .set_pulse  = cm32_pwm_set_pulse,
    .control    = OS_NULL,
};

static os_err_t cm32_hw_pwm_init(struct cm32_pwm *pwm_drv)
{
    os_err_t result = OS_EOK;

    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM3, ENABLE);
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM4, ENABLE);

    /* GPIOA and GPIOB clock enable */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);

    GPIO_InitType GPIO_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);
    /* GPIOA Configuration:TIM3 Channel1, 2, 3 and 4 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Current = GPIO_DC_4mA;

    GPIO_InitStructure.Pin        = GPIO_PIN_0;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_TIM3;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin        = GPIO_PIN_7;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_TIM4;
    GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    uint16_t PrescalerValue = 0;
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)(SystemCoreClock / 12000000) - 1;
    /* Time base configuration */
    TIM_TimeBaseStructure.Period    = 665;
    TIM_TimeBaseStructure.Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);
    TIM_InitTimeBase(TIM4, &TIM_TimeBaseStructure);

    OCInitType TIM_OCInitStructure;
    TIM_OCInitStructure.OcMode      = TIM_OCMODE_PWM1;
    TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_DISABLE;
    TIM_OCInitStructure.Pulse       = 333;
    TIM_OCInitStructure.OcPolarity  = TIM_OC_POLARITY_HIGH;

    TIM_InitOc3(TIM3, &TIM_OCInitStructure);

    TIM_ConfigOc3Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);

    TIM_InitOc2(TIM4, &TIM_OCInitStructure);

    TIM_ConfigOc2Preload(TIM4, TIM_OC_PRE_LOAD_ENABLE);

    TIM_ConfigArPreload(TIM3, ENABLE);

    /* TIM3 enable counter */
    TIM_Enable(TIM3, ENABLE);

    TIM_ConfigArPreload(TIM4, ENABLE);

    /* TIM3 enable counter */
    TIM_Enable(TIM4, ENABLE);

    return result;
}

static int os_hw_pwm_init(void)
{
    os_uint32_t idx = 0;
    int result = OS_EOK;

    for (idx = 0; idx < sizeof(cm32_pwm_obj) / sizeof(cm32_pwm_obj[0]); idx++)
    {
        /* pwm init */
        cm32_hw_pwm_init(&cm32_pwm_obj[idx]);

        /* register pwm device */
        cm32_pwm_obj[idx].parent.ops = &cm32_pwm_ops;
        os_device_pwm_register(&(cm32_pwm_obj[idx].parent), cm32_pwm_obj[idx].name);
    }

    return result;
}

OS_DEVICE_INIT(os_hw_pwm_init, OS_INIT_SUBLEVEL_HIGH);

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
 * @file        drv_iwdg.c
 *
 * @brief       This file implements iwdg driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include "drv_iwdg.h"

#define DBG_TAG "drv.iwdg"
#include <dlog.h>

static os_watchdog_t cm32_watchdog;

os_uint16_t IC1ReadValue1 = 0;
os_uint16_t IC1ReadValue2 = 0;
os_uint16_t CaptureNumber = 0;
os_uint16_t Capture       = 0;
os_uint32_t LsiFreq;
os_uint32_t timeout_max = 26;
os_uint32_t lsi_freq    = 40000;

void TIM9_IRQHandler(void)
{
    if (TIM_GetIntStatus(TIM9, TIM_INT_CC3) != RESET)
    {
        if (CaptureNumber == 0)
        {
            /* Get the Input Capture value */
            IC1ReadValue1 = TIM_GetCap3(TIM9);
        }
        else if (CaptureNumber == 2)
        {
            RCC_ClocksType clks;
            /* Get the Input Capture value */
            IC1ReadValue2 = TIM_GetCap3(TIM9);

            /* Capture computation */
            if (IC1ReadValue2 > IC1ReadValue1)
            {
                Capture = (IC1ReadValue2 - IC1ReadValue1);
            }
            else
            {
                Capture = ((0xFFFF - IC1ReadValue1) + IC1ReadValue2);
            }
            RCC_GetClocksFreqValue(&clks);
            /* Frequency computation */
            LsiFreq = (uint32_t)clks.Pclk1Freq / Capture;
            LsiFreq *= 32;

        }

        CaptureNumber++;

        /* Clear TIM9 Capture compare interrupt pending bit */
        TIM_ClrIntPendingBit(TIM9, TIM_INT_CC3);
    }
}

void TIM9_ConfigForLSI(void)
{
    NVIC_InitType NVIC_InitStructure;
    TIM_ICInitType TIM_ICInitStructure;

    /* Enable TIM9 clocks */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM9, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);

    /* Enable the TIM9 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM9_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure TIM9 prescaler */
    TIM_ConfigPrescaler(TIM9, 0, TIM_PSC_RELOAD_MODE_IMMEDIATE);

    /* Connect internally the TM9_CH3 PB14 Input Capture to the LSI clock output */
    GPIO_ConfigPinRemap(GPIOB_PORT_SOURCE, GPIO_PIN_SOURCE14, GPIO_AF1_TIM9);

    /* TIM9 configuration */
    TIM_ICInitStructure.Channel     = TIM_CH_3;
    TIM_ICInitStructure.IcPolarity  = TIM_IC_POLARITY_RISING;
    TIM_ICInitStructure.IcSelection = TIM_IC_SELECTION_DIRECTTI;
    TIM_ICInitStructure.IcPrescaler = TIM_IC_PSC_DIV8;
    TIM_ICInitStructure.IcFilter    = 0;
    TIM_ICInit(TIM9, &TIM_ICInitStructure);

    TIM9->CTRL1 |= TIM_CTRL1_C3SEL;
    /* TIM9 Counter Enable */
    TIM_Enable(TIM9, ENABLE);

    /* Reset the flags */
    TIM9->STS = 0;

    /* Enable the CC3 Interrupt Request */
    TIM_ConfigInt(TIM9, TIM_INT_CC3, ENABLE);
}

static os_err_t cm32_iwdt_init(os_watchdog_t *wdt)
{
    RCC_EnableLsi(ENABLE);
    while (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_LSIRD) == RESET)
    {
    }

    TIM9_ConfigForLSI();

    //while (CaptureNumber != 3)
    //{
    //}

    /* Disable TIM9 CC3 Interrupt Request */
    TIM_ConfigInt(TIM9, TIM_INT_CC3, DISABLE);

    IWDG_WriteConfig(IWDG_WRITE_ENABLE);

    /* IWDG counter clock: LSI/256 */
    IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV256);

    return OS_EOK;
}

static os_err_t cm32_iwdt_control(os_watchdog_t *wdt, int cmd, void *arg)
{
    os_uint32_t timeout = 0;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_WDT_KEEPALIVE:
        IWDG_ReloadKey();
        return OS_EOK;

    /* set window value */
    case OS_DEVICE_CTRL_WDT_SET_TIMEOUT:
        timeout = *(os_uint32_t *)arg;
        if (timeout > timeout_max)
        {
            break;
        }
        else
        {
            IWDG_CntReload(timeout * lsi_freq / 256);
            return OS_EOK;
        }

    case OS_DEVICE_CTRL_WDT_GET_TIMEOUT:
        break;

    case OS_DEVICE_CTRL_WDT_START:
        IWDG_Enable();

        LOG_I(DBG_TAG, "wdt start.");
        return OS_EOK;
    }

    return OS_ENOSYS;
}

const static struct os_watchdog_ops ops =
{
    .init     = &cm32_iwdt_init,
    .control  = &cm32_iwdt_control,
};

static int cm32_iwdg_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t ret = OS_EOK;

    cm32_watchdog.ops = &ops;

    ret = os_hw_watchdog_register(&cm32_watchdog, dev->name, OS_NULL);

    if (ret != OS_EOK)
    {
        LOG_E(DBG_TAG, "Os device register failed %d\n", ret);
    }

    return ret;
}

OS_DRIVER_INFO cm32_iwdt_driver = {
    .name   = "IWDG_HandleTypeDef",
    .probe  = cm32_iwdg_probe,
};

OS_DRIVER_DEFINE(cm32_iwdt_driver, PREV, OS_INIT_SUBLEVEL_LOW);

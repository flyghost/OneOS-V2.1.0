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
 * @file        drv_adc.c
 *
 * @brief       This file implements adc driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_stddef.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <string.h>
#include <drv_log.h>
#include <drv_adc.h>
#include <hc_gpio.h>
#include <hc_adc.h>
#include <hc_bgr.h>

#define DBG_TAG "drv.adc"
#include <dlog.h>

#define HC32_ADC_MAX_CHANNEL 26

struct adc_channel_info {
    en_gpio_port_t port;
    en_gpio_pin_t pin;
    en_adc_samp_ch_sel_t chn;
};

struct hc32_adc
{
    struct os_adc_device adc;
};

static const struct adc_channel_info ch_info[] = {
    {GpioPortA,  GpioPin0,  AdcExInputCH0},
    {GpioPortA,  GpioPin1,  AdcExInputCH1},
    {GpioPortA,  GpioPin2,  AdcExInputCH2},
    {GpioPortA,  GpioPin3,  AdcExInputCH3},
    {GpioPortA,  GpioPin4,  AdcExInputCH4},
    {GpioPortA,  GpioPin5,  AdcExInputCH5},
    {GpioPortA,  GpioPin6,  AdcExInputCH6},
    {GpioPortA,  GpioPin7,  AdcExInputCH7},
    {GpioPortB,  GpioPin0,  AdcExInputCH8},
    {GpioPortB,  GpioPin1,  AdcExInputCH9},
    {GpioPortC,  GpioPin0, AdcExInputCH10},
    {GpioPortC,  GpioPin1, AdcExInputCH11},
    {GpioPortC,  GpioPin2, AdcExInputCH12},
    {GpioPortC,  GpioPin3, AdcExInputCH13},
    {GpioPortC,  GpioPin4, AdcExInputCH14},
    {GpioPortC,  GpioPin5, AdcExInputCH15},
    {GpioPortB,  GpioPin2, AdcExInputCH16},
    {GpioPortB, GpioPin10, AdcExInputCH17},
    {GpioPortB, GpioPin11, AdcExInputCH18},
    {GpioPortB, GpioPin12, AdcExInputCH19},
    {GpioPortB, GpioPin13, AdcExInputCH20},
    {GpioPortB, GpioPin14, AdcExInputCH21},
    {GpioPortB, GpioPin15, AdcExInputCH22},
    {GpioPortE, GpioPin15, AdcExInputCH23},
    {GpioPortE, GpioPin14, AdcExInputCH24},
    {GpioPortE, GpioPin13, AdcExInputCH25},
};

static os_err_t hc32_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{
    os_uint8_t cnt;

    if(channel >= HC32_ADC_MAX_CHANNEL)
    {
        LOG_E(DBG_TAG, "adc channel %d cannot find!\n", channel);

        return OS_ERROR;
    }

    Adc_CfgSglChannel(ch_info[channel].chn);
    Gpio_SetAnalogMode(ch_info[channel].port, ch_info[channel].pin);

    Adc_SGL_Start();

    while(1)
    {
        if(TRUE == Adc_GetIrqStatus(AdcMskIrqSgl))
        {
            for (cnt = 0; cnt < 5; cnt++)
            {
                Adc_SGL_Start();
                *buff = (os_int32_t)((os_uint64_t)Adc_GetSglResult() * dev->mult >> dev->shift);
            }

            Adc_ClrIrqStatus(AdcMskIrqSgl);
            break;
        }
    }

    Adc_SGL_Stop();

    return OS_EOK;
}

static os_err_t hc32_adc_open(struct os_adc_device *dev)
{
    Adc_SGL_Start();
    return OS_EOK;
}

static os_err_t hc32_adc_close(struct os_adc_device *dev)
{
    Adc_SGL_Stop();

    return OS_EOK;
}

static os_err_t hc32_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    if (!enable)
    {
        return hc32_adc_close(dev);
    }
    else
    {
        return hc32_adc_open(dev);
    }
}

static os_err_t hc32_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static const struct os_adc_ops hc32_adc_ops = {
    .adc_enabled = hc32_adc_enabled,
    .adc_control = hc32_adc_control,
    .adc_read    = hc32_adc_read,
};

static inline void __os_hw_adc_init(void)
{
    stc_adc_cfg_t              stcAdcCfg;

    DDL_ZERO_STRUCT(stcAdcCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralAdcBgr, TRUE);

    Bgr_BgrEnable();

    stcAdcCfg.enAdcMode         = AdcSglMode;
    stcAdcCfg.enAdcClkDiv       = AdcMskClkDiv8;
    stcAdcCfg.enAdcSampCycleSel = AdcMskSampCycle12Clk;
    stcAdcCfg.enAdcRefVolSel    = AdcMskRefVolSelAVDD;
    stcAdcCfg.enAdcOpBuf        = AdcMskBufEnable; /* AdcMskBufDisable; */
    stcAdcCfg.enInRef           = AdcMskInRefDisable;
    stcAdcCfg.enAdcAlign        = AdcAlignRight;
    Adc_Init(&stcAdcCfg);
}

int os_hw_adc_init(void)
{
    os_err_t result = OS_ERROR;
    struct hc32_adc *adc;

    adc = os_calloc(1, sizeof(struct hc32_adc));

    OS_ASSERT(adc);

    __os_hw_adc_init();

    adc->adc.ops       = &hc32_adc_ops;
    adc->adc.max_value = (1UL << 12) - 1;
    adc->adc.ref_low   = 0;                 /* ref 0 - 3.3v */
    adc->adc.ref_hight = 3300;

    result = os_hw_adc_register(&adc->adc, "adc", NULL);

    return result;
}

/* OS_DEVICE_INIT(os_hw_adc_init); */

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
 * @brief       This file implements adc driver for cm32.
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

#define DBG_TAG "drv.adc"
#include <dlog.h>

/* ADC clock selection definition */
#define ADC_CLK_PCLK             (1u)
#define ADC_CLK_MPLLQ            (2u)
#define ADC_CLK_UPLLR            (3u)

/* Select PCLK as ADC clock */
#define ADC_CLK                  ADC_CLK_PCLK

/* ADC resolution definitions */
#define ADC_RESOLUTION_8BIT      (8u)
#define ADC_RESOLUTION_10BIT     (10u)
#define ADC_RESOLUTION_12BIT     (12u)

#define ADC_RESOLUTION           ADC_RESOLUTION_12BIT

enum os_adc_unit
{
    ADC1,
};

struct adc_configs
{
    enum os_adc_unit en_adc_uint;
    ADC_InitType cm_adc_init;
    ADC_InitTypeEx  adc_cfg;
};

struct cm32_adc
{
    struct adc_configs cm_adc;
    struct os_adc_device cm32_adc_device;
};

struct cm32_adc g_adc_context =
{
    .cm_adc = {
        .en_adc_uint = ADC1,
        .cm_adc_init.MultiChEn      = DISABLE,
        .cm_adc_init.ContinueConvEn = DISABLE,
        .cm_adc_init.ExtTrigSelect  = ADC_EXT_TRIGCONV_NONE,
        .cm_adc_init.DatAlign       = ADC_DAT_ALIGN_R,
        .cm_adc_init.ChsNumber      = 1,
    },
};

static void AdcSetPinMode(uint8_t u8AdcPin)
{
    GPIO_Module* enPort;
    uint32_t rcc_APB2Periph;
    uint16_t enPin;
    GPIO_InitType GPIO_InitStructure;

    switch (u8AdcPin)
    {
    case ADC_CH_0:
        /* internal */
        return;

    case ADC_CH_1_PA0:
        enPort = GPIOA;
        enPin  = GPIO_PIN_0;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_2_PA1:
        enPort = GPIOA;
        enPin  = GPIO_PIN_1;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_3_PA2:
        enPort = GPIOA;
        enPin  = GPIO_PIN_2;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_4_PA3:
        enPort = GPIOA;
        enPin  = GPIO_PIN_3;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_5_PA4:
        enPort = GPIOA;
        enPin  = GPIO_PIN_4;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_6_PA5:
        enPort = GPIOA;
        enPin  = GPIO_PIN_5;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_7_PA6:
        enPort = GPIOA;
        enPin  = GPIO_PIN_6;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_8_PA7:
        enPort = GPIOA;
        enPin  = GPIO_PIN_7;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOA;
        break;

    case ADC_CH_9_PB0:
        enPort = GPIOB;
        enPin  = GPIO_PIN_0;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOB;
        break;

    case ADC_CH_10_PB1:
        enPort = GPIOB;
        enPin  = GPIO_PIN_1;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOB;
        break;

    case ADC_CH_11_PC0:
        enPort = GPIOC;
        enPin  = GPIO_PIN_0;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOC;
        break;

    case ADC_CH_12_PC1:
        enPort = GPIOC;
        enPin  = GPIO_PIN_1;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOC;
        break;

    case ADC_CH_13_PC2:
        enPort = GPIOC;
        enPin  = GPIO_PIN_2;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOC;
        break;

    case ADC_CH_14_PC3:
        enPort = GPIOC;
        enPin  = GPIO_PIN_3;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOC;
        break;

    case ADC_CH_15_PC4:
        enPort = GPIOC;
        enPin  = GPIO_PIN_4;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOC;
        break;

    case ADC_CH_16_PC5:
        enPort = GPIOC;
        enPin  = GPIO_PIN_5;
        rcc_APB2Periph = RCC_APB2_PERIPH_GPIOC;
        break;

    case ADC_CH_17:
        /* internal */
        return;

    case ADC_CH_18:
        /* internal */
        return;

    default:
        return;
    }

    RCC_EnableAPB2PeriphClk(rcc_APB2Periph, ENABLE);

    GPIO_InitStruct(&GPIO_InitStructure);

    GPIO_InitStructure.Pin       = enPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Analog;
    GPIO_InitPeripheral(enPort, &GPIO_InitStructure);
}

static void AdcSetChannelPinMode(ADC_Module* ADCx,
                                 uint32_t u32Channel)
{
#if (ADC_CH_REMAP)
    uint8_t u8AdcPin;
#else
    uint8_t u8ChOffset = 0u;
#endif

    if ((NULL == ADCx) || (0u == u32Channel))
    {
        return;
    }

#if (ADC_CH_REMAP)
    u8AdcPin = ADC_GetChannelPinNum(ADCx, u8ChIndex);
    AdcSetPinMode(u8AdcPin, enMode);
#else
    AdcSetPinMode(u32Channel + u8ChOffset);
#endif
}

static void AdcInitConfig(void)
{
#if (ADC_RESOLUTION == ADC_RESOLUTION_8BIT)
    ADC_SetConvResultBitNum(ADC, ADC_RST_BIT_8);
#elif (ADC_RESOLUTION == ADC_RESOLUTION_10BIT)
    ADC_SetConvResultBitNum(ADC, ADC_RST_BIT_10);
#else
    ADC_SetConvResultBitNum(ADC, ADC_RST_BIT_12);
#endif

    ADC_Init(ADC, &(g_adc_context.cm_adc.cm_adc_init));

    /* Enable ADC */
    ADC_Enable(ADC, ENABLE);
    /* Check ADC Ready */
    while(ADC_GetFlagStatusNew(ADC, ADC_FLAG_RDY) == RESET)
        ;
    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC))
        ;
}

static void AdcClockConfig(void)
{
#if (ADC_CLK == ADC_CLK_PCLK)
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

    /* RCC_ADCHCLK_DIV16*/
    ADC_ConfigClk(ADC_CTRL3_CKMOD_AHB, RCC_ADCHCLK_DIV16);
#ifdef USE_HSI_PLL
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSI, RCC_ADC1MCLK_DIV8); /* selsect HSI as RCC ADC1M CLK Source */
#else
    RCC_ConfigAdc1mClk(RCC_ADC1MCLK_SRC_HSE, RCC_ADC1MCLK_DIV8); /* selsect HSE as RCC ADC1M CLK Source */	
#endif

#elif (ADC_CLK == ADC_CLK_MPLLQ)

    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_mpll_cfg_t stcMpllCfg;

    if (CLKSysSrcMPLL == CLK_GetSysClkSource())
    {
        /*
         * Configure MPLLQ(same as MPLLP and MPLLR) when you
         * configure MPLL as the system clock.
         */
    }
    else
    {
        /* Use XTAL as MPLL source. */
        stcXtalCfg.enFastStartup = Enable;
        stcXtalCfg.enMode = ClkXtalModeOsc;
        stcXtalCfg.enDrv  = ClkXtalLowDrv;
        CLK_XtalConfig(&stcXtalCfg);
        CLK_XtalCmd(Enable);

        /* Set MPLL out 240MHz. */
        stcMpllCfg.pllmDiv = 1u;
        /* mpll = 8M / pllmDiv * plln */
        stcMpllCfg.plln    = 30u;
        stcMpllCfg.PllpDiv = 16u;
        stcMpllCfg.PllqDiv = 16u;
        stcMpllCfg.PllrDiv = 16u;
        CLK_SetPllSource(ClkPllSrcXTAL);
        CLK_MpllConfig(&stcMpllCfg);
        CLK_MpllCmd(Enable);
    }
    CLK_SetPeriClkSource(ClkPeriSrcMpllp);

#elif (ADC_CLK == ADC_CLK_UPLLR)

    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_upll_cfg_t stcUpllCfg;

    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcUpllCfg);

    /* Use XTAL as UPLL source. */
    stcXtalCfg.enFastStartup = Enable;
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* Set UPLL out 240MHz. */
    stcUpllCfg.pllmDiv = 2u;
    /* upll = 8M(XTAL) / pllmDiv * plln */
    stcUpllCfg.plln    = 60u;
    stcUpllCfg.PllpDiv = 16u;
    stcUpllCfg.PllqDiv = 16u;
    stcUpllCfg.PllrDiv = 16u;
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_UpllConfig(&stcUpllCfg);
    CLK_UpllCmd(Enable);
    CLK_SetPeriClkSource(ClkPeriSrcUpllr);
#endif
}

static void AdcConfig(void)
{
    AdcClockConfig();
    AdcInitConfig();
}

static os_uint32_t cm32_adc_get_channel( enum os_adc_unit en_adc_uint, os_uint32_t channel)
{
    os_uint32_t cm32_channel = 0xFF;

    if (1)
    {
        switch (channel)
        {
        case  0:
            cm32_channel = ADC_CH_0;
            break;
        case  1:
            cm32_channel = ADC_CH_1;
            break;
        case  2:
            cm32_channel = ADC_CH_2;
            break;
        case  3:
            cm32_channel = ADC_CH_3;
            break;
        case  4:
            cm32_channel = ADC_CH_4;
            break;
        case  5:
            cm32_channel = ADC_CH_5;
            break;
        case  6:
            cm32_channel = ADC_CH_6;
            break;
        case  7:
            cm32_channel = ADC_CH_7;
            break;
        case  8:
            cm32_channel = ADC_CH_8;
            break;
        case  9:
            cm32_channel = ADC_CH_9;
            break;
        case 10:
            cm32_channel = ADC_CH_10;
            break;
        case 11:
            cm32_channel = ADC_CH_11;
            break;
        case 12:
            cm32_channel = ADC_CH_12;
            break;
        case 13:
            cm32_channel = ADC_CH_13;
            break;
        case 14:
            cm32_channel = ADC_CH_14;
            break;
        case 15:
            cm32_channel = ADC_CH_15;
            break;
        case 16:
            cm32_channel = ADC_CH_16;
            break;
        case 17:
            cm32_channel = ADC_CH_17;
            break;
        case 18:
            cm32_channel = ADC_CH_18;
            break;
        default:
            LOG_E(DBG_TAG, "channel num is wrong, please check!");
            break;
        }
    }
    else
    {
        LOG_E(DBG_TAG, "adc type is wrong, please check!");
    }

    return cm32_channel;
}

static os_err_t cm32_adc_enabled(struct os_adc_device *dev, os_bool_t enable)
{
    OS_ASSERT(dev != OS_NULL);

    if (!enable)
    {
        ADC_Enable(ADC, DISABLE);
        return OS_EOK;
    }
    else
    {
        AdcInitConfig();
        return OS_EOK;
    }
}

static os_err_t cm32_adc_control(struct os_adc_device *dev, int cmd, void *arg)
{
    return OS_EOK;
}

static os_err_t cm32_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buff)
{
    os_err_t ret = OS_EOK;
    enum os_adc_unit en_adc_uint;
    uint32_t  u32Channel;
    uint8_t  cnt;

    en_adc_uint = (*(enum os_adc_unit *)(dev->parent.user_data));
    u32Channel  = cm32_adc_get_channel(en_adc_uint, channel);

    if(0xFF == u32Channel)
    {
        return OS_ERROR;
    }

    AdcSetChannelPinMode(ADC, channel);

    ADC_ConfigRegularChannel(ADC, channel, 1, ADC_SAMP_TIME_55CYCLES5);

    for(cnt =0; cnt < 2; cnt++)
    {
        /* Start ADC Software Conversion */
        ADC_EnableSoftwareStartConv(ADC, ENABLE);

        while(ADC_GetFlagStatus(ADC, ADC_FLAG_ENDC) == 0) {
        }

        ADC_ClearFlag(ADC, ADC_FLAG_ENDC);
        *buff = ADC_GetDat(ADC) * 3300 / 4095;
    }

    return ret;
}

static const struct os_adc_ops cm32_adc_ops = {
    .adc_enabled = cm32_adc_enabled,
    .adc_control = cm32_adc_control,
    .adc_read    = cm32_adc_read,
};

static int cm32_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t result = OS_ERROR;

    struct os_adc_device *dev_adc = &g_adc_context.cm32_adc_device;

    AdcConfig();

    dev_adc->ops    = &cm32_adc_ops;
    dev_adc->max_value = (1UL << 12) - 1;
    dev_adc->ref_low   = 0;                /* ref 0 - 3.3v */
    dev_adc->ref_hight = 3300;

    result = os_hw_adc_register(dev_adc, dev->name, NULL);

    return result;
}

OS_DRIVER_INFO cm32_adc_driver = {
    .name   = "ADC_Type",
    .probe  = cm32_adc_probe,
};

OS_DRIVER_DEFINE(cm32_adc_driver, PREV, OS_INIT_SUBLEVEL_LOW);

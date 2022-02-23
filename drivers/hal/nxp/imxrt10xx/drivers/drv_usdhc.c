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
 * @file        drv_sdio_common.c
 *
 * @brief       This file provides sdio-usdhc bus common config.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_memory.h>

#include <fsl_sd.h>
#include <fsl_sdio.h>
#include <fsl_mmc.h>

#include <drv_usdhc.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usdhc"
#include <drv_log.h>

#ifdef BSP_USDHC1_ENABLE
AT_NONCACHEABLE_SECTION_ALIGN(static os_uint8_t usdhc1_dma_buff[IMXRT_USDHU_DMA_BUFF_SIZE], IMXRT_USDHU_DMA_BUFF_ALIGN_SIZE);
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
SDK_ALIGN(static os_uint8_t usdhc1_dma_buff_cache[IMXRT_USDHU_DMA_BUFF_CACHE_SIZE * 2U], IMXRT_USDHU_DMA_BUFF_ALIGN_SIZE);
#endif

static const struct nxp_usdhc_info usdhc1_info = 
{
    .base                   = USDHC1,
    .dma_buff               = usdhc1_dma_buff,
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    .dma_buff_cache         = usdhc1_dma_buff_cache;
#endif

    .pwr_pin.pin            = BSP_USDHC1_PWR_PIN,
    .pwr_pin.level          = BSP_USDHC1_PWR_PIN_LEVEL,

    .irq_pin.pin            = BSP_USDHC1_IRQ_PIN,
    .irq_pin.level          = BSP_USDHC1_IRQ_PIN_LEVEL,

#if defined(BSP_USDHC1_MODE_SD)
    .mode                   = IMXRT_USDHC_MODE_SD,
    .maxfrq                 = MMC_CLOCK_HS200,
#endif

#if defined(BSP_USDHC1_MODE_MMC)
    .mode                   = IMXRT_USDHC_MODE_MMC,
    .maxfrq                 = SD_CLOCK_50MHZ,
#endif

#if defined(BSP_USDHC1_MODE_SDIO)
    .mode                   = IMXRT_USDHC_MODE_SDIO,
    .maxfrq                 = MMC_CLOCK_HS200,
#endif
};
OS_HAL_DEVICE_DEFINE("USDHC_Type", "usdhc1", usdhc1_info);
#endif

#ifdef BSP_USDHC2_ENABLE
AT_NONCACHEABLE_SECTION_ALIGN(static os_uint8_t usdhc2_dma_buff[IMXRT_USDHU_DMA_BUFF_SIZE], IMXRT_USDHU_DMA_BUFF_ALIGN_SIZE);
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
SDK_ALIGN(static os_uint8_t usdhc2_dma_buff_cache[IMXRT_USDHU_DMA_BUFF_CACHE_SIZE * 2U], IMXRT_USDHU_DMA_BUFF_ALIGN_SIZE);
#endif

static const struct nxp_usdhc_info usdhc2_info = 
{
    .base                   = USDHC2,
    .dma_buff               = usdhc2_dma_buff;
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    .dma_buff_cache         = usdhc2_dma_buff_cache;
#endif

#if defined(BSP_USDHC2_MODE_SD)
        .mode                   = IMXRT_USDHC_MODE_SD,
#endif

#if defined(BSP_USDHC2_MODE_MMC)
        .mode                   = IMXRT_USDHC_MODE_MMC,
#endif

#if defined(BSP_USDHC2_MODE_SDIO)
        .mode                   = IMXRT_USDHC_MODE_SDIO,
#endif

};
OS_HAL_DEVICE_DEFINE("USDHC_Type", "usdhc2", usdhc2_info);
#endif

struct os_imxrt_usdhc *imxrt_usdhc1 = OS_NULL;
struct os_imxrt_usdhc *imxrt_usdhc2 = OS_NULL;

OS_WEAK os_err_t imxrt_usdhc_sd_register(struct os_imxrt_usdhc *imxrt_usdhc)
{
    return OS_EOK;
}
OS_WEAK os_err_t imxrt_usdhc_mmc_register(struct os_imxrt_usdhc *imxrt_usdhc)
{
    return OS_EOK;
}
OS_WEAK os_err_t imxrt_usdhc_sdio_register(struct os_imxrt_usdhc *imxrt_usdhc)
{
    return OS_EOK;
}

bool _usdhc1_init_get_detectstatus(void)
{
    if (imxrt_usdhc1 != OS_NULL)
    {
        if (os_pin_read(imxrt_usdhc1->usdhc_info->irq_pin.pin) == imxrt_usdhc1->usdhc_info->irq_pin.level)
            return OS_TRUE;
        else
            return OS_FALSE;
    }
    
    return OS_TRUE;
}

bool _usdhc2_init_get_detectstatus(void)
{
    if (imxrt_usdhc2 != OS_NULL)
    {
        if (os_pin_read(imxrt_usdhc2->usdhc_info->irq_pin.pin) == imxrt_usdhc2->usdhc_info->irq_pin.level)
            return OS_TRUE;
        else
            return OS_FALSE;
    }
    
    return OS_TRUE;
}

void _usdhc_irq_cb(void *args)
{
    struct os_imxrt_usdhc *imxrt_usdhc = (struct os_imxrt_usdhc *)args;
    
    if (imxrt_usdhc->s_cd.callback != OS_NULL)
    {
        imxrt_usdhc->s_cd.callback(imxrt_usdhc->s_cd.cardDetected(), imxrt_usdhc->s_cd.userData);
    }
}

static os_err_t _usdhc_detectinit(struct os_imxrt_usdhc *imxrt_usdhc)
{
    imxrt_usdhc->s_cd.cdDebounce_ms = 100U;
    imxrt_usdhc->s_cd.type          = SDMMC_SD_CD_TYPE;
    imxrt_usdhc->s_cd.callback      = imxrt_usdhc->callback;
    imxrt_usdhc->s_cd.userData      = imxrt_usdhc->userData;

    if (imxrt_usdhc->usdhc_info->base == USDHC1)
    {
        imxrt_usdhc->s_cd.cardDetected  = _usdhc1_init_get_detectstatus;
    }
    else if (imxrt_usdhc->usdhc_info->base == USDHC2)
    {
        imxrt_usdhc->s_cd.cardDetected  = _usdhc2_init_get_detectstatus;
    }

    if (imxrt_usdhc->s_cd.type == kSD_DetectCardByGpioCD)
    {
        if (imxrt_usdhc->usdhc_info->irq_pin.pin < 0)
        {
            return OS_EOK;
        }
        
        os_pin_mode(imxrt_usdhc->usdhc_info->irq_pin.pin, PIN_MODE_INPUT);
        
        os_pin_attach_irq(imxrt_usdhc->usdhc_info->irq_pin.pin, PIN_IRQ_MODE_RISING_FALLING, _usdhc_irq_cb, imxrt_usdhc);
        
        os_pin_irq_enable(imxrt_usdhc->usdhc_info->irq_pin.pin, OS_TRUE);
        
        if (os_pin_read(imxrt_usdhc->usdhc_info->irq_pin.pin) == imxrt_usdhc->usdhc_info->irq_pin.level)
        {
            if (imxrt_usdhc->s_cd.callback != OS_NULL)
            {
                imxrt_usdhc->s_cd.callback(OS_TRUE, imxrt_usdhc->s_cd.userData);
            }
        }
        else
        {
            LOG_E(DRV_EXT_TAG, "cannot detect card!");
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static void _usdhc1_pwrctl(bool enable)
{
    if (imxrt_usdhc1 != OS_NULL)
    {
        if (enable)
            os_pin_write(imxrt_usdhc1->usdhc_info->pwr_pin.pin, imxrt_usdhc1->usdhc_info->pwr_pin.level);
        else
            os_pin_write(imxrt_usdhc1->usdhc_info->pwr_pin.pin, !imxrt_usdhc1->usdhc_info->pwr_pin.level);
    }
}

static void _usdhc2_pwrctl(bool enable)
{
    if (imxrt_usdhc2 != OS_NULL)
    {
        if (enable)
            os_pin_write(imxrt_usdhc2->usdhc_info->pwr_pin.pin, imxrt_usdhc2->usdhc_info->pwr_pin.level);
        else
            os_pin_write(imxrt_usdhc2->usdhc_info->pwr_pin.pin, !imxrt_usdhc2->usdhc_info->pwr_pin.level);
    }
}

static os_err_t _usdhc_pwrctl_init(struct os_imxrt_usdhc *imxrt_usdhc)
{
    if (imxrt_usdhc->usdhc_info->pwr_pin.pin < 0)
    {
        return OS_EOK;
    }

    os_pin_mode(imxrt_usdhc->usdhc_info->pwr_pin.pin, PIN_MODE_OUTPUT);

    if (imxrt_usdhc->usdhc_info->base == USDHC1)
    {
        imxrt_usdhc->pwr_enable = _usdhc1_pwrctl;
    }
    else if (imxrt_usdhc->usdhc_info->base == USDHC2)
    {
        imxrt_usdhc->pwr_enable = _usdhc2_pwrctl;
    }

    imxrt_usdhc->pwr_enable(OS_TRUE);

    return OS_EOK;
}

os_uint32_t _clock_config(void)
{
    CLOCK_InitSysPll(&sysPllConfig_BOARD_BootClockRUN);
    /*configure system pll PFD0 fractional divider to 24, output clock is 528MHZ * 18 / 24 = 396 MHZ*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 24U);
    /* Configure USDHC clock source and divider */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 1U); /* USDHC clock root frequency maximum: 198MHZ */
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);

    return 396000000 / 2;
}

static os_err_t _usdhc_init(struct os_imxrt_usdhc *imxrt_usdhc)
{
    imxrt_usdhc->host.dmaDesBuffer          = imxrt_usdhc->usdhc_info->dma_buff;
    imxrt_usdhc->host.dmaDesBufferWordsNum  = SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;
    imxrt_usdhc->host.enableCacheControl    = kSDMMCHOST_CacheControlRWBuffer;
#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
    imxrt_usdhc->host->cacheAlignBuffer     = imxrt_usdhc->usdhc_info->dma_buff_cache;
    imxrt_usdhc->host->cacheAlignBufferSize = IMXRT_USDHU_DMA_BUFF_ALIGN_SIZE * 2U;
#endif
    imxrt_usdhc->host.hostController.sourceClock_Hz = _clock_config();
    imxrt_usdhc->host.hostController.base  = imxrt_usdhc->usdhc_info->base;

    imxrt_usdhc->io_voltage.type            = kSD_IOVoltageCtrlNotSupport;
    imxrt_usdhc->io_voltage.func            = OS_NULL;

    if (imxrt_usdhc->usdhc_info->base == USDHC1)
    {
        imxrt_usdhc1                        = imxrt_usdhc;
    }
    else if (imxrt_usdhc->usdhc_info->base == USDHC2)
    {
        imxrt_usdhc2                        = imxrt_usdhc;
    }

    _usdhc_pwrctl_init(imxrt_usdhc);

    if (imxrt_usdhc->usdhc_info->mode != IMXRT_USDHC_MODE_MMC)
    {
        if (_usdhc_detectinit(imxrt_usdhc) != OS_EOK)
        {
            return OS_ERROR;
        }
    }

    switch(imxrt_usdhc->usdhc_info->mode)
    {
    case IMXRT_USDHC_MODE_SD:
        imxrt_usdhc->sd_card->host                  = &imxrt_usdhc->host;
        imxrt_usdhc->sd_card->usrParam.cd           = &imxrt_usdhc->s_cd;
        imxrt_usdhc->sd_card->usrParam.pwr          = (sd_pwr_t)imxrt_usdhc->pwr_enable;
        imxrt_usdhc->sd_card->usrParam.ioStrength   = OS_NULL; 
        imxrt_usdhc->sd_card->usrParam.ioVoltage    = &imxrt_usdhc->io_voltage;
        imxrt_usdhc->sd_card->usrParam.maxFreq      = imxrt_usdhc->usdhc_info->maxfrq;
    break;
    case IMXRT_USDHC_MODE_MMC:
        imxrt_usdhc->mmc_card->host                 = &imxrt_usdhc->host;
        imxrt_usdhc->mmc_card->usrParam.ioStrength  = OS_NULL; 
        imxrt_usdhc->mmc_card->usrParam.maxFreq     = imxrt_usdhc->usdhc_info->maxfrq;
    break;
    case IMXRT_USDHC_MODE_SDIO:
        imxrt_usdhc->sdio_card->host                = &imxrt_usdhc->host;
        imxrt_usdhc->sdio_card->usrParam.cd         = &imxrt_usdhc->s_cd;
        imxrt_usdhc->sdio_card->usrParam.pwr        = (sd_pwr_t)imxrt_usdhc->pwr_enable;
        imxrt_usdhc->sdio_card->usrParam.ioStrength = OS_NULL; 
        imxrt_usdhc->sdio_card->usrParam.ioVoltage  = &imxrt_usdhc->io_voltage;
        imxrt_usdhc->sdio_card->usrParam.maxFreq    = imxrt_usdhc->usdhc_info->maxfrq;
    break;
    default:
    break;
    }

    if (imxrt_usdhc->usdhc_info->base == USDHC1)
    {
        NVIC_SetPriority(USDHC1_IRQn, SDMMC_SD_CD_IRQ_PRIORITY);
    }
    else if (imxrt_usdhc->usdhc_info->base == USDHC2)
    {
        NVIC_SetPriority(USDHC2_IRQn, SDMMC_SD_CD_IRQ_PRIORITY);
    }

    return OS_EOK;
}

const static struct imxrt_usdhc_ops usdhc_ops = {
    .init    = _usdhc_init,
};

static int imxrt_usdhc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct os_imxrt_usdhc *imxrt_usdhc = OS_NULL;

    imxrt_usdhc = os_calloc(1, sizeof(struct os_imxrt_usdhc));
    if (imxrt_usdhc == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "imxrt_usdhc memory call failed!");
        return OS_ENOMEM;
    }
    
    imxrt_usdhc->usdhc_info = (struct nxp_usdhc_info *)dev->info;
    imxrt_usdhc->name = dev->name;
    imxrt_usdhc->ops  = &usdhc_ops;
    
    switch(imxrt_usdhc->usdhc_info->mode)
    {
    case IMXRT_USDHC_MODE_SD:
        imxrt_usdhc->sd_card = os_calloc(1, sizeof(sd_card_t));
        if (imxrt_usdhc->sd_card == OS_NULL)
        {
            LOG_E(DRV_EXT_TAG, "imxrt_usdhc->sd_card memory call failed!");
            return OS_ENOMEM;
        }
        
        imxrt_usdhc_sd_register(imxrt_usdhc);
    break;
    case IMXRT_USDHC_MODE_MMC:
        imxrt_usdhc->mmc_card = os_calloc(1, sizeof(mmc_card_t));
        if (imxrt_usdhc->mmc_card == OS_NULL)
        {
            LOG_E(DRV_EXT_TAG, "imxrt_usdhc->mmc_card memory call failed!");
            return OS_ENOMEM;
        }
        
        imxrt_usdhc_mmc_register(imxrt_usdhc);
    break;
    case IMXRT_USDHC_MODE_SDIO:
        imxrt_usdhc->sdio_card = os_calloc(1, sizeof(sdio_card_t));
        if (imxrt_usdhc->sdio_card == OS_NULL)
        {
            LOG_E(DRV_EXT_TAG, "imxrt_usdhc->sdio_card memory call failed!");
            return OS_ENOMEM;
        }
        
        imxrt_usdhc_sdio_register(imxrt_usdhc);
    break;
    default:
    break;
    }

    return OS_EOK;
}

OS_DRIVER_INFO imxrt_usdhc_driver = {
    .name   = "USDHC_Type",
    .probe  = imxrt_usdhc_probe,
};

OS_DRIVER_DEFINE(imxrt_usdhc_driver, DEVICE, OS_INIT_SUBLEVEL_LOW);



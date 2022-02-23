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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for nxp
 *
 * @revision
 * Date         Author          Notes
 * 2021-07-13   OneBSP Team      First Version
 ***********************************************************************************************************************
 */
#define BSP_USDHC1_IRQ_PIN          10
#define BSP_USDHC1_IRQ_PIN_LEVEL    0
#define BSP_USDHC1_PWR_PIN          10
#define BSP_USDHC1_PWR_PIN_LEVEL    0
#define BSP_USDHC1_DMA_BUFF_SIZE    32
#define BSP_SDIO_USDHC2_BLOCK_SIZE  512
 
os_imxrt_sdio_cfg_param_t sdio_cfg_param[] =
{
#if defined(BSP_USDHC1_ENABLE)
    {
    .hBSPt_baseaddr                  = USDHC1,
    .hBSPt_irq                       = USDHC1_IRQn,
    .hBSPt_irq_priority              = 5,
    .hBSPt_cache_ctl                 = kSDMMCHBSPT_CacheControlRWBuffer,
    .cd_type                        = kSD_DetectCardByGpioCD,
    .card_detect_debounce_delay_ms  = 100,
#if defined(BSP_USDHC1_SD)
    .maxfrq                         = MMC_CLOCK_HS200,
#elif defined(BSP_USDHC1_SDIO)
    .maxfrq                         = SD_CLOCK_50MHZ,
#elif defined(BSP_USDHC1_MMC)
    .maxfrq                         = MMC_CLOCK_HS200,
#endif
    .io_voltage.type                = kSD_IOVoltageCtrlByHBSPt,
    .io_voltage.func                = BSP_NULL,
#if defined(BSP_USDHC1_SD)
    .sdio_type                      = BSP_SDIO_TYPE_SD,
#elif defined(BSP_USDHC1_SDIO)
    .sdio_type                      = BSP_SDIO_TYPE_SDIO,
    .block_size[kSDIO_FunctionNum0] = BSP_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum1] = BSP_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum2] = BSP_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum3] = BSP_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum4] = BSP_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum5] = BSP_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum6] = BSP_SDIO_USDHC1_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum7] = BSP_SDIO_USDHC1_BLOCK_SIZE;
#elif defined(BSP_USDHC1_MMC)
    .sdio_type                      = BSP_SDIO_TYPE_MMC,
#endif
#if defined(BSP_USDHC1_PWR_EN) && defined(BSP_USDHC1_PWR_PIN)
    .pwr_pin                        = BSP_USDHC1_PWR_PIN,
    .pwr_pin_level                  = BSP_USDHC1_PWR_PIN_LEVEL,
#else
    .pwr_pin                        = 0,
#endif
#if defined(BSP_USDHC1_IRQ_EN) && defined(BSP_USDHC1_IRQ_PIN)
    .irq_pin                        = BSP_USDHC1_IRQ_PIN,
    .intterupt_type                 = PIN_IRQ_MODE_RISING_FALLING,
    .insert_level                   = BSP_USDHC1_IRQ_PIN_LEVEL,
#else
    .irq_pin                        = NULL,
#endif
    .buf_size                       = BSP_USDHC1_DMA_BUFF_SIZE,
    },
#endif
#if defined(BSP_USDHC2_ENABLE)
    {
    .hBSPt_baseaddr                  = USDHC2,
    .hBSPt_irq                       = USDHC2_IRQn,
    .hBSPt_irq_priority              = 5,
    .hBSPt_cache_ctl                 = kSDMMCHBSPT_CacheControlRWBuffer,
    .cd_type                        = kSD_DetectCardByGpioCD,
    .card_detect_debounce_delay_ms  = 100,
#if defined(BSP_USDHC2_SD)
    .maxfrq                         = MMC_CLOCK_HS200,
#elif defined(BSP_USDHC2_SDIO)
    .maxfrq                         = SD_CLOCK_50MHZ,
    .block_size[kSDIO_FunctionNum0] = BSP_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum1] = BSP_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum2] = BSP_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum3] = BSP_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum4] = BSP_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum5] = BSP_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum6] = BSP_SDIO_USDHC2_BLOCK_SIZE;
    .block_size[kSDIO_FunctionNum7] = BSP_SDIO_USDHC2_BLOCK_SIZE;
#elif defined(BSP_USDHC2_MMC)
    .maxfrq                         = MMC_CLOCK_HS200,
#endif
    .io_voltage.type                = kSD_IOVoltageCtrlByHBSPt,
    .io_voltage.func                = BSP_NULL,
#if defined(BSP_USDHC2_SD)
    .sdio_type                      = BSP_SDIO_TYPE_SD,
#elif defined(BSP_USDHC2_SDIO)
    .sdio_type                      = BSP_SDIO_TYPE_SDIO,
#elif defined(BSP_USDHC2_MMC)
    .sdio_type                      = BSP_SDIO_TYPE_MMC,
#endif
#if defined(BSP_USDHC2_PWR_EN) && defined(BSP_USDHC1_PWR_PIN)
    .pwr_pin                        = BSP_USDHC2_PWR_PIN,
    .pwr_pin_level                  = BSP_USDHC2_PWR_PIN_LEVEL,
#else
    .pwr_pin                        = NULL,
#endif
#if defined(BSP_USDHC2_IRQ_EN) && defined(BSP_USDHC1_PWR_PIN)
    .irq_pin                        = BSP_USDHC2_IRQ_PIN,
    .intterupt_type                 = PIN_IRQ_MODE_RISING_FALLING,
    .insert_level                   = BSP_USDHC2_IRQ_PIN_LEVEL,
#else
    .irq_pin                        = NULL,
#endif
    .buf_size                       = BSP_USDHC2_DMA_BUFF_SIZE,
    },
#endif
};
#endif
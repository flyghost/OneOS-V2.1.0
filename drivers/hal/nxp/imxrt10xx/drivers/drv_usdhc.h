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
 * @file        drv_sdio_common.h
 *
 * @brief       This file provides sdio-usdhc bus common config.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-07   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef DRV_USDHC_H__
#define DRV_USDHC_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <pin.h>

#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_sdmmc_host.h"
#include "fsl_sdmmc_common.h"

#include <fsl_sd.h>
#include <fsl_sdio.h>
#include <fsl_mmc.h>
#include "fsl_usdhc.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BSP_USDHC1_PWR_PIN_LEVEL
#define BSP_USDHC1_PWR_PIN_LEVEL                0
#endif
#ifndef BSP_USDHC1_IRQ_PIN_LEVEL
#define BSP_USDHC1_IRQ_PIN_LEVEL                0
#endif

#ifndef BSP_USDHC2_PWR_PIN_LEVEL
#define BSP_USDHC2_PWR_PIN_LEVEL                0
#endif
#ifndef BSP_USDHC2_IRQ_PIN_LEVEL
#define BSP_USDHC2_IRQ_PIN_LEVEL                0
#endif

#define IMXRT_USDHU_DMA_BUFF_SIZE               0x40
#define IMXRT_USDHU_DMA_BUFF_CACHE_SIZE         0x04
#define IMXRT_USDHU_DMA_BUFF_ALIGN_SIZE         0x04

#define SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE   (32U)
#define SDMMC_SD_CD_TYPE                        kSD_DetectCardByGpioCD
#define SDMMC_SD_CD_IRQ_PRIORITY                6U
#define SDMMC_SD_IO_VOLTAGE_CONTROL_TYPE        kSD_IOVoltageCtrlByHost

enum imxrt_usdhc_mode
{
    IMXRT_USDHC_MODE_SD = 1,
    IMXRT_USDHC_MODE_MMC,
    IMXRT_USDHC_MODE_SDIO
};

struct imxrt_usdhc_pin
{
    os_bool_t   status;
    os_base_t   pin;
    os_uint8_t  level;
};

struct nxp_usdhc_info
{
    USDHC_Type             *base;
    os_uint32_t             mode;
    os_uint32_t             maxfrq;
    struct imxrt_usdhc_pin  pwr_pin;
    struct imxrt_usdhc_pin  irq_pin;
    
    os_uint8_t             *dma_buff;
    os_uint8_t             *dma_buff_cache;
};

struct os_imxrt_usdhc;

struct imxrt_usdhc_ops
{
    os_err_t (*init)(struct os_imxrt_usdhc *imxrt_usdhc);
    os_err_t (*read)(struct os_imxrt_usdhc *imxrt_usdhc);
    os_err_t (*write)(struct os_imxrt_usdhc *imxrt_usdhc);
};

struct os_imxrt_usdhc
{
    sdmmchost_t                     host;
    sd_detect_card_t                s_cd;
    sd_io_voltage_t                 io_voltage;
    
    sd_pwr_t                        pwr_enable;
    sd_cd_t                         callback;
    struct nxp_usdhc_info          *usdhc_info;
    char                           *name;
    sd_card_t                      *sd_card;
    mmc_card_t                     *mmc_card;
    sdio_card_t                    *sdio_card;
    void                           *userData;
    const struct imxrt_usdhc_ops   *ops;
};

#ifdef __cplusplus
}
#endif

#endif


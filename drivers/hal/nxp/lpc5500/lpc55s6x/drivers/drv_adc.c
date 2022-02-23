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
 * @brief       This file implements adc driver for nxp.
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
#include <drv_adc.h>
#include "fsl_power.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.adc"
#include <drv_log.h>

#define ADC_GET_TIMEOUT 0xFFFF

typedef struct lpc_adc
{
    struct os_adc_device adc;
    struct lpc_adc_info *info;
    lpadc_conv_result_t adc_result;
    os_uint8_t status;
}lpc_adc_t;

static os_err_t lpc_adc_read(struct os_adc_device *adc, os_uint32_t channel, os_int32_t *buff)
{
    lpc_adc_t *lpc_adc;
    os_int32_t adc_buff;
    os_uint32_t count = 0;
    
    lpc_adc = os_container_of(adc, struct lpc_adc, adc);

    if (lpc_adc->status != OS_TRUE)
    {
        LOG_E(DRV_EXT_TAG, "adc need enable first!");
        return OS_ERROR;
    }

    LPADC_DoSoftwareTrigger(lpc_adc->info->adc_base, 1 << channel);
    while (!LPADC_GetConvResult(lpc_adc->info->adc_base, &lpc_adc->adc_result, 0) && (count < ADC_GET_TIMEOUT))
    {
        count++;
    }
    if (count >= ADC_GET_TIMEOUT)
    {
        LOG_E(DRV_EXT_TAG, "channel %d cannot read! please check timeout value or channel config!", channel);
        return OS_ERROR;
    }

    *buff = (os_int32_t)((os_uint64_t)((lpc_adc->adc_result.convValue) >> 3U) * adc->mult >> adc->shift);
    return OS_EOK;
}

static os_err_t lpc_adc_enabled(struct os_adc_device *adc, os_bool_t enable)
{
    lpc_adc_t *lpc_adc;
    
    lpc_adc = os_container_of(adc, struct lpc_adc, adc);
    
    if (enable == OS_TRUE)
    {
        lpc_adc->status = OS_TRUE;
    }
    else
    {
        lpc_adc->status = OS_FALSE;
    }
    
    return OS_EOK;
}

static os_err_t lpc_adc_control(struct os_adc_device *adc, int cmd, void *arg)
{
    return OS_EOK;
}

static void lpc_adc_init(void)
{
    POWER_DisablePD(kPDRUNCFG_PD_LDOGPADC);
}

static const struct os_adc_ops lpc_adc_ops = {
    .adc_enabled            = lpc_adc_enabled,
    .adc_control            = OS_NULL,
    .adc_read               = lpc_adc_read,
};

static int lpc_adc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    
    struct lpc_adc_info *adc_info = (struct lpc_adc_info *)dev->info;
    
    lpc_adc_t *lpc_adc = os_calloc(1, sizeof(lpc_adc_t)); 
    OS_ASSERT(lpc_adc);
    
    lpc_adc->info = adc_info;
        
    struct os_adc_device *dev_adc = &lpc_adc->adc;
    dev_adc->ops        = &lpc_adc_ops;
    dev_adc->max_value  = (1UL << 12) - 1;;
    dev_adc->ref_low    = 0;                 /* ref 0 - 3.3v */
    dev_adc->ref_hight  = 3300;
    
    lpc_adc_init();
    result = os_hw_adc_register(dev_adc, dev->name, OS_NULL);

    return result;
}

OS_DRIVER_INFO lpc_adc_driver = {
    .name   = "ADC_Type",
    .probe  = lpc_adc_probe,
};

OS_DRIVER_DEFINE(lpc_adc_driver, DEVICE, OS_INIT_SUBLEVEL_LOW);


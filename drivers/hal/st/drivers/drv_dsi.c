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
 * @file        drv_lcd_mipi.c
 *
 * @brief       This file implements lcd mipi driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <string.h>
#include <os_sem.h>
#include <os_memory.h>
#include <graphic/dsi.h>
#include <lcd_port.h>
#include <shell.h>
#include "drv_gpio.h"

#define LOG_TAG "drv.dsi"
#include <drv_log.h>

struct stm32_dsi {
    os_device_dsi_t    dsi;
    
    DSI_HandleTypeDef *hdsi;
};

static void stm32_dsi_cmd(os_device_dsi_t *dsi, os_uint32_t channel, os_uint8_t *params, os_uint32_t params_nr)
{
    struct stm32_dsi *st_dsi;

    OS_ASSERT(dsi);

    st_dsi = (struct stm32_dsi *)dsi;

    if (params_nr <= 1)
    {
        HAL_DSI_ShortWrite(st_dsi->hdsi, channel, DSI_DCS_SHORT_PKT_WRITE_P1, params[0], params[1]);
    }
    else
    {
        HAL_DSI_LongWrite(st_dsi->hdsi, channel, DSI_DCS_LONG_PKT_WRITE, params_nr, params[params_nr], params);
    }
}

void stm32_dsi_start(os_device_dsi_t *dsi)
{
    struct stm32_dsi *st_dsi;

    OS_ASSERT(dsi);

    st_dsi = (struct stm32_dsi *)dsi;

    HAL_DSI_Start(st_dsi->hdsi);
}

void stm32_dsi_stop(os_device_dsi_t *dsi)
{
    struct stm32_dsi *st_dsi;

    OS_ASSERT(dsi);

    st_dsi = (struct stm32_dsi *)dsi;

    HAL_DSI_Stop(st_dsi->hdsi);
}

static struct os_device_dsi_ops ops =
{
    .start = stm32_dsi_start,
    .stop  = stm32_dsi_stop,
    .cmd   = stm32_dsi_cmd,
};

static int stm32_dsi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_dsi *dsi;

    dsi = os_calloc(1, sizeof(struct stm32_dsi));
    
    OS_ASSERT(dsi);
    
    dsi->hdsi = (DSI_HandleTypeDef *)dev->info;

    dsi->dsi.ops   = &ops;
    
    os_dsi_register("dsi", &dsi->dsi);

    os_kprintf("stm32 dsi found.\r\n");
    
    return OS_EOK;
}

OS_DRIVER_INFO stm32_dsi_driver = {
    .name   = "DSI_HandleTypeDef",
    .probe  = stm32_dsi_probe,
};

OS_DRIVER_DEFINE(stm32_dsi_driver,DEVICE,OS_INIT_SUBLEVEL_HIGH);


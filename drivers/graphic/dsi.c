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
 * @file        dsi.c
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
#include <lcd_port.h>
#include <graphic/dsi.h>

void os_dsi_reset(os_device_dsi_t *dsi, os_bool_t state)
{
    if (state)
    {
        os_pin_write(dsi->reset_pin, PIN_LOW);
    }
    else
    {
        os_pin_write(dsi->reset_pin, PIN_HIGH);
    }
}


void os_dsi_register(const char *name, os_device_dsi_t *dsi)
{
    OS_ASSERT(dsi != OS_NULL);
    OS_ASSERT(dsi->ops != OS_NULL);

    dsi->reset_pin = OS_USING_DSI_RST;
    
    os_pin_mode(dsi->reset_pin, PIN_MODE_OUTPUT);

    os_dsi_reset(dsi, OS_TRUE);
    os_task_msleep(20);
    os_dsi_reset(dsi, OS_FALSE);
    os_task_msleep(10);

    dsi->parent.type  = OS_DEVICE_TYPE_GRAPHIC;
    os_device_register(&dsi->parent, name);
}


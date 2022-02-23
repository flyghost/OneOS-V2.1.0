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
 * @file        dsi.h
 *
 * @brief       This file implements lcd mipi driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _DSI_H_
#define _DSI_H_

#include <board.h>
#include <drv_cfg.h>

typedef struct os_device_dsi os_device_dsi_t;

struct os_device_dsi_ops
{
    void (*start)(os_device_dsi_t *dsi);
    void (*stop)(os_device_dsi_t *dsi);
    void (*cmd)(os_device_dsi_t *dsi, os_uint32_t channel, os_uint8_t *params, os_uint32_t params_nr);
};

struct os_device_dsi {
    os_device_t parent;

    int reset_pin;
    
    struct os_device_dsi_ops *ops;
};

#define os_dsi_start(dsi) dsi->ops->start(dsi)
#define os_dsi_stop(dsi)  dsi->ops->stop(dsi)
#define os_dsi_cmd(dsi, channel, params, params_nr) dsi->ops->cmd(dsi, channel, params, params_nr)

void os_dsi_reset(os_device_dsi_t *dsi, os_bool_t state);
void os_dsi_register(const char *name, os_device_dsi_t *graphic);

#endif /* _DSI_H_ */


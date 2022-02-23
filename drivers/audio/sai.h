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
 * @file        sai.h
 *
 * @brief       SAI function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _SAI_H_
#define _SAI_H_

#include <board.h>
#include <drv_cfg.h>

#ifndef OS_AUDIO_CMD_TX_ENABLE
#define OS_AUDIO_CMD_TX_ENABLE                IOC_SOUND(0x00)
#define OS_AUDIO_CMD_RX_ENABLE                IOC_SOUND(0x01)
#define OS_AUDIO_CMD_SET_FRQ                  IOC_SOUND(0x02)
#define OS_AUDIO_CMD_SET_CHANNEL              IOC_SOUND(0x03)
#define OS_AUDIO_CMD_TX_DISABLE               IOC_SOUND(0x04)
#define OS_AUDIO_CMD_RX_DISABLE               IOC_SOUND(0x05)
#endif

typedef struct os_device_sai os_device_sai_t;

struct os_device_sai_ops
{
    os_err_t (*transimit)(os_device_sai_t *sai, uint8_t *buff, uint32_t size);
    os_err_t (*receive)(os_device_sai_t *sai, uint8_t *buff, uint32_t size);
    os_err_t (*enable_tx)(os_device_sai_t *sai, os_bool_t enable);
    os_err_t (*enable_rx)(os_device_sai_t *sai, os_bool_t enable);
    os_err_t (*set_frq)(os_device_sai_t *sai, uint32_t frequency);
    os_err_t (*set_channel)(os_device_sai_t *sai, uint8_t channels);
};

struct os_device_sai {
    os_device_t parent;
    const struct os_device_sai_ops *ops;
};

os_err_t os_sai_register(const char *name, os_device_sai_t *graphic);
void os_hw_sai_isr(struct os_device_sai *sai, struct os_device_cb_info *info);

#endif /* _sai_H_ */

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
 * @file        i2s.h
 *
 * @brief       I2S function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _I2S_H_
#define _I2S_H_

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

enum
{
    I2S_REPLAY_EVT_NONE  = 0x00,
    I2S_REPLAY_EVT_START = 0x01,
    I2S_REPLAY_EVT_STOP  = 0x02,
};

typedef struct os_i2s_device os_i2s_device_t;

struct os_i2s_buf_info
{
    os_uint8_t *buffer;
    os_uint16_t block_size;
    os_uint16_t block_count;
    os_uint32_t total_size;
};

struct os_i2s_ops
{
    os_err_t (*init)(struct os_i2s_device *i2s);
    os_err_t (*transimit)(struct os_i2s_device *i2s, uint8_t *pData, uint32_t Size);
    os_err_t (*receive)(struct os_i2s_device *i2s, uint8_t *pData, uint32_t Size);
    os_err_t (*enable_tx)(struct os_i2s_device *i2s, os_bool_t enable);
    os_err_t (*enable_rx)(struct os_i2s_device *i2s, os_bool_t enable);
    os_err_t (*set_frq)(struct os_i2s_device *i2s, uint32_t frequency);
    os_err_t (*set_channel)(struct os_i2s_device *i2s, uint8_t channels);
};


struct os_i2s_device{
    os_device_t parent; 
    const struct os_i2s_ops *ops;
};

os_err_t os_i2s_register(struct os_i2s_device *i2s, const char *name, void *data);
void os_hw_i2s_isr(struct os_i2s_device *i2s, struct os_device_cb_info *info);

#endif /* _i2s_H_ */

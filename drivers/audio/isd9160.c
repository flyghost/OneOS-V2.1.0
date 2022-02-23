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
 * @file        isd9160.c
 *
 * @brief       This file implements audio driver for isd9160.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <arch_interrupt.h>

#include <stdint.h>
#include <string.h>
#include "drv_cfg.h"

#include "drv_audio.h"
#include <i2c/i2c.h>
#include <audio/sai.h>

#define DBG_EXT_TAG "audio.isd9160"


typedef enum {
    I2C_CMD_SLPRT = 0,
    I2C_CMD_UPGRADE,
    I2C_CMD_RESPONSE,
    I2C_CMD_RECORD,
    I2C_CMD_PLAYBACK,
    I2C_CMD_AUDIOSTOP,
    
    I2C_CMD_END
} I2C_PROC_CMD;

struct isd9160_dev
{
    struct os_i2c_client i2c;
    os_int16_t           rst_pin;
};

struct isd9160_player_device
{
    struct os_audio_device    audio;
    struct os_audio_configure replay_config;
    os_uint8_t                volume;
    os_device_t     *cfg_bus;
    os_device_t     *data_bus;
    struct isd9160_dev isd9160;
};

void hton_4(uint8_t *data, uint32_t value)
{
    data[0] = (value >> (8 * 3)) & 0xff;
    data[1] = (value >> (8 * 2)) & 0xff;
    data[2] = (value >> (8 * 1)) & 0xff;
    data[3] = (value >> (8 * 0)) & 0xff;
}

void ntoh_4(const uint8_t *data, uint32_t *value)
{
    *value = 0;
    *value |= data[0] << (8 * 3);
    *value |= data[1] << (8 * 2);
    *value |= data[2] << (8 * 1);
    *value |= data[3] << (8 * 0);
}

void hton_2(uint8_t *data, uint16_t value)
{
    data[0] = (value >> (8 * 1)) & 0xff;
    data[1] = (value >> (8 * 0)) & 0xff;
}

/* ISD9160 Device Driver Interface */

static os_err_t audio_isd9160_config(struct os_audio_device *audio, struct os_audio_caps *caps)
{
    os_err_t                      result = OS_EOK;
    struct isd9160_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct isd9160_player_device *)audio->parent.user_data;

    switch (caps->config_type)
    {
    case AUDIO_VOLUME_CMD:
    {
        os_uint8_t volume = caps->udata.value;

        volume *= 2;

        //isd9160_set_volume(&aduio_dev->isd9160, volume);

        aduio_dev->volume = volume;
        LOG_D(DBG_EXT_TAG, "set volume %d", volume);
        break;
    }

    case AUDIO_PARAM_CMD:
    {
        os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &caps->udata.config.samplerate);
        os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &caps->udata.config.channels);

        aduio_dev->replay_config.samplerate = caps->udata.config.samplerate;
        aduio_dev->replay_config.channels   = caps->udata.config.channels;
        aduio_dev->replay_config.samplebits = caps->udata.config.samplebits;
        LOG_D(DBG_EXT_TAG, "set samplerate %d", aduio_dev->replay_config.samplerate);
        break;
    }

    default:
        result = OS_ERROR;
        break;
    }

    return result;
}

static os_err_t audio_isd9160_init(struct os_audio_device *audio)
{
    os_err_t                      result = OS_EOK;
    struct isd9160_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);

    aduio_dev = (struct isd9160_player_device *)audio->parent.user_data;

    os_pin_mode(aduio_dev->isd9160.rst_pin, PIN_MODE_OUTPUT);

    os_pin_write(aduio_dev->isd9160.rst_pin, PIN_LOW);
    os_task_msleep(50);
    os_pin_write(aduio_dev->isd9160.rst_pin, PIN_HIGH);
    os_task_msleep(50);

    uint32_t slprt_size = 0;
    uint8_t  slprt_data[64];

    memset(slprt_data, 0, 4);
    os_i2c_client_read(&aduio_dev->isd9160.i2c, I2C_CMD_SLPRT, 1, slprt_data, 4);

    ntoh_4(slprt_data, &slprt_size);

    if (slprt_size > sizeof(slprt_data))
        slprt_size = sizeof(slprt_data);

    if (slprt_size > 0)
        os_i2c_client_read(&aduio_dev->isd9160.i2c, 0, 0, slprt_data, slprt_size);

    slprt_data[slprt_size] = 0;

    LOG_D(DBG_EXT_TAG, "slprt_data:\r\n%s\r\n", slprt_data);
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->replay_config.samplerate);
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->replay_config.channels);

    return result;
}

static os_err_t audio_isd9160_start(struct os_audio_device *audio)
{
    struct isd9160_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct isd9160_player_device *)audio->parent.user_data;

    os_i2c_client_write(&aduio_dev->isd9160.i2c, I2C_CMD_PLAYBACK, 1, OS_NULL, 0);

    LOG_D(DBG_EXT_TAG, "open sound device");

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_ENABLE, OS_NULL);
    
    return OS_EOK;
}

static os_err_t audio_isd9160_stop(struct os_audio_device *audio)
{
    struct isd9160_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct isd9160_player_device *)audio->parent.user_data;

    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_DISABLE, OS_NULL);

    os_i2c_client_write(&aduio_dev->isd9160.i2c, I2C_CMD_AUDIOSTOP, 1, OS_NULL, 0);

    LOG_D(DBG_EXT_TAG, "close sound device");

    return OS_EOK;
}

os_size_t audio_isd9160_transmit(struct os_audio_device *audio, const void *writeBuf, os_size_t size)
{
    struct isd9160_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct isd9160_player_device *)audio->parent.user_data;

    return os_device_write_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)writeBuf, size);
}

os_size_t audio_isd9160_receive(struct os_audio_device *audio, void *readBuf, os_size_t size)
{
    struct isd9160_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct isd9160_player_device *)audio->parent.user_data;
    
    return os_device_read_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)readBuf, size);
}

os_err_t audio_isd9160_data_tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    if (dev->user_data != OS_NULL)
    {
        struct isd9160_player_device *isd9160_player_dev = dev->user_data;

        os_device_send_notify(&isd9160_player_dev->audio.parent);

        return OS_EOK;
    }
    return OS_ENOSYS;
}

static struct os_audio_ops isd9160_player_ops = {
    .getcaps            = OS_NULL,
    .configure          = audio_isd9160_config,
    .init               = audio_isd9160_init,
    .start              = audio_isd9160_start,
    .stop               = audio_isd9160_stop,
    .transmit           = audio_isd9160_transmit, 
    .receive            = audio_isd9160_receive,
};

int os_hw_audio_player_init(void)
{
    struct isd9160_player_device *isd9160_player;
    struct isd9160_dev *isd9160;

    isd9160_player = os_calloc(1, sizeof(struct isd9160_player_device));
    OS_ASSERT(isd9160_player != OS_NULL);

    isd9160 = &isd9160_player->isd9160;

    isd9160_player->replay_config.samplerate = 44100;
    isd9160_player->replay_config.channels   = 2;
    isd9160_player->replay_config.samplebits = 16;
    isd9160_player->volume                   = 50;

    isd9160_player->audio.ops = &isd9160_player_ops;

    isd9160_player->cfg_bus = os_device_find(BSP_ISD9160_I2C_BUS);
    if (isd9160_player->cfg_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the config device!\r\n");
        return OS_ERROR;
    }
    
    isd9160_player->data_bus = os_device_find(BSP_AUDIO_DATA_TX_BUS);
    if (isd9160_player->data_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the data device!\r\n");
        return OS_ERROR;
    }
    isd9160_player->data_bus->user_data = isd9160_player;
    os_device_open(isd9160_player->data_bus);

    struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
    info->type = OS_DEVICE_CB_TYPE_TX;
    info->cb = audio_isd9160_data_tx_done;
    os_device_control(isd9160_player->data_bus, OS_DEVICE_CTRL_SET_CB, info);
    
    isd9160->i2c.bus = (struct os_i2c_bus_device *)isd9160_player->cfg_bus;

    isd9160->i2c.client_addr = BSP_ISD9160_I2C_ADDR;
    isd9160->rst_pin         = BSP_ISD9160_RST_PIN;

    os_audio_player_register(&isd9160_player->audio, "audio0", isd9160_player);

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_audio_player_init, OS_INIT_SUBLEVEL_LOW);

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
 * @file        es8388.c
 *
 * @brief       This file implements audio driver for es8388.
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

#include "es8388_ll.h"
#include <dlog.h>
#include <i2c/i2c.h>

#ifdef OS_USING_SAI
#include <audio/sai.h>
#endif
#ifdef OS_USING_I2S
#include <audio/i2s.h>
#endif

#define DBG_EXT_TAG "ES8388"

struct es8388_player_device
{
    struct            os_audio_device    audio;
    struct            os_audio_configure replay_config;
    os_uint8_t        volume; 
    os_device_t       *cfg_bus;
    os_device_t       *data_bus;
};

struct es8388_recorder_device
{
    struct os_audio_device    audio;
    struct os_audio_configure read_config;
    
    os_device_t *cfg_bus;
    os_device_t *data_bus;
};

static os_err_t audio_es8388_player_config(struct os_audio_device *audio, struct os_audio_caps *caps)
{
    os_err_t result = OS_EOK;
    OS_ASSERT(audio != OS_NULL);
       
    struct es8388_player_device *aduio_dev;
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    switch (caps->config_type)
    {
        case AUDIO_VOLUME_CMD:
        {
            os_uint8_t volume = caps->udata.value;

            es8388_volume_set(volume);
            aduio_dev->volume = volume;
            LOG_D(DBG_EXT_TAG, "set volume %d", volume);
            break;
        }

        case AUDIO_PARAM_CMD:
        {
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &caps->udata.config.samplerate);
#ifdef OS_USING_SAI             
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &caps->udata.config.channels);
#endif            
            
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

static os_err_t audio_es8388_player_init(struct os_audio_device *audio)  
{  
    os_err_t result = OS_EOK;
    OS_ASSERT(audio != OS_NULL);
        
    struct es8388_player_device *aduio_dev;
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    
    es8388_init(aduio_dev->cfg_bus, BSP_ES8388_POWER_PIN); 
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->replay_config.samplerate);
#ifdef OS_USING_SAI
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->replay_config.channels); 
#endif      
    return result;
}

static os_err_t audio_es8388_player_deinit(struct os_audio_device *audio)
{
    OS_ASSERT(audio != OS_NULL);

    struct es8388_player_device *aduio_dev = (struct es8388_player_device *)audio;

    return es8388_deinit(aduio_dev->cfg_bus);
}

static os_err_t audio_es8388_player_start(struct os_audio_device *audio)
{
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    
    LOG_D(DBG_EXT_TAG, "open sound device");
    
    es8388_start(ES_MODE_DAC);
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_ENABLE, OS_NULL);   
    
    return OS_EOK;
}

static os_err_t audio_es8388_player_stop(struct os_audio_device *audio)
{
    struct es8388_player_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_TX_DISABLE, OS_NULL);
    
    es8388_stop(ES_MODE_DAC);
        
    return OS_EOK;
}

os_size_t audio_es8388_transmit(struct os_audio_device *audio, const void *writeBuf, os_size_t size)
{
    struct es8388_player_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct es8388_player_device *)audio->parent.user_data;

    return os_device_write_nonblock(aduio_dev->data_bus, 0, (os_uint8_t *)writeBuf, size);
}

const static struct os_audio_ops es8388_player_ops =
{
    .getcaps            = OS_NULL,
    .configure          = audio_es8388_player_config,
    .init               = audio_es8388_player_init,
    .deinit             = audio_es8388_player_deinit,
    .start              = audio_es8388_player_start,
    .stop               = audio_es8388_player_stop,
    .transmit           = audio_es8388_transmit,
    .receive            = OS_NULL,
};

static os_err_t audio_es8388_recorder_config(struct os_audio_device *audio, struct os_audio_caps *caps)
{
    os_err_t result = OS_EOK;
    OS_ASSERT(audio != OS_NULL);
    
    struct es8388_recorder_device *aduio_dev;
    aduio_dev = (struct es8388_recorder_device *)audio->parent.user_data;
    switch (caps->config_type)
    {
        case AUDIO_PARAM_CMD:
        {
            
#ifdef OS_USING_SAI                 
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &caps->udata.config.channels);
#endif
            
#ifdef OS_USING_I2S            
            os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->read_config.samplerate);
#endif            
            aduio_dev->read_config.channels   = caps->udata.config.channels;
            LOG_D(DBG_EXT_TAG, "set samplerate %d", aduio_dev->read_config.channels);
            break;
        }

        default:
            result = OS_ERROR;
            break;
        }
               
    return result;
}

static os_err_t audio_es8388_recorder_init(struct os_audio_device *audio)  
{  
    os_err_t result = OS_EOK;
    OS_ASSERT(audio != OS_NULL);
        
    struct es8388_recorder_device *aduio_dev;
    aduio_dev = (struct es8388_recorder_device *)audio->parent.user_data;
    
    es8388_init(aduio_dev->cfg_bus, BSP_ES8388_POWER_PIN); 
    
#ifdef OS_USING_SAI
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_CHANNEL, &aduio_dev->read_config.channels); 
#endif    

#ifdef OS_USING_I2S    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_SET_FRQ, &aduio_dev->read_config.samplerate);
#endif
       
    return result;
}

static os_err_t audio_es8388_recorder_deinit(struct os_audio_device *audio)
{
    OS_ASSERT(audio != OS_NULL);

    struct es8388_recorder_device *aduio_dev = (struct es8388_recorder_device *)audio;

    return es8388_deinit(aduio_dev->cfg_bus);
}

static os_err_t audio_es8388_recorder_start(struct os_audio_device *audio)
{
    struct es8388_recorder_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_recorder_device *)audio->parent.user_data;
    
    LOG_D(DBG_EXT_TAG, "open sound device");
    
    es8388_start(ES_MODE_ADC);
    
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_RX_ENABLE, OS_NULL);

    return OS_EOK;
}

static os_err_t audio_es8388_recorder_stop(struct os_audio_device *audio)
{    
    struct es8388_recorder_device *aduio_dev;

    OS_ASSERT(audio != OS_NULL);
    aduio_dev = (struct es8388_recorder_device *)audio->parent.user_data;
        
    os_device_control(aduio_dev->data_bus, OS_AUDIO_CMD_RX_DISABLE, OS_NULL);
    
    es8388_stop(ES_MODE_ADC);
    
    return OS_EOK;
}

os_size_t audio_es8388_receive(struct os_audio_device *audio, void *readBuf, os_size_t size)
{
    struct es8388_recorder_device *aduio_dev;
    
    OS_ASSERT(audio != OS_NULL);
    
    aduio_dev = (struct es8388_recorder_device *)audio->parent.user_data;
        
    return os_device_read_nonblock(aduio_dev->data_bus, 0, readBuf, size); 
}

const static struct os_audio_ops es8388_recorder_ops =
{
    .getcaps            = OS_NULL,
    .configure          = audio_es8388_recorder_config,
    .init               = audio_es8388_recorder_init,
    .deinit             = audio_es8388_recorder_deinit,
    .start              = audio_es8388_recorder_start,
    .stop               = audio_es8388_recorder_stop,
    .transmit           = OS_NULL,
    .receive            = audio_es8388_receive,
};

static os_err_t audio_es8388_data_tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    if (dev->user_data != OS_NULL)
    {
        struct es8388_player_device *es8388_player_dev = dev->user_data;

        struct os_device_cb_info *dev_info = os_list_first_entry(&es8388_player_dev->audio.parent.cb_heads[OS_DEVICE_CB_TYPE_TX], struct os_device_cb_info, list);

        dev_info->data = info->data;

        os_device_send_notify(&es8388_player_dev->audio.parent);

        return OS_EOK;
    }
    
    return OS_ENOSYS;
}

int os_hw_audio_player_init(void)
{
    struct es8388_player_device *es8388_player_dev = os_calloc(1, sizeof(struct es8388_player_device));

    es8388_player_dev->replay_config.channels   = 2;
    es8388_player_dev->replay_config.samplebits = 16;
    
#if defined OS_USING_SAI_FOR_RECORDER    
    es8388_player_dev->replay_config.samplerate = OS_AUDIO_SAMPLERATE;
    es8388_player_dev->volume                   = OS_AUDIO_PLAYER_VOLUME;
#else 
    es8388_player_dev->replay_config.samplerate = 44100;
    es8388_player_dev->volume                   = 30;
#endif

    es8388_player_dev->audio.ops = &es8388_player_ops;
    
    es8388_player_dev->cfg_bus = os_device_find(BSP_ES8388_I2C_BUS);
    if (es8388_player_dev->cfg_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the config device!\r\n");
        return OS_ERROR;
    }
    
    es8388_player_dev->data_bus = os_device_find(BSP_AUDIO_DATA_TX_BUS);
    if (es8388_player_dev->data_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the data device!\r\n");
        return OS_ERROR;
    }
    es8388_player_dev->data_bus->user_data = es8388_player_dev;
    os_device_open(es8388_player_dev->data_bus);
    
    struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
    info->type = OS_DEVICE_CB_TYPE_TX;
    info->cb = audio_es8388_data_tx_done;
    os_device_control(es8388_player_dev->data_bus, OS_DEVICE_CTRL_SET_CB, info);
    
    os_audio_player_register(&es8388_player_dev->audio, "audio0", es8388_player_dev);

    return OS_EOK;
}
#if defined(OS_USING_SAI_FOR_PLAYER) || defined(OS_USING_I2S_FOR_PLAYER)
OS_DEVICE_INIT(os_hw_audio_player_init, OS_INIT_SUBLEVEL_LOW);
#endif

int os_hw_audio_recorder_init(void)
{
    struct es8388_recorder_device *es8388_recorder_dev = os_calloc(1, sizeof(struct es8388_recorder_device));
    
    es8388_recorder_dev->read_config.samplerate = 44100;
#if defined OS_USING_SAI_FOR_RECORDER    
    es8388_recorder_dev->read_config.channels   = OS_AUDIO_CHANNEL;
#else 
    es8388_recorder_dev->read_config.channels   = 2;
#endif
    es8388_recorder_dev->read_config.samplebits = 16;

    es8388_recorder_dev->audio.ops = &es8388_recorder_ops;
    
    es8388_recorder_dev->cfg_bus = os_device_find(BSP_ES8388_I2C_BUS);
    if (es8388_recorder_dev->cfg_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the config device!\r\n");
        return OS_ERROR;
    }
    
    es8388_recorder_dev->data_bus = os_device_find(BSP_AUDIO_DATA_RX_BUS);
    if (es8388_recorder_dev->data_bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "can not find the data device!\r\n");
        return OS_ERROR;
    }
    
    os_device_open(es8388_recorder_dev->data_bus); 
    
    os_audio_recorder_register(&es8388_recorder_dev->audio, "audio1", es8388_recorder_dev);

    os_pin_mode(BSP_ES8388_POWER_PIN, PIN_MODE_OUTPUT);

    return OS_EOK;
}
#if defined(OS_USING_SAI_FOR_RECORDER) || defined(OS_USING_I2S_FOR_RECORDER)
OS_DEVICE_INIT(os_hw_audio_recorder_init, OS_INIT_SUBLEVEL_LOW);
#endif

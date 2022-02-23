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
 * @file        audio.h
 *
 * @brief       This file implements audio related definitions and declarations.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "drv_cfg.h"
#include <os_memory.h>
#include <os_mutex.h>

/* AUDIO Control Command */
#define _AUDIO_CTL(a) (0x10 + a)

#define AUDIO_CTL_CONFIGURE                 _AUDIO_CTL(1)
#define AUDIO_CTL_START                     _AUDIO_CTL(2)
#define AUDIO_CTL_STOP                      _AUDIO_CTL(3)

/*Configure Command*/
#define AUDIO_PARAM_CMD                     0           /* set all params */
#define AUDIO_VOLUME_CMD                    1           /* set volume */

/* Supported Sampling Rates */
#define AUDIO_SAMP_RATE_8K                  0x0001
#define AUDIO_SAMP_RATE_11K                 0x0002
#define AUDIO_SAMP_RATE_16K                 0x0004
#define AUDIO_SAMP_RATE_22K                 0x0008
#define AUDIO_SAMP_RATE_32K                 0x0010
#define AUDIO_SAMP_RATE_44K                 0x0020
#define AUDIO_SAMP_RATE_48K                 0x0040
#define AUDIO_SAMP_RATE_96K                 0x0080
#define AUDIO_SAMP_RATE_128K                0x0100
#define AUDIO_SAMP_RATE_160K                0x0200
#define AUDIO_SAMP_RATE_172K                0x0400
#define AUDIO_SAMP_RATE_192K                0x0800

#define AUDIO_VOLUME_MAX                    (100)
#define AUDIO_VOLUME_MIN                    (0)


/* the preferred number and size of audio pipeline buffer for the audio device */
struct os_audio_buf_info
{
    os_uint8_t *buffer;
    os_uint16_t block_size;
    os_uint16_t block_count;
    os_uint32_t total_size;
};

struct os_audio_device;
struct os_audio_caps;
struct os_audio_configure;
struct os_audio_ops
{
    os_err_t (*getcaps)(struct os_audio_device *audio, struct os_audio_caps *caps);
    os_err_t (*configure)(struct os_audio_device *audio, struct os_audio_caps *caps);
    os_err_t (*init)(struct os_audio_device *audio);
    os_err_t (*deinit)(struct os_audio_device *audio);
    os_err_t (*start)(struct os_audio_device *audio);
    os_err_t (*stop)(struct os_audio_device *audio);
    os_size_t (*transmit)(struct os_audio_device *audio, const void *writeBuf, os_size_t size);
    os_size_t (*receive)(struct os_audio_device *audio, void *readBuf, os_size_t size);
};

struct os_audio_configure
{
    os_uint32_t samplerate;
    os_uint16_t channels;
    os_uint16_t samplebits;
};

struct os_audio_caps
{
    int config_type;
    
    union
    {
        int     value;
        struct  os_audio_configure config;
    } udata;
};

struct os_audio_replay
{
    struct os_mempool *mp;
    os_uint8_t *write_data;
    os_bool_t activated;
};

struct os_audio_record
{
    struct os_mutex lock; 
};

struct os_audio_device
{
    struct os_device           parent;
    const struct os_audio_ops *ops;
    struct os_audio_replay    *replay;
    struct os_audio_record    *record;
};

os_err_t    os_audio_player_register(struct os_audio_device *audio, const char *name, void *data);
os_err_t    os_audio_recorder_register(struct os_audio_device *audio, const char *name, void *data);
os_err_t    os_audio_tx_complete(struct os_audio_device *audio);
void        os_audio_rx_done(struct os_audio_device *audio, os_uint8_t *pbuf, os_size_t len);

#define CODEC_STANDARD                0x04
#define I2S_STANDARD                  I2S_STANDARD_PHILIPS

#define CODEC_VOLUME_MAX            (63)

#endif /* __AUDIO_H__ */

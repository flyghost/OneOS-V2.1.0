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
 * @file        audio_player.c
 *
 * @brief       This file provides functions for audio player device operations.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <device.h>
#include <string.h>

#include "drv_cfg.h"

#include "audio.h"

#define DBG_EXT_TAG              "audio_player"
#include <dlog.h>

os_err_t _audio_player_callback(os_device_t *dev, struct os_device_cb_info *info)
{
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(info != OS_NULL);
    struct os_audio_device *audio_dev = (struct os_audio_device *)dev;
    os_mp_free(audio_dev->replay->mp, info->data);

    dev->tx_count--;
    
    return OS_EOK;
}

static os_err_t _audio_player_init(struct os_device *dev)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    
    audio = (struct os_audio_device *) dev;

    /* initialize replay & record */
    audio->replay = OS_NULL;

    /* initialize replay */
    struct os_audio_replay *replay = (struct os_audio_replay *) os_calloc(1, sizeof(struct os_audio_replay));

    if (replay == OS_NULL)
        return OS_ENOMEM;
    
    memset(replay, 0, sizeof(struct os_audio_replay));

    /* init memory pool for replay */
    replay->mp = os_mp_create("replay_pool", OS_AUDIO_REPLAY_MP_BLOCK_COUNT, OS_AUDIO_REPLAY_MP_BLOCK_SIZE);
    if (replay->mp == OS_NULL)
    {
        os_free(replay);
        LOG_E(DBG_EXT_TAG, "create memory pool for repaly failed");
        return OS_ENOMEM;
    }

    dev->tx_count = 0;
    dev->tx_size  = OS_AUDIO_REPLAY_MP_BLOCK_COUNT;

    replay->activated = OS_FALSE;
    audio->replay = replay;
    
    if (audio->ops->init)
        audio->ops->init(audio);

    struct os_device_cb_info cb_info = {
        .type = OS_DEVICE_CB_TYPE_TX,
        .data = OS_NULL,
        .cb   = _audio_player_callback,
    };

    os_device_control(dev, OS_DEVICE_CTRL_SET_CB, &cb_info);

    return result;
}

static os_err_t _audio_player_deinit(struct os_device *dev)
{
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    if (audio->replay->activated == OS_TRUE)
    {
        /*wait for all data has been played!*/
        while (audio->replay->mp->blk_free_num != audio->replay->mp->blk_total_num)
        {
            os_task_msleep(10);
        }
        
        if (audio->ops->stop)
            audio->ops->stop(audio);

        audio->replay->activated = OS_FALSE;
        LOG_D(DBG_EXT_TAG, "stop audio replay device");
    }

    if (audio->ops->deinit)
        audio->ops->deinit(audio);

    os_mp_destroy(audio->replay->mp);
    os_free(audio->replay);

    dev->tx_count = 0;
    dev->tx_size  = 0;

    return OS_EOK;
}

static os_size_t _audio_player_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_size_t count = 0;
    os_base_t level;
    
    const os_uint8_t *buff = buffer;
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    
    audio = (struct os_audio_device *)dev;

    while (count < size)
    {
        level = os_irq_lock();
        
        if (dev->tx_count == dev->tx_size)
        {
            os_irq_unlock(level);
            return count;
        }

        dev->tx_count++;
        
        os_irq_unlock(level);

        audio->replay->write_data = os_mp_alloc(audio->replay->mp, OS_WAIT_FOREVER);

        if (size - count > OS_AUDIO_REPLAY_MP_BLOCK_SIZE)
        {
            memcpy(audio->replay->write_data, buff + count, OS_AUDIO_REPLAY_MP_BLOCK_SIZE);
            count += OS_AUDIO_REPLAY_MP_BLOCK_SIZE;
        }
        else
        {
            memcpy(audio->replay->write_data, buff + count, size - count);
            count += size - count;
        }

        if (audio->replay->activated != OS_TRUE)
        {
            if (audio->ops->start)
                audio->ops->start(audio);

            audio->replay->activated = OS_TRUE;
            LOG_D(DBG_EXT_TAG, "start audio replay device\r\n");
        }

        if (audio->ops->transmit)
        {
            audio->ops->transmit(audio, audio->replay->write_data, OS_AUDIO_REPLAY_MP_BLOCK_SIZE);
        }
    }

    return count;
}

static os_err_t _audio_player_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    switch (cmd)
    {
    case AUDIO_CTL_CONFIGURE:
    {
        struct os_audio_caps *caps = (struct os_audio_caps *) args;

        if (audio->ops->configure)
        {
            result = audio->ops->configure(audio, caps);
        }
        break;
    }
    case AUDIO_CTL_START:
    {
        if (audio->ops->start)
            result = audio->ops->start(audio);
        break;
    }
    case AUDIO_CTL_STOP:
    {
        if (audio->ops->stop)
            result = audio->ops->stop(audio);
        break;
    }

    default:
        break;
    }

    return result;
}

const static struct os_device_ops audio_ops =
{
    .init    = _audio_player_init,
    .deinit  = _audio_player_deinit,
    .write   = _audio_player_write,
    .control = _audio_player_control
};

os_err_t os_audio_player_register(struct os_audio_device *audio, const char *name, void *data)
{
    os_err_t result = OS_EOK;
    struct os_device *device;

    OS_ASSERT(audio != OS_NULL);
    device = &(audio->parent);

    device->type = OS_DEVICE_TYPE_SOUND;    
    device->ops  = &audio_ops;
    device->user_data = data;

    /* register a character device */
    result = os_device_register(device, name);

    return result;
}



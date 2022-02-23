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
 * @file        audio_recorder.c
 *
 * @brief       This file provides functions for audio recorder device operations.
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

#define DBG_TAG              "audio_recorder"
#include <dlog.h>

static os_err_t _audio_recorder_init(struct os_device *dev)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;

    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *)dev;

    audio->record = (struct os_audio_record *) os_calloc(1, sizeof(struct os_audio_record));

    OS_ASSERT(audio->record != OS_NULL);

    if (audio->ops->init)
        audio->ops->init(audio);

    if (audio->ops->start)
        audio->ops->start(audio);

    os_task_msleep(1000);

    LOG_D(DBG_TAG, "start audio record device\r\n");

    return result;
}

static os_err_t _audio_recorder_deinit(struct os_device *dev)
{
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    LOG_D(DBG_TAG, "stop audio replay device");

    if (audio->ops->stop)
        audio->ops->stop(audio);

    if (audio->ops->deinit)
        audio->ops->deinit(audio);

    os_free(audio->record);

    return OS_EOK;
}

static os_size_t _audio_recorder_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_audio_device *audio = (struct os_audio_device *)dev;

    OS_ASSERT(audio != OS_NULL);
    OS_ASSERT(audio->record != OS_NULL);

    if (audio->ops->receive(audio, buffer, size))
    {
        return size;            
    }
    
    return 0;
}

static os_err_t _audio_recorder_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t result = OS_EOK;
    struct os_audio_device *audio;
    OS_ASSERT(dev != OS_NULL);
    audio = (struct os_audio_device *) dev;

    /* dev stat...*/
    switch (cmd)
    {
    
    case AUDIO_CTL_CONFIGURE:
    {
        struct os_audio_caps *caps = (struct os_audio_caps *) args;

        if (audio->ops->configure != OS_NULL)
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
    .init    = _audio_recorder_init,
    .deinit  = _audio_recorder_deinit,
    .read    = _audio_recorder_read,
    .control = _audio_recorder_control
};

os_err_t os_audio_recorder_register(struct os_audio_device *audio, const char *name, void *data)
{
    os_err_t result = OS_EOK;
    struct os_device *device;

    OS_ASSERT(audio != OS_NULL);
    OS_ASSERT(audio->ops != OS_NULL);
    OS_ASSERT(audio->ops->receive != OS_NULL);
    
    device = &(audio->parent);

    device->type = OS_DEVICE_TYPE_SOUND;
    device->ops  = &audio_ops;
    device->user_data = data;

    /* register a character device */
    result = os_device_register(device, name);

    return result;
}



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
 * @file        i2s.c
 *
 * @brief       This file implements i2s driver for stm32.
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
#include <audio/i2s.h>
#include <dlog.h>
#define DBG_EXT_TAG "I2S"

void os_hw_i2s_isr(struct os_i2s_device *i2s, struct os_device_cb_info *info)
{
    os_device_t *dev = (os_device_t *)&i2s->parent;

    struct os_device_cb_info *dev_info = os_list_first_entry(&dev->cb_heads[OS_DEVICE_CB_TYPE_TX], struct os_device_cb_info, list);

    dev_info->data = info->data;

    os_device_send_notify(dev);
}

os_err_t i2s_init(os_device_t *dev)
{  
    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "i2s device not exist!\r\n");
        return OS_EEMPTY;
    }

    return i2s->ops->init(i2s);
}

os_size_t i2s_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_err_t status = OS_EOK;

    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "i2s device not exist!\r\n");
        return 0;
    }

    status = i2s->ops->receive(i2s, (os_uint8_t *)buffer, size);
    if (status != OS_EOK)
    {
        return 0;
    }
    
    return size;
}

os_size_t i2s_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_err_t status = OS_EOK;

    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "i2s device not exist!\r\n");
        return 0;
    }

    status = i2s->ops->transimit(i2s, (os_uint8_t *)buffer, size);
    if (status != OS_EOK)
    {
        return 0;
    }
    
    return size;
}

os_err_t i2s_control(os_device_t *dev, os_int32_t cmd, void *args)
{
    os_err_t status = OS_EOK;
    
    struct os_i2s_device *i2s = (struct os_i2s_device *)dev;
    if (i2s == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "i2s device not exist!\r\n");
        return OS_EEMPTY;
    }

    switch (cmd)
    {
    case OS_AUDIO_CMD_TX_ENABLE:
        status = i2s->ops->enable_tx(i2s, OS_TRUE);
        break;
    case OS_AUDIO_CMD_TX_DISABLE:
        status = i2s->ops->enable_tx(i2s, OS_FALSE);
        break;
    case OS_AUDIO_CMD_RX_ENABLE:
        status = i2s->ops->enable_rx(i2s, OS_TRUE);
        break;
    case OS_AUDIO_CMD_RX_DISABLE:
        status = i2s->ops->enable_rx(i2s, OS_FALSE);
        break;
    case OS_AUDIO_CMD_SET_FRQ:
        status = i2s->ops->set_frq(i2s, *(uint32_t *)args);
    case OS_AUDIO_CMD_SET_CHANNEL:
        status = i2s->ops->set_channel(i2s, *(uint8_t *)args);
        break;
    default:
        status = OS_ENOSYS;
        break;
    }

    return status;
}

const static struct os_device_ops i2s_ops = {
    .init    = i2s_init, 
    .read    = i2s_read,
    .write   = i2s_write,
    .control = i2s_control,
};

os_err_t os_i2s_register(struct os_i2s_device *i2s, const char *name, void *data)
{
    OS_ASSERT(i2s != OS_NULL);
    OS_ASSERT(i2s->ops != OS_NULL);
    
    i2s->parent.type  = OS_DEVICE_TYPE_SOUND;
    i2s->parent.ops  = &i2s_ops;
    
    return os_device_register(&i2s->parent, name);
}


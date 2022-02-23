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
 * @file        sai.c
 *
 * @brief       This file implements SAI driver for stm32.
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
#include <audio/sai.h>
#include <dlog.h>
#define DBG_EXT_TAG "audio.sai"

void os_hw_sai_isr(struct os_device_sai *sai, struct os_device_cb_info *info)
{
    os_device_t *dev = (os_device_t *)&sai->parent;

    struct os_device_cb_info *dev_info = os_list_first_entry(&dev->cb_heads[OS_DEVICE_CB_TYPE_TX], struct os_device_cb_info, list);

    dev_info->data = info->data;

    os_device_send_notify(dev);
}

os_size_t sai_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_err_t status = OS_EOK;

    struct os_device_sai *sai = (struct os_device_sai *)dev;
    if (sai == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "i2s device not exist!\r\n");
        return 0;
    }

    status = sai->ops->receive(sai, (os_uint8_t *)buffer, size);
    if (status != OS_EOK)
    {
        return 0;
    }
    
    return size;
}

os_size_t sai_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
	os_err_t status = OS_EOK;
	
    struct os_device_sai *sai = (struct os_device_sai *)dev;
    if (sai == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "i2s device not exist!\r\n");
        return 0;
    }

    status = sai->ops->transimit(sai, (os_uint8_t *)buffer, size);
    if (status != OS_EOK)
    {
        return 0;
    }
    
    return size;
}

os_err_t sai_control(os_device_t *dev, os_int32_t cmd, void *args)
{
    os_err_t status = OS_EOK;
    
    struct os_device_sai *sai = (struct os_device_sai *)dev;
    if (sai == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG, "sai device not exist!\r\n");
        return OS_EEMPTY;
    }
    
   switch (cmd)
    {
    case OS_AUDIO_CMD_TX_ENABLE:
        status = sai->ops->enable_tx(sai, OS_TRUE);
        break;
    case OS_AUDIO_CMD_TX_DISABLE:
        status = sai->ops->enable_tx(sai, OS_FALSE);
        break;
    case OS_AUDIO_CMD_RX_ENABLE:
        status = sai->ops->enable_rx(sai, OS_TRUE);
        break;
    case OS_AUDIO_CMD_RX_DISABLE:
        status = sai->ops->enable_rx(sai, OS_FALSE);
        break;
    case OS_AUDIO_CMD_SET_FRQ:
        status = sai->ops->set_frq(sai, *(uint32_t *)args);
        break;
    case OS_AUDIO_CMD_SET_CHANNEL:
        status = sai->ops->set_channel(sai, *(uint8_t *)args);
        break;
    default:
        status = OS_ENOSYS;
        break;
    }

    return status;
}

const static struct os_device_ops sai_ops = {
    .read    = sai_read,
    .write   = sai_write,
    .control = sai_control
};

os_err_t os_sai_register(const char *name, os_device_sai_t *sai)
{
    os_err_t          ret;
    
    OS_ASSERT(sai != OS_NULL);
    OS_ASSERT(sai->ops != OS_NULL);
    
    sai->parent.type  = OS_DEVICE_TYPE_SOUND;
    sai->parent.ops  = &sai_ops;
    
    ret = os_device_register(&sai->parent, name);
    
    return ret;
}


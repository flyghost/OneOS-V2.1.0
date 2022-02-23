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
 * @file        inputcapture.c
 *
 * @brief       this file implements inputcapture related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <device.h>
#include <board.h>
#include <ring_buff.h>
#include "misc/inputcapture.h"
#define DRV_EXT_TAG "capture"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>

#ifdef OS_USING_INPUT_CAPTURE
static os_err_t os_inputcapture_init(struct os_device *dev)
{
    os_err_t                       ret;
    struct os_inputcapture_device *inputcapture;

    OS_ASSERT(dev != OS_NULL);

    ret                     = OS_EOK;
    inputcapture            = (struct os_inputcapture_device *)dev;
    inputcapture->watermark = OS_INPUT_CAPTURE_RB_SIZE / 2;
    if (inputcapture->ops->init)
    {
        ret = inputcapture->ops->init(inputcapture);
    }

    return ret;
}

static os_err_t os_inputcapture_open(struct os_device *dev, os_uint16_t oflag)
{
    os_err_t                       ret;
    struct os_inputcapture_device *inputcapture;

    OS_ASSERT(dev != OS_NULL);

    ret          = OS_EOK;
    inputcapture = (struct os_inputcapture_device *)dev;
    if (inputcapture->ringbuff == OS_NULL)
    {
        inputcapture->ringbuff = rb_ring_buff_create(sizeof(struct os_inputcapture_data) * OS_INPUT_CAPTURE_RB_SIZE);
    }
    if (inputcapture->ops->open)
    {
        ret = inputcapture->ops->open(inputcapture);
    }

    return ret;
}

static os_err_t os_inputcapture_close(struct os_device *dev)
{
    os_err_t                       ret;
    struct os_inputcapture_device *inputcapture;

    OS_ASSERT(dev != OS_NULL);

    ret          = OS_ERROR;
    inputcapture = (struct os_inputcapture_device *)dev;

    if (inputcapture->ops->close)
    {
        ret = inputcapture->ops->close(inputcapture);
    }

    if (ret != OS_EOK)
    {
        return ret;
    }

    if (inputcapture->ringbuff)
    {
        rb_ring_buff_destroy(inputcapture->ringbuff);
        inputcapture->ringbuff = OS_NULL;
    }
    return ret;
}

static os_size_t os_inputcapture_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_size_t                      receive_size;
    struct os_inputcapture_device *inputcapture;

    OS_ASSERT(dev != OS_NULL);

    inputcapture = (struct os_inputcapture_device *)dev;
    receive_size =
        rb_ring_buff_get(inputcapture->ringbuff, (os_uint8_t *)buffer, sizeof(struct os_inputcapture_data) * size);

    return receive_size / sizeof(struct os_inputcapture_data);
}

static os_err_t os_inputcapture_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t                       result;
    struct os_inputcapture_device *inputcapture;

    OS_ASSERT(dev != OS_NULL);

    result       = OS_EOK;
    inputcapture = (struct os_inputcapture_device *)dev;
    switch (cmd)
    {
    case INPUTCAPTURE_CMD_CLEAR_BUF:
        if (inputcapture->ringbuff)
        {
            rb_ring_buff_reset(inputcapture->ringbuff);
        }
        break;
    case INPUTCAPTURE_CMD_SET_WATERMARK:
        inputcapture->watermark = *(os_size_t *)args;
        break;
    default:
        result = OS_ENOSYS;
        break;
    }

    return result;
}

const static struct os_device_ops inputcapture_ops = {
    os_inputcapture_init,
    os_inputcapture_open,
    os_inputcapture_close,
    os_inputcapture_read,
    OS_NULL,
    os_inputcapture_control,
};

/**
 ***********************************************************************************************************************
 * @brief           register inputcapture device
 *
 * @param[in]       inputcapture    pointer of inputcapture device
 * @param[in]       name            pointer of inputcapture name
 * @param[in]       user_data       user_data
 *
 * @return          os_err_t
 * @retval          OS_EOK          run successfully
 * @retval          OS_EINVAL       device or name is NULL or device not exist
 ***********************************************************************************************************************
 */
os_err_t os_device_inputcapture_register(struct os_inputcapture_device *inputcapture, const char *name, void *user_data)
{
    struct os_device *device;

    OS_ASSERT(inputcapture != OS_NULL);
    OS_ASSERT(inputcapture->ops != OS_NULL);
    OS_ASSERT(inputcapture->ops->get_pulsewidth != OS_NULL);

    device = &(inputcapture->parent);

    device->type           = OS_DEVICE_TYPE_MISCELLANEOUS;
    inputcapture->ringbuff = OS_NULL;

    device->ops = &inputcapture_ops;
    device->user_data = user_data;

    return os_device_register(device, name);
}

/**
 ***********************************************************************************************************************
 * @brief           isr for inputcapture interrupt
 *
 * @param[in]       inputcapture    pointer of inputcapture device
 * @param[in]       level       	pulse level flags:OS_TRUE-high level pulse;OS_FALSE-low level pulse
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_hw_inputcapture_isr(struct os_inputcapture_device *inputcapture, os_bool_t level)
{
    struct os_inputcapture_data data;
    os_size_t                   receive_size;
    if (inputcapture->ops->get_pulsewidth(inputcapture, &data.pulsewidth_us) != OS_EOK)
    {
        return;
    }

    data.is_high = level;
    if (rb_ring_buff_put(inputcapture->ringbuff, (os_uint8_t *)&data, sizeof(struct os_inputcapture_data)) == 0)
    {
        LOG_W(DBG_TAG,"inputcapture ringbuffer doesn't have enough space.");
    }

    receive_size = rb_ring_buff_data_len(inputcapture->ringbuff) / sizeof(struct os_inputcapture_data);

    if (receive_size >= inputcapture->watermark)
    {
        inputcapture->parent.rx_count = receive_size;
        os_device_recv_notify(&inputcapture->parent);
    }
}
#endif

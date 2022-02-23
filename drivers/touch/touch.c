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
 * @file        touch.c
 *
 * @brief       touch
 *
 * @details     touch
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <touch/touch.h>
#include <string.h>
#include <os_errno.h>
#include <pin/pin.h>
#include <os_assert.h>
#include <dlog.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "touch"
#include <drv_log.h>

static os_err_t os_touch_control(os_device_t *dev, int cmd, void *args)
{
    os_touch_t *touch;
    os_err_t    result = OS_EOK;
    OS_ASSERT(dev != OS_NULL);
    touch = (os_touch_t *)dev;

    switch (cmd)
    {
    case OS_TOUCH_CTRL_GET_ID:
        if (args)
            result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_GET_ID, args);
        else
            result = OS_ERROR;
        break;
    case OS_TOUCH_CTRL_GET_INFO:
        if (args)
            result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_GET_INFO, args);
        else
            result = OS_ERROR;
        break;
    case OS_TOUCH_CTRL_SET_MODE:
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_SET_MODE, args);
        break;
    case OS_TOUCH_CTRL_SET_X_RANGE:
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_SET_X_RANGE, args);
        if (result == OS_EOK)
        {
            touch->info.range_x = *(os_int32_t *)args;
            LOG_D(DRV_EXT_TAG, "set x coordinate range :%d\r\n", touch->info.range_x);
        }
        break;
    case OS_TOUCH_CTRL_SET_Y_RANGE:
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_SET_Y_RANGE, args);
        if (result == OS_EOK)
        {
            touch->info.range_y = *(os_uint32_t *)args;
            LOG_D(DRV_EXT_TAG, "set y coordinate range :%d \r\n", touch->info.range_y);
        }
        break;
    case OS_TOUCH_CTRL_SET_USER_X_RANGE:
        touch->info.user_range_x = *(os_int32_t *)args;
        LOG_D(DRV_EXT_TAG, "set user x coordinate range :%d\r\n", touch->info.user_range_x);
        break;
    case OS_TOUCH_CTRL_SET_USER_Y_RANGE:
        touch->info.user_range_y = *(os_uint32_t *)args;
        LOG_D(DRV_EXT_TAG, "set user y coordinate range :%d \r\n", touch->info.user_range_y);
        break;
    case OS_TOUCH_CTRL_DISABLE_INT:
        dev->rx_size = 0;
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_DISABLE_INT, args);
        break;
    case OS_TOUCH_CTRL_ENABLE_INT:
        dev->rx_size = 1;
        result = touch->ops->touch_control(touch, OS_TOUCH_CTRL_ENABLE_INT, args);
        break;
    default:
        return OS_ERROR;
    }

    return result;
}

static os_err_t os_touch_init(os_device_t *dev)
{
    OS_ASSERT(dev != OS_NULL);

    dev->rx_size  = 0;
    dev->rx_count = 0;

    return OS_EOK;
}

static os_err_t os_touch_deinit(os_device_t *dev)
{
    OS_ASSERT(dev != OS_NULL);

    /* touch disable interrupt */
    os_touch_control(dev, OS_TOUCH_CTRL_DISABLE_INT, 0);

    return OS_EOK;
}

static os_size_t os_touch_read(os_device_t *dev, os_off_t pos, void *buf, os_size_t len)
{
    os_size_t   result = 0;
    os_touch_t *touch;
    os_uint32_t range_x, range_y;
    struct os_touch_data *read_data;

    OS_ASSERT(dev != OS_NULL);
    touch = (os_touch_t *)dev;

    if (buf == NULL || len == 0)
    {
        return 0;
    }

    result = touch->ops->touch_readpoint(touch, buf, len);
    if(result == 0)
        return 0;

    dev->rx_count = 0;
    
    read_data = (struct os_touch_data *)buf;

    range_x = touch->info.range_x;
    range_y = touch->info.range_y;

#ifdef OS_TOUCH_X_REVERSE
    if(read_data->x_coordinate >= touch->info.range_x)
        read_data->x_coordinate = 0;
    else
        read_data->x_coordinate = touch->info.range_x - read_data->x_coordinate;
#endif

#ifdef OS_TOUCH_Y_REVERSE
    if(read_data->y_coordinate >= touch->info.range_y)
        read_data->y_coordinate = 0;
    else
        read_data->y_coordinate = touch->info.range_y - read_data->y_coordinate;
#endif

#ifdef OS_TOUCH_XY_SWAP
    os_uint16_t tmp;
    tmp = read_data->x_coordinate;
    read_data->x_coordinate = read_data->y_coordinate;
    read_data->y_coordinate = tmp;
    range_x = touch->info.range_y;
    range_y = touch->info.range_x;
#endif

    if (touch->info.range_x != touch->info.user_range_x)
        read_data->x_coordinate = read_data->x_coordinate * touch->info.user_range_x / range_x;

    if (touch->info.range_y != touch->info.user_range_y)
        read_data->y_coordinate = read_data->y_coordinate * touch->info.user_range_y / range_y;

    return result;
}


const static struct os_device_ops os_touch_ops =
{
    os_touch_init,
    os_touch_deinit,
    os_touch_read,
    OS_NULL,
    os_touch_control
};

void os_touch_irq_notify(os_touch_t *touch)
{
    OS_ASSERT(touch != OS_NULL);

    touch->parent.rx_count = 1;
    os_device_recv_notify(&touch->parent);
}

/**
 ***********************************************************************************************************************
 * @brief          register a touch device
 *
 * \@param[in]      touch           touch device
 * \@param[in]      name            touch device name
 * \@param[in]      flag            touch device flag
 * \@param[in]      data            user data
 *
 * \@return         error code
 * \@retval         OS_EOK          register success
 * \@retval         else            register fail
 ***********************************************************************************************************************
 */
int os_hw_touch_register(os_touch_t *touch, const char *name, void *data)
{
    os_int8_t    result;
    os_device_t *device;
    OS_ASSERT(touch != OS_NULL);

    touch->info.user_range_x = touch->info.range_x;
    touch->info.user_range_y = touch->info.range_y;

    device            = &touch->parent;
    device->ops       = &os_touch_ops;
    device->type      = OS_DEVICE_TYPE_TOUCH;
    device->user_data = data;

    result = os_device_register(device, name);

    if (result != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG,"os_touch register err code: %d", result);
        return result;
    }

    LOG_I(DRV_EXT_TAG,"os_touch init success");

    return OS_EOK;
}

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
 * @file        ft5406.c
 *
 * @brief       ft5406
 *
 * @details     ft5406
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_clock.h>
#include <i2c.h>
#include <string.h>
#include <stdlib.h>
#include <os_errno.h>
#include <os_memory.h>
#include <pin/pin.h>
#include <string.h>
#include <drv_gpio.h>
#define LOG_TAG "ft5406"
#define DBG_LVL DBG_INFO
#include <drv_log.h>

#include "touch.h"
#include "ft5406.h"

struct ft5406_touch {
    os_touch_t touch_device;
    struct os_i2c_bus_device *i2c_bus;
    os_uint16_t               i2c_addr;
    os_uint16_t id;
    /* field holding the current number of simultaneous active touches */
    os_uint8_t  currActiveTouchNb;
    /* field holding the touch index currently managed */
    os_uint8_t  currActiveTouchIdx;
};


typedef struct _ft5406_touch_point
{
    uint8_t XH;
    uint8_t XL;
    uint8_t YH;
    uint8_t YL;
    uint8_t RESERVED[2];
} ft5406_touch_point_t;

typedef struct _ft5406_touch_data
{
    uint8_t GEST_ID;
    uint8_t TD_STATUS;
    ft5406_touch_point_t TOUCH[FT5406_RT_MAX_TOUCHES];
} ft5406_touch_data_t;

#define TOUCH_POINT_GET_EVENT(T) ((touch_event_t)((T).XH >> 6))
#define TOUCH_POINT_GET_ID(T)    ((T).YH >> 4)
#define TOUCH_POINT_GET_X(T)     ((((T).XH & 0x0f) << 8) | (T).XL)
#define TOUCH_POINT_GET_Y(T)     ((((T).YH & 0x0f) << 8) | (T).YL)

static void I2C_IO_Write(struct ft5406_touch *ft5406, uint8_t reg, uint8_t value)
{
    unsigned char buff[2] = {reg, value};
    os_i2c_master_send(ft5406->i2c_bus, ft5406->i2c_addr, 0, buff, 2);
}

static uint8_t I2C_IO_Read(struct ft5406_touch *ft5406, uint8_t reg)
{
    unsigned char value;
    os_i2c_master_send(ft5406->i2c_bus, ft5406->i2c_addr, 0, &reg, 1);
    os_i2c_master_recv(ft5406->i2c_bus, ft5406->i2c_addr, 0, &value, 1);
    return value;
}

uint16_t I2C_IO_ReadMultiple(struct ft5406_touch *ft5406, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    os_i2c_master_send(ft5406->i2c_bus, ft5406->i2c_addr, 0, &reg, 1);
    os_i2c_master_recv(ft5406->i2c_bus, ft5406->i2c_addr, 0, buffer, length);
    return 0;
}

static void I2C_IO_Delay(uint32_t delay)
{
    os_task_msleep(delay);
}

static void ft5406_Start(struct ft5406_touch *ft5406)
{
    I2C_IO_Write(ft5406, 0, 0);
    I2C_IO_Delay(100);
}

static void ft5406_GetXY(struct ft5406_touch *ft5406, uint16_t *X, uint16_t *Y)
{
    uint8_t regAddress = 1;
    touch_event_t touch_event;
    ft5406_touch_data_t point;

    /* Read X and Y positions */
    I2C_IO_ReadMultiple(ft5406, regAddress,(uint8_t*)&point, sizeof(ft5406_touch_data_t));

    touch_event = TOUCH_POINT_GET_EVENT(point.TOUCH[0]);

    /* Update coordinates only if there is touch detected */
    if ((touch_event == kTouch_Down) || (touch_event == kTouch_Contact))
    {
        if (X)
        {
            *X = TOUCH_POINT_GET_X(point.TOUCH[0]);
        }
        if (Y)
        {
            *Y = TOUCH_POINT_GET_Y(point.TOUCH[0]);
        }
        ft5406->currActiveTouchIdx = 1;
    }

}

static void ft5406_GetMultiXY(struct ft5406_touch *ft5406,touch_point_t *touch_array)
{
    uint8_t                regAddress = 1;
    touch_event_t          touch_event;
    ft5406_touch_data_t    touch_data;
    uint16_t               status = 0;
    int                    touch_count =0;
    int                    i = 0;


    /* Read X and Y positions */
    I2C_IO_ReadMultiple(ft5406, regAddress, (uint8_t*)&touch_data, sizeof(ft5406_touch_data_t));

    /* Check  if there is touched evnet  */
    touch_event = TOUCH_POINT_GET_EVENT(touch_data.TOUCH[0]);
    if ((touch_event == kTouch_Down) || (touch_event == kTouch_Contact))
    {
        ft5406->currActiveTouchIdx = 1;
    }
    
    /* Check for valid number of touches - otherwise ignore touch information */
    if (touch_data.TD_STATUS > FT5406_RT_MAX_TOUCHES)
    {
        touch_data.TD_STATUS = 0;
    }

    /* Decode number of touches */
    touch_count = touch_data.TD_STATUS;

    /* Decode valid touch points */
    for (i = 0; i < touch_data.TD_STATUS; i++)
    {
        touch_array[i].TOUCH_ID    = TOUCH_POINT_GET_ID(touch_data.TOUCH[i]);
        touch_array[i].TOUCH_EVENT = TOUCH_POINT_GET_EVENT(touch_data.TOUCH[i]);
        touch_array[i].TOUCH_X     = TOUCH_POINT_GET_X(touch_data.TOUCH[i]);
        touch_array[i].TOUCH_Y     = TOUCH_POINT_GET_Y(touch_data.TOUCH[i]);
    }

}

static uint8_t ft5406_DetectTouch(struct ft5406_touch *ft5406)
{
    uint8_t nbTouch = ft5406->currActiveTouchIdx;

    ft5406->currActiveTouchIdx = 0;
    return nbTouch;
}

static os_size_t ft5406_read_point(struct os_touch_device *touch, struct os_touch_data *data, os_size_t read_num)
{
    uint16_t input_x  = 0;
    uint16_t input_y  = 0;
    uint16_t input_w  = 0;
    uint16_t new_data = 0;

    struct ft5406_touch *ft5406 = (struct ft5406_touch *)touch;

    ft5406_GetXY(ft5406, &input_x, &input_y);
    //os_kprintf("%d,%d \n",input_x,input_y);

    if (0 == ft5406_DetectTouch(ft5406))
        return 0;

    data->event        = OS_TOUCH_EVENT_DOWN;
    data->timestamp    = os_touch_get_ts();
    data->width        = input_w;
    data->x_coordinate = input_x;
    data->y_coordinate = input_y;
    data->track_id     = 0;

    return 1;
}

static os_err_t ft5406_control(struct os_touch_device *device, int cmd, void *data)
{
    struct ft5406_touch *ft6x06 = (struct ft5406_touch *)device;

    switch (cmd)
    {
    case OS_TOUCH_CTRL_GET_ID:
        break;
    case OS_TOUCH_CTRL_GET_INFO:
        *(struct os_touch_info *)data = device->info;
        break;
    case OS_TOUCH_CTRL_SET_MODE: /* change int trig type */
        break;
    default:
        break;
    }

    return OS_EOK;
}


static struct os_touch_ops ft5406_touch_ops =
{
    .touch_readpoint = ft5406_read_point,
    .touch_control   = ft5406_control,
};

static int os_hw_ft5406_init(void)
{
    struct ft5406_touch *ft5406 = os_calloc(1, sizeof(struct ft5406_touch));
    OS_ASSERT(ft5406);

    ft5406->i2c_addr = FT5406_RT_I2C_ADDRESS;
    ft5406->i2c_bus  = os_i2c_bus_device_find(OS_FT5406_I2C_BUS_NAME);
    OS_ASSERT(ft5406->i2c_bus);

    ft5406_Start(ft5406);
    
    os_touch_t *touch_device = &ft5406->touch_device;
    
    /* register touch device */
    touch_device->info.type      = OS_TOUCH_TYPE_CAPACITANCE;
    touch_device->info.vendor    = OS_TOUCH_VENDOR_UNKNOWN;
    touch_device->info.point_num = 1;
    touch_device->info.range_x   = 272;
    touch_device->info.range_y   = 480;
    touch_device->ops            = &ft5406_touch_ops;
    
    os_hw_touch_register(touch_device, "touch", ft5406);
    
    return 0;
}

OS_CMPOENT_INIT(os_hw_ft5406_init, OS_INIT_SUBLEVEL_LOW);


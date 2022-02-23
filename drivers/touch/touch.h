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
 * @file        touch.h
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

#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <os_task.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_RTC
#define os_touch_get_ts() time(OS_NULL) /* API for the touch to get the timestamp */
#else
#define os_touch_get_ts() os_tick_get() /* API for the touch to get the timestamp */
#endif

#define OS_PIN_NONE 0xFFFF /* RT PIN NONE */

/* Touch vendor types */
#define OS_TOUCH_VENDOR_UNKNOWN (0) /* unknown */
#define OS_TOUCH_VENDOR_GT      (1) /* GTxx series */
#define OS_TOUCH_VENDOR_FT      (2) /* FTxx series */

/* Touch ic type*/
#define OS_TOUCH_TYPE_NONE        (0) /* touch ic none */
#define OS_TOUCH_TYPE_CAPACITANCE (1) /* capacitance ic */
#define OS_TOUCH_TYPE_RESISTANCE  (2) /* resistance ic */

/* Touch control cmd types */
#define OS_TOUCH_CTRL_GET_ID            (0) /* Get device id */
#define OS_TOUCH_CTRL_GET_INFO          (1) /* Get touch info */
#define OS_TOUCH_CTRL_SET_MODE          (2) /* Set touch's work mode. ex. OS_TOUCH_MODE_POLLING,OS_TOUCH_MODE_INT */
#define OS_TOUCH_CTRL_SET_X_RANGE       (3) /* Set x coordinate range */
#define OS_TOUCH_CTRL_SET_Y_RANGE       (4) /* Set y coordinate range */
#define OS_TOUCH_CTRL_SET_USER_X_RANGE  (5) /* Set x coordinate range */
#define OS_TOUCH_CTRL_SET_USER_Y_RANGE  (6) /* Set y coordinate range */
#define OS_TOUCH_CTRL_SET_X_TO_Y        (7) /* Set X Y coordinate exchange */
#define OS_TOUCH_CTRL_DISABLE_INT       (8) /* Disable interrupt */
#define OS_TOUCH_CTRL_ENABLE_INT        (9) /* Enable interrupt */

/* Touch event */
#define OS_TOUCH_EVENT_NONE (0) /* Touch none */
#define OS_TOUCH_EVENT_UP   (1) /* Touch up event */
#define OS_TOUCH_EVENT_DOWN (2) /* Touch down event */
#define OS_TOUCH_EVENT_MOVE (3) /* Touch move event */

struct os_touch_info
{
    os_uint8_t type;        /* The touch type */
    os_uint8_t vendor;      /* Vendor of touchs */
    os_uint8_t point_num;   /* Support point num */
    os_int32_t range_x;     /* X coordinate range */
    os_int32_t range_y;     /* Y coordinate range */
    os_int32_t user_range_x;/* X coordinate range defined by user*/
    os_int32_t user_range_y;/* Y coordinate range defined by user*/
};

struct os_touch_config
{
    struct os_device_pin_mode *irq_pin;  /* Interrupt pin, The purpose of this pin is to notification read data */
    char                      *dev_name; /* The name of the communication device */
    void                      *user_data;
};

typedef struct os_touch_device os_touch_t;
struct os_touch_device
{
    struct os_device       parent; /* The standard device */
    struct os_touch_info   info;   /* The touch info data */
    struct os_touch_config config; /* The touch config data */

    const struct os_touch_ops *ops;            /* The touch ops */
    os_err_t (*irq_handle)(os_touch_t *touch); /* Called when an interrupt is generated, registered by the driver */
};

struct os_touch_data
{
    os_uint8_t  event;        /* The touch event of the data */
    os_uint8_t  track_id;     /* Track id of point */
    os_uint8_t  width;        /* Point of width */
    os_uint16_t x_coordinate; /* Point of x coordinate */
    os_uint16_t y_coordinate; /* Point of y coordinate */
    os_tick_t   timestamp;    /* The timestamp when the data was received */
};

struct os_touch_ops
{
    os_size_t (*touch_readpoint)(struct os_touch_device *touch, struct os_touch_data *data, os_size_t touch_num);
    os_err_t  (*touch_control)(struct os_touch_device *touch, int cmd, void *arg);
};

void os_touch_irq_notify(os_touch_t *touch);
int os_hw_touch_register(os_touch_t *touch, const char *name, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __TOUCH_H__ */

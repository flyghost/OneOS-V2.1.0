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
 * @file        gt9147_sample.c
 *
 * @brief       gt9147_sample
 *
 * @details     gt9147_sample
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_ipc.h>
#include <os_sem.h>
#include <os_memory.h>
#include <os_errno.h>
#include "gt9147.h"
#ifdef OS_USING_SHELL
#include <shell.h>
#endif

#define THREAD_PRIORITY   25
#define THREAD_STACK_SIZE 1024
#define THREAD_TIMESLICE  5

static os_task_t *           gt9147_task = OS_NULL;
static os_sem_t *            gt9147_sem  = OS_NULL;
static os_device_t *         dev         = OS_NULL;
static struct os_touch_data *read_data;
static struct os_touch_info  info;

static void gt9147_entry(void *parameter)
{
    os_device_control(dev, OS_TOUCH_CTRL_GET_INFO, &info);

    read_data = (struct os_touch_data *)os_calloc(1, sizeof(struct os_touch_data) * info.point_num);

    while (1)
    {
        os_sem_wait(gt9147_sem, OS_WAIT_FOREVER);

        if (os_device_read_nonblock(dev, 0, read_data, info.point_num) == info.point_num)
        {
            for (os_uint8_t i = 0; i < info.point_num; i++)
            {
                if (read_data[i].event == OS_TOUCH_EVENT_DOWN || read_data[i].event == OS_TOUCH_EVENT_MOVE)
                {
                    os_kprintf("%d %d %d %d %d\r\n",
                               read_data[i].track_id,
                               read_data[i].x_coordinate,
                               read_data[i].y_coordinate,
                               read_data[i].timestamp,
                               read_data[i].width);
                }
            }
        }
        os_device_control(dev, OS_TOUCH_CTRL_ENABLE_INT, OS_NULL);
    }
}

static os_err_t rx_callback(os_device_t *dev, struct os_device_cb_info *info)
{
    os_sem_post(gt9147_sem);
    os_device_control(dev, OS_TOUCH_CTRL_DISABLE_INT, OS_NULL);
    return 0;
}

/* Test function */
int gt9147_sample(const char *name, os_uint16_t x, os_uint16_t y)
{
    void *id;

    dev = os_device_find(name);
    if (dev == OS_NULL)
    {
        os_kprintf("can't find device:%s\r\n", name);
        return -1;
    }

    if (os_device_open(dev) != OS_EOK)
    {
        os_kprintf("open device failed!");
        return -1;
    }

    id = os_calloc(1, sizeof(os_uint8_t) * 8);
    os_device_control(dev, OS_TOUCH_CTRL_GET_ID, id);
    os_uint8_t *read_id = (os_uint8_t *)id;
    os_kprintf("id = %d %d %d %d \r\n", read_id[0] - '0', read_id[1] - '0', read_id[2] - '0', read_id[3] - '0');

    os_device_control(dev, OS_TOUCH_CTRL_SET_X_RANGE, &x); /* if possible you can set your x y coordinate*/
    os_device_control(dev, OS_TOUCH_CTRL_SET_Y_RANGE, &y);
    os_device_control(dev, OS_TOUCH_CTRL_GET_INFO, id);
    os_kprintf("range_x = %d \r\n", (*(struct os_touch_info *)id).range_x);
    os_kprintf("range_y = %d \r\n", (*(struct os_touch_info *)id).range_y);
    os_kprintf("point_num = %d \r\n", (*(struct os_touch_info *)id).point_num);
    os_free(id);

    struct os_device_cb_info cb_info = 
    {
        .type = OS_DEVICE_CB_TYPE_RX,
        .cb   = rx_callback,
    };

    os_device_control(dev, OS_DEVICE_CTRL_SET_CB, &cb_info);
    
    gt9147_sem = os_sem_create("dsem", 0, OS_IPC_FLAG_FIFO);

    if (gt9147_sem == OS_NULL)
    {
        os_kprintf("create dynamic semaphore failed.\r\n");
        return -1;
    }

    gt9147_task = os_task_create("task1", gt9147_entry, OS_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);

    if (gt9147_task != OS_NULL)
        os_task_startup(gt9147_task);

    return 0;
}

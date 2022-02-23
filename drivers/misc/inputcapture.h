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
 * @file        inputcapture.h
 *
 * @brief       this file implements inputcapture related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __OS_INPUT_CAPTURE_H__
#define __OS_INPUT_CAPTURE_H__

#include <os_assert.h>
#include <os_errno.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INPUTCAPTURE_CMD_CLEAR_BUF     (128 + 0) /* clear capture buf */
#define INPUTCAPTURE_CMD_SET_WATERMARK (128 + 1) /* Set the callback threshold */

struct os_inputcapture_data
{
    os_uint32_t pulsewidth_us;
    os_bool_t   is_high;
};

struct os_inputcapture_device
{
    os_size_t        watermark;
    struct os_device parent;

    const struct os_inputcapture_ops *ops;
	
    struct rb_ring_buff *ringbuff;
    
};

struct os_inputcapture_ops
{
    os_err_t (*init)(struct os_inputcapture_device *inputcapture);
    os_err_t (*open)(struct os_inputcapture_device *inputcapture);
    os_err_t (*close)(struct os_inputcapture_device *inputcapture);
    os_err_t (*get_pulsewidth)(struct os_inputcapture_device *inputcapture, os_uint32_t *pulsewidth_us);
};

void os_hw_capture_isr(struct os_inputcapture_device *inputcapture, os_bool_t level);

os_err_t os_device_inputcapture_register(struct os_inputcapture_device *inputcapture, const char *name, void *data);
#ifdef __cplusplus
}
#endif

#endif /* __OS_INPUT_CAPTURE_H__ */

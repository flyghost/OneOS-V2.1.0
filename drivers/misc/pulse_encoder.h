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
 * @file        pulse_encoder.h
 *
 * @brief       this file implements pulse_encoder related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __PULSE_ENCODER_H__
#define __PULSE_ENCODER_H__

#include <os_assert.h>
#include <os_errno.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PULSE_ENCODER_CMD_GET_TYPE    IOC_ENCODER(0x00) /* get a pulse_encoder type information */
#define PULSE_ENCODER_CMD_ENABLE      IOC_ENCODER(0x01) /* enable pulse_encoder */
#define PULSE_ENCODER_CMD_DISABLE     IOC_ENCODER(0x02) /* disable pulse_encoder */
#define PULSE_ENCODER_CMD_CLEAR_COUNT IOC_ENCODER(0x03) /* clear pulse_encoder count */
#define PULSE_ENCODER_CMD_SET_PERIOD  IOC_ENCODER(0x04) /* set pulse_encoder period */

enum os_pulse_encoder_type
{
    UNKNOWN_PULSE_ENCODER_TYPE = 0x00, /* Unknown pulse_encoder type */
    SINGLE_PHASE_PULSE_ENCODER,        /* single phase pulse_encoder */
    AB_PHASE_PULSE_ENCODER             /* two phase pulse_encoder */
};

struct os_pulse_encoder_device;

struct os_pulse_encoder_ops
{
    os_err_t (*enabled)(struct os_pulse_encoder_device *pulse_encoder, os_bool_t enable);
    os_err_t (*get_count)(struct os_pulse_encoder_device *pulse_encoder, os_int16_t *buffer);
    os_err_t (*control)(struct os_pulse_encoder_device *pulse_encoder, os_uint32_t cmd, void *args);
};

struct os_pulse_encoder_device
{
    struct os_device                   parent;
    const struct os_pulse_encoder_ops *ops;
    enum os_pulse_encoder_type         type;
    os_int32_t over_under_flowcount;
    os_uint32_t max_period;
    os_uint32_t period;
    os_uint8_t dir;
};

typedef struct os_pulse_encoder_device os_pulse_encoder_device_t;

os_err_t os_pulse_encoder_enable(struct os_pulse_encoder_device *pulse_encoder);
os_err_t os_pulse_encoder_disable(struct os_pulse_encoder_device *pulse_encoder);
os_err_t os_pulse_encoder_clear(struct os_pulse_encoder_device *pulse_encoder);
os_err_t os_pulse_encoder_read(struct os_pulse_encoder_device *pulse_encoder, os_int32_t *buffer);
os_err_t os_pulse_encoder_set_period(struct os_pulse_encoder_device *pulse_encoder, os_uint32_t period);
os_err_t
os_device_pulse_encoder_register(struct os_pulse_encoder_device       *device,
                                const char              *name);


#ifdef __cplusplus
}
#endif

#endif /* __PULSE_ENCODER_H__ */

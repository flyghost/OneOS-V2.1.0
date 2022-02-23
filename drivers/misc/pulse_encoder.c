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
 * @file        pulse_encoder.c
 *
 * @brief       this file implements pulse_encoder related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <device.h>
#include "misc/pulse_encoder.h"

os_err_t os_pulse_encoder_enable(struct os_pulse_encoder_device *pulse_encoder)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(pulse_encoder != OS_NULL);

    result = pulse_encoder->ops->enabled(pulse_encoder, OS_TRUE);

    return result;
}

os_err_t os_pulse_encoder_disable(struct os_pulse_encoder_device *pulse_encoder)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(pulse_encoder != OS_NULL);

    result = pulse_encoder->ops->enabled(pulse_encoder, OS_FALSE);

    return result;
}

os_err_t os_pulse_encoder_read(struct os_pulse_encoder_device *pulse_encoder, os_int32_t *buffer)
{
    os_err_t result = OS_EOK;
    os_int16_t cur_encoder_count = 0;
    
    result = pulse_encoder->ops->get_count(pulse_encoder, &cur_encoder_count);
    
    if (pulse_encoder->over_under_flowcount < 0)
    {
        *buffer = cur_encoder_count + (pulse_encoder->over_under_flowcount + 1) * pulse_encoder->period;
    }
    else
    {
        *buffer = cur_encoder_count + pulse_encoder->over_under_flowcount * pulse_encoder->period;
    }

    return result;
}

os_err_t os_pulse_encoder_clear(struct os_pulse_encoder_device *pulse_encoder)
{  
    return  pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_CLEAR_COUNT, OS_NULL);
}

os_err_t os_pulse_encoder_set_period(struct os_pulse_encoder_device *pulse_encoder, os_uint32_t period)
{    
    return  pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_SET_PERIOD, &period);
}

static os_size_t _pulse_encoder_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{ 
    return os_pulse_encoder_read((struct os_pulse_encoder_device *)dev, (os_int32_t *)buffer);
}

static os_err_t _pulse_encoder_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t                        result = OS_EOK;
    
    struct os_pulse_encoder_device *pulse_encoder = (struct os_pulse_encoder_device *)dev;

    switch (cmd)
    {
    case PULSE_ENCODER_CMD_CLEAR_COUNT:
        result = pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_CLEAR_COUNT, OS_NULL);
        break;
    case PULSE_ENCODER_CMD_SET_PERIOD:
        result = pulse_encoder->ops->control(pulse_encoder, PULSE_ENCODER_CMD_SET_PERIOD, (os_uint32_t *)args);
        break;
    case PULSE_ENCODER_CMD_ENABLE:
        result = pulse_encoder->ops->enabled(pulse_encoder, OS_TRUE);
        break;
    case PULSE_ENCODER_CMD_DISABLE:
        result = pulse_encoder->ops->enabled(pulse_encoder, OS_FALSE);
        break;
    default:
        result = OS_ENOSYS;
        break;
    }

    return result;
}

const static struct os_device_ops pulse_encoder_ops = {
    .read    = _pulse_encoder_read,
    .control = _pulse_encoder_control
};

/**
 ***********************************************************************************************************************
 * @brief           register pulse_encoder device
 *
 * @param[in]       pulse_encoder   pointer of pulse_encoder device
 * @param[in]       name            pointer of pulse_encoder name
 * @param[in]       user_data       not used
 *
 * @return          os_err_t
 * @retval          OS_EOK          run successfully
 * @retval          OS_EINVAL       pointer of device or name is NULL or device not exist
 ***********************************************************************************************************************
 */
os_err_t
os_device_pulse_encoder_register(struct os_pulse_encoder_device          *pulse_encoder,
                                const char                              *name)
{
    struct os_device *device;

    OS_ASSERT(pulse_encoder != OS_NULL);
    OS_ASSERT(pulse_encoder->ops != OS_NULL);

    device = &(pulse_encoder->parent);

    device->type = OS_DEVICE_TYPE_MISCELLANEOUS;
    device->ops  = &pulse_encoder_ops;

    pulse_encoder->over_under_flowcount = 0;

    return os_device_register(device, name);
}

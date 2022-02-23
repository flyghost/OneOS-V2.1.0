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
 * @file        dac.c
 *
 * @brief       this file implements dac related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <driver.h>
#include <misc/dac.h>

os_err_t os_dac_write(os_dac_device_t *dac, os_uint32_t channel, os_int32_t mv)
{
    if (mv > dac->ref_hight || mv < dac->ref_low)
    {
        os_kprintf("invalid dac param %s, %d, %d.\r\n", device_name(&dac->parent), channel, mv);
        return OS_EINVAL;
    }

    os_uint32_t value = os_dac_mv2value(dac, mv);

    if (value > dac->max_value)
        value = dac->max_value;

    return dac->ops->write(dac, channel, value);
}

static os_size_t _dac_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    return os_dac_write((struct os_dac_device *)dev, pos, *(os_int32_t *)buffer);
}

static os_err_t _dac_control(os_device_t *dev, int cmd, void *args)
{
    os_dac_device_t *dac = (struct os_dac_device *)dev;

    if (cmd == OS_DAC_CMD_ENABLE)
    {
        return dac->ops->enabled(dac, (os_uint32_t)args, OS_TRUE);
    }
    else if (cmd == OS_DAC_CMD_DISABLE)
    {
        return dac->ops->enabled(dac, (os_uint32_t)args, OS_FALSE);
    }

    return OS_ENOSYS;
}

const static struct os_device_ops dac_ops = {
    .write   = _dac_write,
    .control = _dac_control,
};

/**
 ***********************************************************************************************************************
 * @brief           register dac device
 *
 * @param[in]       device          pointer of dac device
 * @param[in]       name            pointer of dac name
 * @param[in]       ops             pointer of dac operation function set
 * @param[in]       user_data       pointer of DAC_Handler
 *
 * @return          os_err_t
 * @retval          OS_EOK          run successfully
 * @retval          OS_EINVAL       pointer of device or name is NULL or device not exist
 ***********************************************************************************************************************
 */
os_err_t os_dac_register(os_dac_device_t *dac, const char *name, const void *user_data)
{
    OS_ASSERT(dac->ops && dac->ops->write && dac->ops->enabled && dac->max_value);

    calc_mult_shift(&dac->mult, &dac->shift, dac->ref_hight - dac->ref_low, dac->max_value, 1);

    os_kprintf("dac register ref:[%d, %d], value:%u, mult:%u, shift:%u.\r\n",
               dac->ref_low, dac->ref_hight, dac->max_value, dac->mult, dac->shift);

    dac->parent.ops       = &dac_ops;
    dac->parent.type      = OS_DEVICE_TYPE_MISCELLANEOUS;
    dac->parent.user_data = (void *)user_data;
    return os_device_register(&dac->parent, name);
}


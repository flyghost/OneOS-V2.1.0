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
 * @file        dac.h
 *
 * @brief       this file implements dac related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DAC_H__
#define __DAC_H__
#include <os_assert.h>
#include <os_errno.h>
#include <device.h>

typedef struct os_dac_device os_dac_device_t;
struct os_dac_ops
{
    os_err_t (*enabled)(os_dac_device_t *dac, os_uint32_t channel, os_bool_t enabled);
    os_err_t (*write)(os_dac_device_t *dac, os_uint32_t channel, os_uint32_t value);
};

struct os_dac_device
{
    struct os_device         parent;
    const struct os_dac_ops *ops;

    os_int32_t  ref_hight;
    os_int32_t  ref_low;
    os_uint32_t max_value;

    os_uint32_t mult;       /* dac value = voltage(mV) * mult >> shift */
    os_uint32_t shift;
};

#define OS_DAC_CMD_ENABLE   IOC_MISCELLANEOUS(1)
#define OS_DAC_CMD_DISABLE  IOC_MISCELLANEOUS(2)

os_err_t os_dac_register(os_dac_device_t *dac, const char *name, const void *user_data);
os_err_t os_dac_write(os_dac_device_t *dac, os_uint32_t channel, os_int32_t mv);

#define os_dac_mv2value(dac, mv)        ((os_uint64_t)(mv - dac->ref_low) * dac->mult >> dac->shift)
#define os_dac_enable(dev, channel)     (dev)->ops->enabled(dev, channel, OS_TRUE)
#define os_dac_disable(dev, channel)    (dev)->ops->enabled(dev, channel, OS_FALSE)

#endif /* __DAC_H__ */

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
 * @file        adc.c
 *
 * @brief       this file implements adc related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <os_util.h>
#include <os_assert.h>
#include <drv_cfg.h>
#include <dlog.h>

#include "adc.h"

#define DBG_TAG "adc"
#define DBG_LVL DBG_INFO

void os_hw_adc_isr(struct os_adc_device *adc, int event)
{   
    os_device_t  *dev = (os_device_t  *)&adc->parent;
        
    switch (event & 0xff)
    {
    case OS_ADC_EVENT_CONVERT_DONE:
    {
        os_device_recv_notify(dev);
        break;
    }
    default:
        break;
    }    
}

os_err_t os_adc_read(struct os_adc_device *adc, os_uint32_t channel, os_int32_t *buffer)
{
    OS_ASSERT(adc != OS_NULL);

    return adc->ops->adc_read(adc, channel, buffer);
}

os_err_t os_adc_enable(struct os_adc_device *adc)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(adc != OS_NULL);

    result = adc->ops->adc_enabled(adc, OS_TRUE);

    return result;
}

os_err_t os_adc_disable(struct os_adc_device *adc)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(adc != OS_NULL);
    
    result = adc->ops->adc_enabled(adc, OS_FALSE);

    return result;
}

static os_size_t _adc_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    int i;
    os_err_t ret;

    if ((size % sizeof(os_int32_t)) != 0)
    {
        os_kprintf("invalid adc buff size.\r\n");
        return 0;
    }

    for (i = 0; i < size; i += sizeof(os_int32_t))
    {
        ret = os_adc_read((struct os_adc_device *)dev, pos, (os_int32_t *)((unsigned long)buffer + i));
        if (ret != OS_EOK)
        {
            os_kprintf("adc read failed %d, %d\r\n", i, size);
            break;
        }
    }

    return i;
}

static os_err_t _adc_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t        result = OS_EOK;
    
    struct os_adc_device *adc;

    OS_ASSERT(dev != OS_NULL);
    
    adc = (struct os_adc_device *)dev;

    switch (cmd)
    {
    case OS_ADC_CMD_ENABLE:
        result = adc->ops->adc_enabled(adc, OS_TRUE);
        break;
    case OS_ADC_CMD_DISABLE:
        result = adc->ops->adc_enabled(adc, OS_FALSE);
        break;
    default:
    /* Control device */
        result = OS_ENOSYS;
        break;
    }

    return result;
}

const static struct os_device_ops adc_ops = {
    .read    = _adc_read,
    .control = _adc_control,
};

/**
 ***********************************************************************************************************************
 * @brief           register adc device
 *
 * @param[in]       device          pointer of adc device
 * @param[in]       name            pointer of adc name
 * @param[in]       ops             pointer of adc operation function set
 * @param[in]       user_data       pointer of ADC_Handler
 *
 * @return          os_err_t
 * @retval          OS_EOK          run successfully
 * @retval          OS_EINVAL       pointer of device or name is NULL or device not exist
 ***********************************************************************************************************************
 */
os_err_t
os_hw_adc_register(struct os_adc_device *device, const char *name, void *data)
{
    os_err_t result = OS_EOK;
    OS_ASSERT(device != OS_NULL);

    calc_mult_shift(&device->mult, &device->shift, device->max_value, device->ref_hight - device->ref_low, 1);

    device->parent.type      = OS_DEVICE_TYPE_MISCELLANEOUS;
    device->parent.ops       = &adc_ops;
    device->parent.user_data = device;

    result = os_device_register(&device->parent, name);

    return result;
}


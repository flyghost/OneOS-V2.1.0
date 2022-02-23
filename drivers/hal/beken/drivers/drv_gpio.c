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
 * @file        drv_gpio.c
 *
 * @brief       The driver file for gpio.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-09   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_dbg.h>
#include <os_stddef.h>
#include <os_assert.h>

#include "interrupt.h"
#include "typedef.h"
#include "drv_gpio.h"
#include "gpio.h"
#include "pin.h"
#include "gpio_pub.h"


#ifdef BEKEN_USING_GPIO

#define DBG_SECTION_NAME  "[GPIO]"
extern void *os_memset(void *buff, os_uint8_t val, os_size_t count);

typedef void (*gpio_isr_handler_t)(void *param);

struct gpio_irq_desc
{
    gpio_isr_handler_t handler;
    void              *param;
    os_base_t          mode;
};

struct _gpio_dev
{
    struct os_device *parent;
    struct gpio_irq_desc irq_desc[GPIONUM];
};

static struct _gpio_dev gpio_dev;

static void gpio_irq_dispatch(unsigned char index)
{
    struct _gpio_dev *_dev = &gpio_dev;

    LOG_D(DBG_SECTION_NAME, "%s run...\n", __FUNCTION__);
    if (index >= GPIONUM)
    {
        LOG_D(DBG_ERROR, "%s index[%d] Wrongful\n", __FUNCTION__, index);
        return;
    }
    if (_dev->irq_desc[index].handler != OS_NULL)
    {
        LOG_D(DBG_SECTION_NAME, "gpio irq pin:%d\n", index);
        _dev->irq_desc[index].handler(_dev->irq_desc[index].param);
    }
}

static void _gpio_mode(struct os_device *device, os_base_t pin, os_base_t mode)
{
    OS_ASSERT(device != OS_NULL);

    switch (mode)
    {
    case PIN_MODE_INPUT:
        bk_gpio_config_input(pin);
        break;

    case PIN_MODE_INPUT_PULLUP:
        bk_gpio_config_input_pup(pin);
        break;

    case PIN_MODE_INPUT_PULLDOWN:
        bk_gpio_config_input_pdwn(pin);
        break;

    case PIN_MODE_OUTPUT:
        bk_gpio_config_output(pin);
        break;
    }
}

static void _gpio_write(struct os_device *device, os_base_t pin, os_base_t value)
{
    OS_ASSERT(device != OS_NULL);

    bk_gpio_output(pin, value);
}

static int _gpio_read(struct os_device *device, os_base_t pin)
{
    OS_ASSERT(device != OS_NULL);

    return bk_gpio_input(pin);
}

static os_err_t _gpio_attach_irq(struct os_device *device, os_int32_t pin, 
                                 os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    LOG_D(DBG_SECTION_NAME, "attach irq pin:%d mode:%d\n", pin, mode);

    if (pin >= GPIONUM)
        return OS_ERROR;

    gpio_dev.irq_desc[pin].handler = hdr;
    gpio_dev.irq_desc[pin].param   = args;
    if (mode == PIN_IRQ_MODE_RISING)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_RISING;
    }
    else if (mode == PIN_IRQ_MODE_FALLING)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_FALLING;
    }
	else if (mode == PIN_IRQ_MODE_HIGH_LEVEL)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_HIGH;
    }
	else if (mode == PIN_IRQ_MODE_LOW_LEVEL)
    {
        gpio_dev.irq_desc[pin].mode = GPIO_INT_LEVEL_LOW;
    }
	
    return OS_EOK;
}

static os_err_t _gpio_dettach_irq(struct os_device *device, os_int32_t pin)
{
    LOG_D(DBG_SECTION_NAME, "dettach irq pin:%d\n", pin);

    if (pin >= GPIONUM)
        return OS_ERROR;

    gpio_int_disable(pin);
    os_memset(&gpio_dev.irq_desc[pin], 0, sizeof(struct gpio_irq_desc));

    return OS_EOK;
}

static os_err_t _gpio_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    LOG_D(DBG_SECTION_NAME, "enable irq pin:%d enabled:%d\n", pin, enabled);

    if (pin >= GPIONUM)
        return OS_ERROR;

    if (enabled)
        gpio_int_enable(pin, gpio_dev.irq_desc[pin].mode, gpio_irq_dispatch);
    else
        gpio_int_disable(pin);

    return OS_EOK;
}

const static struct os_pin_ops _pin_ops =
{
    _gpio_mode,
    _gpio_write,
    _gpio_read,
    _gpio_attach_irq,
    _gpio_dettach_irq,
    _gpio_irq_enable,
};

int bk_hw_gpio_init(void)
{
    os_memset(&gpio_dev, 0, sizeof(gpio_dev));
    os_device_pin_register(0, &_pin_ops, OS_NULL);
    gpio_dev.parent = os_device_find("pin_0");

    return OS_EOK;
}

int bk_hw_gpio_exit(void)
{
    /* unregister device */
    os_device_unregister(gpio_dev.parent);
}

OS_BOARD_INIT(bk_hw_gpio_init);

#endif

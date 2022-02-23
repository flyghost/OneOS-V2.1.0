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
 * @brief       This file implements gpio driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_memory.h>
#include "am_mcu_apollo.h"

#ifdef OS_USING_PIN

struct os_pin_irq_hdr am_pin_irq_hdr_tab[APOLLO_PIN_NUMBERS];

void am_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        am_hal_gpio_pin_config(pin, AM_HAL_GPIO_OUTPUT);
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        am_hal_gpio_pin_config(pin, AM_HAL_GPIO_INPUT);
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        am_hal_gpio_pin_config(pin, AM_HAL_GPIO_INPUT | AM_HAL_GPIO_PULLUP);
    }
    else if ((mode == PIN_MODE_INPUT_PULLDOWN) || (mode == PIN_MODE_OUTPUT_OD))
    {
        /* input setting: pull down. */
        am_hal_gpio_pin_config(pin, AM_HAL_GPIO_OPENDRAIN);
    }
    else
    {
        /* input setting:default. */
        am_hal_gpio_pin_config(pin, AM_HAL_GPIO_3STATE);
    }
}

void am_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    if (value == PIN_LOW)
    {
        am_hal_gpio_out_bit_clear(pin);
    }
    else if (value == PIN_HIGH)
    {
        am_hal_gpio_out_bit_set(pin);
    }
}

int am_pin_read(struct os_device *dev, os_base_t pin)
{
    int value = PIN_LOW;

    if (am_hal_gpio_pin_config_read(pin) == AM_HAL_GPIO_OUTPUT)
    {
        if (am_hal_gpio_out_bit_read(pin) == 0)
        {
            value = PIN_LOW;
        }
        else
        {
            value = PIN_HIGH;
        }
    }
    else
    {
        if (am_hal_gpio_input_bit_read(pin) == 0)
        {
            value = PIN_LOW;
        }
        else
        {
            value = PIN_HIGH;
        }
    }

    return value;
}

os_err_t am_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    os_base_t  level;
    os_int32_t irqindex = -1;

    irqindex = pin;

    level = os_irq_lock();
    if(am_pin_irq_hdr_tab[irqindex].pin == pin   &&
       am_pin_irq_hdr_tab[irqindex].hdr == hdr   &&
       am_pin_irq_hdr_tab[irqindex].mode == mode &&
       am_pin_irq_hdr_tab[irqindex].args == args)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
    if (am_pin_irq_hdr_tab[irqindex].pin != -1)
    {
        os_irq_unlock(level);
        return OS_EBUSY;
    }
    am_pin_irq_hdr_tab[irqindex].pin  = pin;
    am_pin_irq_hdr_tab[irqindex].hdr  = hdr;
    am_pin_irq_hdr_tab[irqindex].mode = mode;
    am_pin_irq_hdr_tab[irqindex].args = args;

    os_irq_unlock(level);

    return OS_EOK;
}

os_err_t am_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    os_base_t  level;
    os_int32_t irqindex = -1;

    irqindex = pin;

    level = os_irq_lock();
    if (am_pin_irq_hdr_tab[irqindex].pin == -1)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
    am_pin_irq_hdr_tab[irqindex].pin  = -1;
    am_pin_irq_hdr_tab[irqindex].hdr  = OS_NULL;
    am_pin_irq_hdr_tab[irqindex].mode = 0;
    am_pin_irq_hdr_tab[irqindex].args = OS_NULL;
    os_irq_unlock(level);

    return OS_EOK;
}

os_err_t am_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    os_base_t  level;
    os_int32_t irqindex = -1;

    irqindex = pin;

    if (enabled == PIN_IRQ_ENABLE)
    {
        level = os_irq_lock();

        /* Configure the GPIO/button interrupt polarity */
        if (am_pin_irq_hdr_tab[irqindex].mode == PIN_IRQ_MODE_RISING)
        {
            am_hal_gpio_int_polarity_bit_set(am_pin_irq_hdr_tab[irqindex].pin, AM_HAL_GPIO_RISING);
        }
        else if (am_pin_irq_hdr_tab[irqindex].mode == PIN_IRQ_MODE_FALLING)
        {
            am_hal_gpio_int_polarity_bit_set(am_pin_irq_hdr_tab[irqindex].pin, AM_HAL_GPIO_FALLING);
        }

        /* Clear the GPIO Interrupt (write to clear) */
        am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(am_pin_irq_hdr_tab[irqindex].pin));

        /* Enable the GPIO/button interrupt */
        am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(am_pin_irq_hdr_tab[irqindex].pin));

        os_irq_unlock(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        if (am_hal_gpio_int_enable_get() != AM_HAL_GPIO_BIT(am_pin_irq_hdr_tab[irqindex].pin))
        {
            return OS_ENOSYS;
        }

        /* Disable the GPIO/button interrupt */
        am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(am_pin_irq_hdr_tab[irqindex].pin));
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}

static void __am_hal_gpio_int_service(uint64_t ui64Status)
{
    uint32_t ui32Status;
    uint32_t ui32Clz;

    /* Handle any active interrupts in the lower 32 bits */
    ui32Status = (uint32_t)ui64Status;
    while (ui32Status)
    {
        /* Pick one of any remaining active interrupt bits */
#ifdef __IAR_SYSTEMS_ICC__
        ui32Clz = __CLZ(ui32Status);
#else
        ui32Clz = __builtin_clz(ui32Status);
#endif

        /* Turn off the bit we picked in the working copy */
        ui32Status &= ~(0x80000000 >> ui32Clz);

        if (am_pin_irq_hdr_tab[31 - ui32Clz].hdr)
        {
            /* If we found an interrupt handler routine, call it now. */
            am_pin_irq_hdr_tab[31 - ui32Clz].hdr(am_pin_irq_hdr_tab[31 - ui32Clz].args);
        }
    }

    /* Handle any active interrupts in the upper 32 bits */
    ui32Status = (uint32_t)(ui64Status >> 32);
    while (ui32Status)
    {
        /* Pick one of any remaining active interrupt bits */
#ifdef __IAR_SYSTEMS_ICC__
        ui32Clz = __CLZ(ui32Status);
#else
        ui32Clz = __builtin_clz(ui32Status);
#endif

        /* Turn off the bit we picked in the working copy */
        ui32Status &= ~(0x80000000 >> ui32Clz);

        if (am_pin_irq_hdr_tab[63 - ui32Clz].hdr)
        {
            am_pin_irq_hdr_tab[63 - ui32Clz].hdr(am_pin_irq_hdr_tab[63 - ui32Clz].args);
        }
    }
}

void am_gpio_isr(void)
{
    uint64_t ui64Status;

    /* Read and clear the GPIO interrupt status. */
    ui64Status = am_hal_gpio_int_status_get(false);
    am_hal_gpio_int_clear(ui64Status);
    __am_hal_gpio_int_service(ui64Status);
}

const static struct os_pin_ops am_pin_ops =
{
    am_pin_mode,
    am_pin_write,
    am_pin_read,
    am_pin_attach_irq,
    am_pin_dettach_irq,
    am_pin_irq_enable,
};

int os_hw_pin_init(void)
{
    int i = 0;
    for (i = 0; i < APOLLO_PIN_NUMBERS; i++)
        am_pin_irq_hdr_tab[i].pin = -1;

    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);

    os_device_pin_register(0, &am_pin_ops, OS_NULL);

    os_kprintf("pin_init!\n");

    return 0;
}

/* INIT_BOARD_EXPORT(os_hw_pin_init); */
#endif

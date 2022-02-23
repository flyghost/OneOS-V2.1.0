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
 * @brief       This file implements gpio driver for GD32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_gpio.h>
#include <device.h>
#include <os_memory.h>
#include <os_errno.h>

#include <pin/pin.h>
#include "gd32vf103.h"

#ifdef OS_USING_PIN

static const struct pin_index pins[] = 
{
#if defined(GPIOA)
    __GD32_PIN(0, A, 0),   __GD32_PIN(1, A, 1),   __GD32_PIN(2, A, 2),   __GD32_PIN(3, A, 3),
    __GD32_PIN(4, A, 4),   __GD32_PIN(5, A, 5),   __GD32_PIN(6, A, 6),   __GD32_PIN(7, A, 7),
    __GD32_PIN(8, A, 8),   __GD32_PIN(9, A, 9),   __GD32_PIN(10, A, 10), __GD32_PIN(11, A, 11),
    __GD32_PIN(12, A, 12), __GD32_PIN(13, A, 13), __GD32_PIN(14, A, 14), __GD32_PIN(15, A, 15),
#if defined(GPIOB)
    __GD32_PIN(16, B, 0),  __GD32_PIN(17, B, 1),  __GD32_PIN(18, B, 2),  __GD32_PIN(19, B, 3),
    __GD32_PIN(20, B, 4),  __GD32_PIN(21, B, 5),  __GD32_PIN(22, B, 6),  __GD32_PIN(23, B, 7),
    __GD32_PIN(24, B, 8),  __GD32_PIN(25, B, 9),  __GD32_PIN(26, B, 10), __GD32_PIN(27, B, 11),
    __GD32_PIN(28, B, 12), __GD32_PIN(29, B, 13), __GD32_PIN(30, B, 14), __GD32_PIN(31, B, 15),
#endif /* defined(GPIOB) */
#endif /* defined(GPIOA) */
};

static const struct pin_irq_map pin_irq_map[] =
{
    {GPIO_PIN_0, EXTI0_IRQn},
    {GPIO_PIN_1, EXTI1_IRQn},
    {GPIO_PIN_2, EXTI2_IRQn},
    {GPIO_PIN_3, EXTI3_IRQn},
    {GPIO_PIN_4, EXTI4_IRQn},
    {GPIO_PIN_5, EXTI5_9_IRQn},
    {GPIO_PIN_6, EXTI5_9_IRQn},
    {GPIO_PIN_7, EXTI5_9_IRQn},
    {GPIO_PIN_8, EXTI5_9_IRQn},
    {GPIO_PIN_9, EXTI5_9_IRQn},
    {GPIO_PIN_10, EXTI10_15_IRQn},
    {GPIO_PIN_11, EXTI10_15_IRQn},
    {GPIO_PIN_12, EXTI10_15_IRQn},
    {GPIO_PIN_13, EXTI10_15_IRQn},
    {GPIO_PIN_14, EXTI10_15_IRQn},
    {GPIO_PIN_15, EXTI10_15_IRQn},
};

static struct os_pin_irq_hdr pin_irq_hdr_tab[] =
{
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
    {-1, 0, OS_NULL, OS_NULL},
};

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])
const struct pin_index *get_pin(os_uint8_t pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];
        if (index->index == -1)
            index = OS_NULL;
    }
    else
    {
        index = OS_NULL;
    }

    return index;
};

static void GD32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }
    gpio_bit_write(index->gpio_port, index->pin, (bit_status)value);
}

static int GD32_pin_read(struct os_device *dev, os_base_t pin)
{
    int                     value = PIN_LOW;
    const struct pin_index *index;
    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return value;
    }

    value = gpio_input_bit_get(index->gpio_port, index->pin);

    return value;
}

static void GD32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    const struct pin_index *index;
    uint32_t set_mode = GPIO_MODE_OUT_PP;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }
    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        set_mode = GPIO_MODE_OUT_PP;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        set_mode = GPIO_MODE_IN_FLOATING;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        set_mode = GPIO_MODE_IPU;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        set_mode = GPIO_MODE_IPD;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        set_mode = GPIO_MODE_OUT_OD;
    }
    rcu_periph_clock_enable(index->clk);
    gpio_init(index->gpio_port, set_mode, GPIO_OSPEED_50MHZ, index->pin);
}

OS_INLINE os_int32_t bit2bitno(os_uint32_t bit)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        if ((0x01 << i) == bit)
        {
            return i;
        }
    }
    return -1;
}

OS_INLINE const struct pin_irq_map *get_pin_irq_map(os_uint32_t pinbit)
{
    os_int32_t mapindex = bit2bitno(pinbit);
    if (mapindex < 0 || mapindex >= ITEM_NUM(pin_irq_map))
    {
        return OS_NULL;
    }
    return &pin_irq_map[mapindex];
};

static os_err_t GD32_pin_attach_irq(struct os_device   *device,
        os_int32_t          pin,
        os_uint32_t         mode,
        void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    os_base_t               level;
    os_int32_t              irqindex = -1;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return OS_ENOSYS;
    }

    level = os_irq_lock();

    if (pin_irq_hdr_tab[irqindex].pin == pin       &&
            pin_irq_hdr_tab[irqindex].hdr == hdr   &&
            pin_irq_hdr_tab[irqindex].mode == mode &&
            pin_irq_hdr_tab[irqindex].args == args)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
    if (pin_irq_hdr_tab[irqindex].pin != -1)
    {
        os_irq_unlock(level);
        return OS_EBUSY;
    }
    pin_irq_hdr_tab[irqindex].pin  = pin;
    pin_irq_hdr_tab[irqindex].hdr  = hdr;
    pin_irq_hdr_tab[irqindex].mode = mode;
    pin_irq_hdr_tab[irqindex].args = args;
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t GD32_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    const struct pin_index *index;
    os_base_t               level;
    os_int32_t              irqindex = -1;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    irqindex = bit2bitno(index->pin);
    if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
    {
        return OS_ENOSYS;
    }

    level = os_irq_lock();
    if (pin_irq_hdr_tab[irqindex].pin == -1)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
    pin_irq_hdr_tab[irqindex].pin  = -1;
    pin_irq_hdr_tab[irqindex].hdr  = OS_NULL;
    pin_irq_hdr_tab[irqindex].mode = 0;
    pin_irq_hdr_tab[irqindex].args = OS_NULL;
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t GD32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    const struct pin_index   *index;
    const struct pin_irq_map *irqmap;
    os_base_t                 level;
    os_int32_t                irqindex = -1;
    exti_trig_type_enum       trigger_mode;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        irqindex = bit2bitno(index->pin);
        if (irqindex < 0 || irqindex >= ITEM_NUM(pin_irq_map))
        {
            return OS_ENOSYS;
        }

        level = os_irq_lock();

        if (pin_irq_hdr_tab[irqindex].pin == -1)
        {
            os_irq_unlock(level);
            return OS_ENOSYS;
        }

        irqmap = &pin_irq_map[irqindex];

        /* Configure GPIO_InitStructure */
        switch (pin_irq_hdr_tab[irqindex].mode)
        {
            case PIN_IRQ_MODE_RISING:
                trigger_mode = EXTI_TRIG_RISING;
                break;
            case PIN_IRQ_MODE_FALLING:
                trigger_mode = EXTI_TRIG_FALLING;
                break;
            case PIN_IRQ_MODE_RISING_FALLING:
                trigger_mode = EXTI_TRIG_BOTH;
                break;
            default:
                os_irq_unlock(level);
                return OS_EINVAL;
        }

        /* enable the Wakeup clock */
        rcu_periph_clock_enable(index->clk);
        rcu_periph_clock_enable(RCU_AF);

        /* configure button pin as input */
        // gpio_init(index->gpio_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, index->pin);
        /* enable and set key EXTI interrupt to the lowest priority */
        eclic_irq_enable(irqmap->irqno, 2, 2);
        /* connect key EXTI line to key GPIO pin */
        gpio_exti_source_select(index->port_src, index->pin_src);
        /* configure key EXTI line */
        exti_init((exti_line_enum)(index->pin), EXTI_INTERRUPT, trigger_mode);
        exti_interrupt_flag_clear((exti_line_enum)(index->pin));

        os_irq_unlock(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        irqmap = get_pin_irq_map(index->pin);
        if (irqmap == OS_NULL)
        {
            return OS_ENOSYS;
        }
        level = os_irq_lock();
        eclic_irq_disable(irqmap->irqno);
        os_irq_unlock(level);
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}

const static struct os_pin_ops _GD32_pin_ops =
{
    GD32_pin_mode,
    GD32_pin_write,
    GD32_pin_read,
    GD32_pin_attach_irq,
    GD32_pin_dettach_irq,
    GD32_pin_irq_enable,
};

OS_INLINE void pin_irq_hdr(int irqno)
{
    if (pin_irq_hdr_tab[irqno].hdr)
    {
        pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}

void GD32_GPIO_EXTI_IRQHandler(os_int8_t exti_line)
{
    if (exti_interrupt_flag_get((exti_line_enum)(1 << exti_line)))
    {
        pin_irq_hdr(exti_line);
        exti_interrupt_flag_clear((exti_line_enum)(1 << exti_line));
    }
}
void EXTI0_IRQHandler(void)
{

    GD32_GPIO_EXTI_IRQHandler(0);

}
void EXTI1_IRQHandler(void)
{

    GD32_GPIO_EXTI_IRQHandler(1);

}
void EXTI2_IRQHandler(void)
{

    GD32_GPIO_EXTI_IRQHandler(2);

}
void EXTI3_IRQHandler(void)
{

    GD32_GPIO_EXTI_IRQHandler(3);

}
void EXTI4_IRQHandler(void)
{

    GD32_GPIO_EXTI_IRQHandler(4);

}
void EXTI5_9_IRQHandler(void)
{

    GD32_GPIO_EXTI_IRQHandler(5);
    GD32_GPIO_EXTI_IRQHandler(6);
    GD32_GPIO_EXTI_IRQHandler(7);
    GD32_GPIO_EXTI_IRQHandler(8);
    GD32_GPIO_EXTI_IRQHandler(9);

}
void EXTI10_15_IRQHandler(void)
{

    GD32_GPIO_EXTI_IRQHandler(10);
    GD32_GPIO_EXTI_IRQHandler(11);
    GD32_GPIO_EXTI_IRQHandler(12);
    GD32_GPIO_EXTI_IRQHandler(13);
    GD32_GPIO_EXTI_IRQHandler(14);
    GD32_GPIO_EXTI_IRQHandler(15);

}

/**
 ***********************************************************************************************************************
 * @brief           os_hw_pin_init:enable gpio clk,register pin device.
 *
 * @param[in]       none
 *
 * @return          Return init result.
 * @retval          OS_EOK       init success.
 * @retval          Others       init failed.
 ***********************************************************************************************************************
 */
int os_hw_pin_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    return os_device_pin_register(0, &_GD32_pin_ops, OS_NULL);
}

#endif

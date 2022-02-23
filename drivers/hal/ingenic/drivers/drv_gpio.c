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

#include <arch_interrupt.h>
#include "drv_gpio.h"
#include <interrupt.h>
#include <arch_misc.h>

#ifdef OS_USING_PIN

static struct pin_index pins[] = {
#if defined(GPIO_PORTA)
    __INDEX_PIN(0, A, 0),    __INDEX_PIN(1, A, 1),    __INDEX_PIN(2, A, 2),    __INDEX_PIN(3, A, 3),
    __INDEX_PIN(4, A, 4),    __INDEX_PIN(5, A, 5),    __INDEX_PIN(6, A, 6),    __INDEX_PIN(7, A, 7),
    __INDEX_PIN(8, A, 8),    __INDEX_PIN(9, A, 9),    __INDEX_PIN(10, A, 10),  __INDEX_PIN(11, A, 11),
    __INDEX_PIN(12, A, 12),  __INDEX_PIN(13, A, 13),  __INDEX_PIN(14, A, 14),  __INDEX_PIN(15, A, 15),    
    __INDEX_PIN(16, A, 16),    __INDEX_PIN(17, A, 17),    __INDEX_PIN(18, A, 18),    __INDEX_PIN(19, A, 19),
    __INDEX_PIN(20, A, 20),    __INDEX_PIN(21, A, 21),    __INDEX_PIN(22, A, 22),    __INDEX_PIN(23, A, 23),
    __INDEX_PIN(24, A, 24),    __INDEX_PIN(25, A, 25),    __INDEX_PIN(26, A, 26),  __INDEX_PIN(27, A, 27),
    __INDEX_PIN(28, A, 28),  __INDEX_PIN(29, A, 29),  __INDEX_PIN(30, A, 30),  __INDEX_PIN(31, A, 31),
#if defined(GPIO_PORTB)
    __INDEX_PIN(32, B, 0),    __INDEX_PIN(33, B, 1),    __INDEX_PIN(34, B, 2),    __INDEX_PIN(35, B, 3),
    __INDEX_PIN(36, B, 4),    __INDEX_PIN(37, B, 5),    __INDEX_PIN(38, B, 6),    __INDEX_PIN(39, B, 7),
    __INDEX_PIN(40, B, 8),    __INDEX_PIN(41, B, 9),    __INDEX_PIN(42, B, 10),  __INDEX_PIN(43, B, 11),
    __INDEX_PIN(44, B, 12),  __INDEX_PIN(45, B, 13),  __INDEX_PIN(46, B, 14),  __INDEX_PIN(47, B, 15),    
    __INDEX_PIN(48, B, 16),    __INDEX_PIN(49, B, 17),    __INDEX_PIN(50, B, 18),    __INDEX_PIN(51, B, 19),
    __INDEX_PIN(52, B, 20),    __INDEX_PIN(53, B, 21),    __INDEX_PIN(54, B, 22),    __INDEX_PIN(55, B, 23),
    __INDEX_PIN(56, B, 24),    __INDEX_PIN(57, B, 25),    __INDEX_PIN(58, B, 26),  __INDEX_PIN(59, B, 27),
    __INDEX_PIN(60, B, 28),  __INDEX_PIN(61, B, 29),  __INDEX_PIN(62, B, 30),  __INDEX_PIN(63, B, 31),
#if defined(GPIO_PORTC)
    __INDEX_PIN(64, C, 0),    __INDEX_PIN(65, C, 1),    __INDEX_PIN(66, C, 2),    __INDEX_PIN(67, C, 3),
    __INDEX_PIN(68, C, 4),    __INDEX_PIN(69, C, 5),    __INDEX_PIN(70, C, 6),    __INDEX_PIN(71, C, 7),
    __INDEX_PIN(72, C, 8),    __INDEX_PIN(73, C, 9),    __INDEX_PIN(74, C, 10),  __INDEX_PIN(75, C, 11),
    __INDEX_PIN(76, C, 12),  __INDEX_PIN(77, C, 13),  __INDEX_PIN(78, C, 14),  __INDEX_PIN(79, C, 15),    
    __INDEX_PIN(80, C, 16),    __INDEX_PIN(81, C, 17),    __INDEX_PIN(82, C, 18),    __INDEX_PIN(83, C, 19),
    __INDEX_PIN(84, C, 20),    __INDEX_PIN(85, C, 21),    __INDEX_PIN(86, C, 22),    __INDEX_PIN(87, C, 23),
    __INDEX_PIN(88, C, 24),    __INDEX_PIN(89, C, 25),    __INDEX_PIN(90, C, 26),  __INDEX_PIN(91, C, 27),
    __INDEX_PIN(92, C, 28),  __INDEX_PIN(93, C, 29),  __INDEX_PIN(94, C, 30),  __INDEX_PIN(95, C, 31),
#if defined(GPIO_PORTD)
    __INDEX_PIN(96, D, 0),    __INDEX_PIN(97, D, 1),    __INDEX_PIN(98, D, 2),    __INDEX_PIN(99, D, 3),
    __INDEX_PIN(100, D, 4),    __INDEX_PIN(101, D, 5),    __INDEX_PIN(102, D, 6),    __INDEX_PIN(103, D, 7),
    __INDEX_PIN(104, D, 8),    __INDEX_PIN(105, D, 9),    __INDEX_PIN(106, D, 10),  __INDEX_PIN(107, D, 11),
    __INDEX_PIN(108, D, 12),  __INDEX_PIN(109, D, 13),  __INDEX_PIN(110, D, 14),  __INDEX_PIN(111, D, 15),    
    __INDEX_PIN(112, D, 16),    __INDEX_PIN(113, D, 17),    __INDEX_PIN(114, D, 18),    __INDEX_PIN(115, D, 19),
    __INDEX_PIN(116, D, 20),    __INDEX_PIN(117, D, 21),    __INDEX_PIN(118, D, 22),    __INDEX_PIN(119, D, 23),
    __INDEX_PIN(120, D, 24),    __INDEX_PIN(121, D, 25),    __INDEX_PIN(122, D, 26),  __INDEX_PIN(123, D, 27),
    __INDEX_PIN(124, D, 28),  __INDEX_PIN(125, D, 29),  __INDEX_PIN(126, D, 30),  __INDEX_PIN(127, D, 31),
#endif /* defined(GPIOD) */
#endif /* defined(GPIOC) */
#endif /* defined(GPIOB) */
#endif /* defined(GPIOA) */
};

struct INGENIC_IRQ_STAT ingenic_irq_stat[] = {
#if defined(GPIO_PORTA)
    {IRQ_GPIO0,0},
#if defined(GPIO_PORTB)
    {IRQ_GPIO1,0},
#if defined(GPIO_PORTC)
    {IRQ_GPIO2,0},
#if defined(GPIO_PORTD)
    {IRQ_GPIO3,0},
#endif
#endif
#endif
#endif
};

struct INGENIC_IRQ_STAT *pState[] = {

#if defined(GPIO_PORTA)
    &ingenic_irq_stat[0],
#if defined(GPIO_PORTB)
    &ingenic_irq_stat[1],
#if defined(GPIO_PORTC)
    &ingenic_irq_stat[2],
#if defined(GPIO_PORTD)
    &ingenic_irq_stat[3],
#endif	
#endif	
#endif
#endif

};

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

static struct pin_index *get_pin(os_uint8_t pin)
{
    struct pin_index *index;

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

static void ingenic_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    gpio_port_set_value(index->port,index->pin, (int)value);
}

static int ingenic_pin_read(struct os_device *dev, os_base_t pin)
{
    int                     value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return value;
    }

    value = gpio_port_get_value(index->port, index->pin);

    return value;
}

static void ingenic_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    enum gpio_function func = GPIO_OUTPUT0;
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    switch(mode){
        case PIN_MODE_OUTPUT:
            func = GPIO_OUTPUT0;
            break;
        case PIN_MODE_INPUT:
        case PIN_MODE_INPUT_PULLUP: 
        case PIN_MODE_INPUT_PULLDOWN:
            func = GPIO_INPUT;
            break;      
        default:
            break;
    }    
    gpio_set_func(index->port,func,(1<<index->pin));

}


    static os_err_t
ingenic_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    struct pin_index *index;
    os_base_t               level;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    level = os_irq_lock();

    if( index->hdr == hdr   &&
            index->args == args){

        os_irq_unlock(level);
        return OS_EOK;
    }

    if(index->args != OS_NULL){

        os_irq_unlock(level);
        return OS_EBUSY;
    }

    index->hdr  = hdr;
    index->args = args;
    index->mode = mode;

    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t ingenic_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    struct pin_index *index;
    os_base_t               level;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    level = os_irq_lock();
    if (index->hdr == OS_NULL)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }

    index->hdr  = OS_NULL;
    index->mode = 0;
    index->args = OS_NULL;

    os_irq_unlock(level);

    return OS_EOK;
}

void gpio_ack_irq(enum gpio_port port,  enum gpio_pin pin)
{
    writel(pin, GPIO_PXFLGC(port));
}

static os_err_t ingenic_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    struct pin_index *index;
    os_base_t level;
    enum gpio_function  func;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {

        level = os_irq_lock();

        switch (index->mode)
        {
            case PIN_IRQ_MODE_HIGH_LEVEL:
                func = GPIO_INT_HI;
                break;
            case PIN_IRQ_MODE_LOW_LEVEL:
                func = GPIO_INT_LO;
                break;
            case PIN_IRQ_MODE_RISING:
                func = GPIO_INT_RE;
                break;
            case PIN_IRQ_MODE_FALLING:
                func = GPIO_INT_FE;
                break;
            case PIN_IRQ_MODE_RISING_FALLING:
                if (gpio_port_get_value(index->port,(1<<index->pin)))
                    func = GPIO_INT_FE;
                else
                    func = GPIO_INT_RE;
                break;
            default:
                os_irq_unlock(level);
                return OS_ENOSYS;
        }

        gpio_set_func(index->port,func,(1<<index->pin));
        gpio_ack_irq(index->port,(1<<index->pin));

        if(pState[index->port]->ref == 0)
            os_hw_interrupt_umask(pState[index->port]->irq);

        pState[index->port]->ref++;

        os_irq_unlock(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        level = os_irq_lock();

        pState[index->port]->ref--;
        if(pState[index->port]->ref == 0)
            os_hw_interrupt_mask(pState[index->port]->irq);

        os_irq_unlock(level);
    }
    else
    {
        return OS_ENOSYS;
    }
    return OS_EOK;
}

void gpio_irq_handler(int irq, void *param)
{
    uint32_t    pend,mask;
    uint32_t    pin_id;
    enum gpio_port  port = (IRQ_GPIO0 - irq);
    enum gpio_pin   pin;


    pend = readl(GPIO_PXFLG(port));
    mask = readl(GPIO_PXMSK(port));

    pend = pend & ~mask;


    while(pend)
    {
        pin_id = os_fls(pend) - 1;
        pin    = 0x01 << pin_id;

        if(pins[__GET_PIN(port,pin_id)].hdr != OS_NULL)
        {
            pins[__GET_PIN(port,pin_id)].hdr(pins[__GET_PIN(port,pin_id)].args);
        }

        pend &= ~(0x01 << pin_id);
        gpio_ack_irq(port, pin);
    }
}

const static struct os_pin_ops _ingenic_pin_ops = {
    ingenic_pin_mode,
    ingenic_pin_write,
    ingenic_pin_read,
    ingenic_pin_attach_irq,
    ingenic_pin_dettach_irq,
    ingenic_pin_irq_enable,
};

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
static os_err_t os_hw_pin_init(void)
{
    setup_gpio_pins();
#if defined(GPIO_PORTA)
    os_hw_interrupt_install(IRQ_GPIO0,gpio_irq_handler,&pins[0],"GPIOAINT");
    os_hw_interrupt_mask(IRQ_GPIO0);
#if defined(GPIO_PORTB)
    os_hw_interrupt_install(IRQ_GPIO1,gpio_irq_handler,&pins[0],"GPIOBINT");
    os_hw_interrupt_mask(IRQ_GPIO1);
#if defined(GPIO_PORTC)
    os_hw_interrupt_install(IRQ_GPIO2,gpio_irq_handler,&pins[0],"GPIOCINT");
    os_hw_interrupt_mask(IRQ_GPIO2);
#if defined(GPIO_PORTD)
    os_hw_interrupt_install(IRQ_GPIO3,gpio_irq_handler,&pins[0],"GPIODINT");
    os_hw_interrupt_mask(IRQ_GPIO3);
#endif
#endif
#endif
#endif

    return os_device_pin_register(0, &_ingenic_pin_ops, OS_NULL);
}

OS_PREV_INIT(os_hw_pin_init, OS_INIT_SUBLEVEL_HIGH);

#endif /* OS_USING_PIN */

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
 * @brief       This file implements gpio driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include "drv_gpio.h"

#ifdef OS_USING_PIN

#define DBG_TAG "drv_gpio"

os_uint32_t gpio_port_map = 0;
static struct pin_index *indexs[GPIO_PORT_MAX];
en_gpio_port_t gpio_port_base[GPIO_PORT_MAX];
static os_list_node_t hc_gpio_irq_list = OS_LIST_INIT(hc_gpio_irq_list);

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

#define GPIO_INFO_MAP(gpio)                                                                                               \
{                                                                                                                         \
    gpio_port_map |= (1 << GPIO_INDEX_##gpio);                                                                            \
    gpio_port_base[GPIO_INDEX_##gpio] = GpioPort##gpio;                                                                   \
}

void __os_hw_pin_init(void)
{
#if defined(GPIOA)
    GPIO_INFO_MAP(A);
#endif
#if defined(GPIOB)
    GPIO_INFO_MAP(B);
#endif
#if defined(GPIOC)
    GPIO_INFO_MAP(C);
#endif
#if defined(GPIOD)
    GPIO_INFO_MAP(D);
#endif
#if defined(GPIOE)
    GPIO_INFO_MAP(E);
#endif
#if defined(GPIOF)
    GPIO_INFO_MAP(F);
#endif
#if defined(GPIOG)
    GPIO_INFO_MAP(G);
#endif
#if defined(GPIOH)
    GPIO_INFO_MAP(H);
#endif
#if defined(GPIOI)
    GPIO_INFO_MAP(I);
#endif
#if defined(GPIOJ)
    GPIO_INFO_MAP(J);
#endif
#if defined(GPIOK)
    GPIO_INFO_MAP(K);
#endif
}

struct HC32_IRQ_STAT hc32_irq_stat[] = {
    {PORTA_IRQn, 0},
    {PORTB_IRQn, 0},
    {PORTC_E_IRQn, 0},
    {PORTD_F_IRQn, 0},
};

struct HC32_IRQ_STAT *pState[] = {
#if defined(GPIOA)
    &hc32_irq_stat[0],
#if defined(GPIOB)
    &hc32_irq_stat[1],
#if defined(GPIOC)
    &hc32_irq_stat[2],
#if defined(GPIOD)
    &hc32_irq_stat[3],
#if defined(GPIOE)
    &hc32_irq_stat[2],
#if defined(GPIOF)
    &hc32_irq_stat[3],
#endif
#endif
#endif
#endif
#endif
#endif
};

static struct pin_index *get_pin_index(os_base_t pin)
{
    struct pin_index *index = OS_NULL;

    if(pin < GPIO_PIN_MAX)
    {
        index = indexs[__PORT_INDEX(pin)];
    }

    return index;
}

en_gpio_port_t get_pin_base(os_base_t pin)
{
    struct pin_index *index = get_pin_index(pin);

    OS_ASSERT(index != OS_NULL);

    return index->gpio_port;
}

static void hc32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    const struct pin_index *index;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }

    Gpio_WriteOutputIO(index->gpio_port, PIN_OFFSET(pin), (boolean_t)value);
}

static int hc32_pin_read(struct os_device *dev, os_base_t pin)
{
    int                     value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return value;
    }

    value = Gpio_GetInputIO(index->gpio_port, PIN_OFFSET(pin));

    return value;
}

static void hc32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    struct pin_index *index;
    stc_gpio_cfg_t    GPIO_Cfg;

#if (PIN_PULL_STATE)
    os_base_t         level;
#endif

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }

    /* Configure GPIO_InitStructure */
    GPIO_Cfg.enCtrlMode = GpioAHB;
    GPIO_Cfg.enDrv = GpioDrvH;

    switch(mode) {
    case PIN_MODE_OUTPUT:
        /* output setting */
        GPIO_Cfg.enDir = GpioDirOut;
        GPIO_Cfg.enPd = GpioPdDisable;
        GPIO_Cfg.enPu = GpioPuDisable;
        break;
    case PIN_MODE_INPUT:
        /* input setting: not pull. */
        GPIO_Cfg.enDir = GpioDirIn;
        GPIO_Cfg.enPd = GpioPdDisable;
        GPIO_Cfg.enPu = GpioPuDisable;
        break;

    case PIN_MODE_INPUT_PULLUP:
        /* input setting: pull up. */
        GPIO_Cfg.enDir = GpioDirIn;
        GPIO_Cfg.enPd = GpioPdDisable;
        GPIO_Cfg.enPu = GpioPuEnable;

        break;
    case PIN_MODE_INPUT_PULLDOWN:

        /* input setting: pull down. */
        GPIO_Cfg.enDir = GpioDirIn;
        GPIO_Cfg.enPd = GpioPdEnable;
        GPIO_Cfg.enPu = GpioPuDisable;
        break;
    case PIN_MODE_OUTPUT_OD:
        /* output setting: od. */
        GPIO_Cfg.enDir = GpioDirOut;
        GPIO_Cfg.enPd = GpioPdDisable;
        GPIO_Cfg.enPu = GpioPuDisable;
        GPIO_Cfg.enOD = GpioOdEnable;

        break;

    default:

        break;
    }
#if (PIN_PULL_STATE)
    /* remeber the pull state. */
    level = os_irq_lock();
    index->pin_pulls[__PIN_INDEX(pin)].pull_up   = GPIO_Cfg.enPu;
    index->pin_pulls[__PIN_INDEX(pin)].pull_down = GPIO_Cfg.enPd;
    os_irq_unlock(level);
#endif
    Gpio_Init(index->gpio_port, PIN_OFFSET(pin), &GPIO_Cfg);
}

static os_err_t
hc32_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    os_base_t               level;
    struct pin_irq_param   *irq;
    struct pin_irq_param   *tmp;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    irq = os_calloc(1, sizeof(struct pin_irq_param));
    OS_ASSERT(irq != OS_NULL);

    level = os_irq_lock();
    os_list_for_each_entry(tmp, &hc_gpio_irq_list, struct pin_irq_param, list)
    {
        if(tmp->pin == pin)
        {
            if(tmp->hdr  == hdr   &&
                    tmp->mode == mode  &&
                    tmp->args == args)
            {
                os_irq_unlock(level);
                os_free(irq);
                return OS_EOK;
            }
            else
            {
                os_irq_unlock(level);
                os_free(irq);
                return OS_EBUSY;
            }
        }
    }

    irq->pin  = pin;
    irq->hdr  = hdr;
    irq->mode = mode;
    irq->args = args;

    os_list_add_tail(&hc_gpio_irq_list, &irq->list);
    os_irq_unlock(level);
    return OS_EOK;
}

static os_err_t hc32_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    const struct pin_index *index;
    os_base_t               level;
    struct pin_irq_param   *irq;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    level = os_irq_lock();
    os_list_for_each_entry(irq, &hc_gpio_irq_list, struct pin_irq_param, list)
    {
        if(irq->pin == pin)
        {
            os_list_del(&irq->list);
            os_irq_unlock(level);
            return OS_EOK;
        }
    }
    os_irq_unlock(level);
    return OS_EEMPTY;
}

static os_err_t hc32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    const struct pin_index   *index;
    os_base_t                 level;
    struct pin_irq_param     *irq;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        level = os_irq_lock();

        os_list_for_each_entry(irq, &hc_gpio_irq_list, struct pin_irq_param, list)
        {
            if(irq->pin == pin)
            {
                if(irq->irq_ref > 0)
                {
                    os_irq_unlock(level);
                    return OS_EOK;
                }

                switch (irq->mode)
                {
                case PIN_IRQ_MODE_RISING:
                    Gpio_EnableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqRising);
                    break;
                case PIN_IRQ_MODE_FALLING:
                    Gpio_EnableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqFalling);
                    break;
                case PIN_IRQ_MODE_HIGH_LEVEL:
                    Gpio_EnableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqHigh);
                    break;
                case PIN_IRQ_MODE_LOW_LEVEL:
                    Gpio_EnableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqLow);
                    break;
                case PIN_IRQ_MODE_RISING_FALLING:
                default:
                    os_irq_unlock(level);
                    return OS_ENOSYS;
                }
                irq->irq_ref++;
                if(pState[__PORT_INDEX(pin)]->ref == 0)
                {
                    EnableNvic(pState[__PORT_INDEX(pin)]->irq, IrqLevel3, TRUE);
                }
                pState[__PORT_INDEX(pin)]->ref++;
                os_irq_unlock(level);
            }
        }
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        level = os_irq_lock();
        os_list_for_each_entry(irq, &hc_gpio_irq_list, struct pin_irq_param, list)
        {
            if(irq->pin == pin)
            {
                if(irq->irq_ref == 0)
                {
                    os_irq_unlock(level);
                    return OS_EOK;
                }

                switch (irq->mode)
                {
                case PIN_IRQ_MODE_RISING:
                    Gpio_DisableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqRising);
                    break;
                case PIN_IRQ_MODE_FALLING:
                    Gpio_DisableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqFalling);
                    break;
                case PIN_IRQ_MODE_HIGH_LEVEL:
                    Gpio_DisableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqHigh);
                    break;
                case PIN_IRQ_MODE_LOW_LEVEL:
                    Gpio_DisableIrq(index->gpio_port, PIN_OFFSET(pin), GpioIrqLow);
                    break;
                case PIN_IRQ_MODE_RISING_FALLING:
                default:
                    os_irq_unlock(level);
                    return OS_ENOSYS;
                }

                irq->irq_ref--;
                pState[__PORT_INDEX(pin)]->ref--;
                if(pState[__PORT_INDEX(pin)]->ref == 0)
                {
                    EnableNvic(pState[__PORT_INDEX(pin)]->irq, IrqLevel3, FALSE);
                }

                os_irq_unlock(level);
            }
        }

    }
    else
    {
        return OS_ENOSYS;
    }
    return OS_EOK;
}
const static struct os_pin_ops _hc32_pin_ops = {
    .pin_mode       = hc32_pin_mode,
    .pin_write      = hc32_pin_write,
    .pin_read       = hc32_pin_read,
    .pin_attach_irq = hc32_pin_attach_irq,
    .pin_detach_irq = hc32_pin_dettach_irq,
    .pin_irq_enable = hc32_pin_irq_enable,
};

OS_INLINE void pin_irq_hdr(os_uint32_t gpio_port_index, en_gpio_port_t gpio_port_base)
{
    const struct pin_index *index;
    en_gpio_pin_t           pin;
    struct pin_irq_param   *irq;

    for(pin = GpioPin0; pin <= GpioPin15; pin++)
    {
        index = get_pin_index(pin + gpio_port_index * 16);

        if (index == OS_NULL)
            continue;

        if(TRUE == Gpio_GetIrqStatus(gpio_port_base, pin))
        {
            os_list_for_each_entry(irq, &hc_gpio_irq_list, struct pin_irq_param, list)
            {
                if(irq->pin == pin + gpio_port_index * 16)
                {
                    irq->hdr(irq->args);
                    Gpio_ClearIrq(gpio_port_base, pin);
                }
            }
        }
    }

}

#if defined(GPIOA)
void PortA_IRQHandler(void)
{
    pin_irq_hdr(GPIOA, GpioPortA);
}
#if defined(GPIOB)
void PortB_IRQHandler(void)
{
    pin_irq_hdr(GPIOB, GpioPortB);
}

#if defined(GPIOC)
void PortC_IRQHandler(void)
{
    pin_irq_hdr(GPIOC, GpioPortC);
}
#if defined(GPIOD)
void PortD_IRQHandler(void)
{
    pin_irq_hdr(GPIOD, GpioPortD);
}
#if defined(GPIOE)
void PortE_IRQHandler(void)
{
    pin_irq_hdr(GPIOE, GpioPortE);
}
#if defined(GPIOF)
void PortF_IRQHandler(void)
{
    pin_irq_hdr(GPIOF, GpioPortF);
}
#endif
#endif
#endif
#endif
#endif

#endif

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
    struct pin_index *tmp = OS_NULL;
    os_uint8_t gpio_port = 0;

#if (PIN_PULL_STATE)
    os_uint8_t gpio_pin = 0;
#endif

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    __os_hw_pin_init();

    for(gpio_port = 0; gpio_port < GPIO_PORT_MAX; gpio_port++)
    {
        if((gpio_port_map & (1 << gpio_port)))
        {
            tmp = (struct pin_index *)os_calloc(1, sizeof(struct pin_index));

            OS_ASSERT(tmp != OS_NULL);

            tmp->gpio_port = gpio_port_base[gpio_port];
#if (PIN_PULL_STATE)
            for(gpio_pin = 0; gpio_pin < GPIO_PIN_PER_PORT; gpio_pin++)
            {
                tmp->pin_pulls[gpio_pin].pull_up   = 0;
                tmp->pin_pulls[gpio_pin].pull_down = 0;
            }
#endif
            indexs[gpio_port] = tmp;

        }
        else
        {
            indexs[gpio_port] = OS_NULL;
        }
    }
    return os_device_pin_register(0, &_hc32_pin_ops, OS_NULL);
}

#endif /* OS_USING_PIN */

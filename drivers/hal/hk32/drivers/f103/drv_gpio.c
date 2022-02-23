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
 * @brief       This file implements gpio driver for hk32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-19   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <arch_misc.h>
#include <pin/pin.h>
#include <driver.h>
#include <os_memory.h>
#include <drv_common.h>
#include "board.h"

#define DBG_TAG "drv_gpio"

#define HK32_PORT(pin)      (pin / 16)
#define HK32_PIN(pin)       (pin % 16)

struct hk32_pin_irq
{
    os_uint16_t pinbit;
    IRQn_Type   irqno;
};

struct hk32_gpio
{
    GPIO_TypeDef *base;
    uint32_t rcc;
};

static const struct hk32_gpio hk32_gpio_table[] =
{
#ifdef GPIOA
    {GPIOA, RCC_APB2Periph_GPIOA},
#ifdef GPIOB
    {GPIOB, RCC_APB2Periph_GPIOB},
#ifdef GPIOC
    {GPIOC, RCC_APB2Periph_GPIOC},
#ifdef GPIOD
    {GPIOD, RCC_APB2Periph_GPIOD},
#ifdef GPIOE
    {GPIOE, RCC_APB2Periph_GPIOE},
#ifdef GPIOF
    {GPIOF, RCC_APB2Periph_GPIOF},
#ifdef GPIOG
    {GPIOG, RCC_APB2Periph_GPIOG},
#endif  /* GPIOG */
#endif  /* GPIOF */
#endif  /* GPIOE */
#endif  /* GPIOD */
#endif  /* GPIOC */
#endif  /* GPIOB */
#endif  /* GPIOA */
};

static const struct hk32_pin_irq hk32_pin_irq[] = {
    {GPIO_Pin_1,  EXTI0_IRQn},
    {GPIO_Pin_1,  EXTI1_IRQn},
    {GPIO_Pin_2,  EXTI2_IRQn},
    {GPIO_Pin_3,  EXTI3_IRQn},
    {GPIO_Pin_4,  EXTI4_IRQn},
    {GPIO_Pin_5,  EXTI9_5_IRQn},
    {GPIO_Pin_6,  EXTI9_5_IRQn},
    {GPIO_Pin_7,  EXTI9_5_IRQn},
    {GPIO_Pin_8,  EXTI9_5_IRQn},
    {GPIO_Pin_9,  EXTI9_5_IRQn},
    {GPIO_Pin_10, EXTI15_10_IRQn},
    {GPIO_Pin_11, EXTI15_10_IRQn},
    {GPIO_Pin_12, EXTI15_10_IRQn},
    {GPIO_Pin_13, EXTI15_10_IRQn},
    {GPIO_Pin_14, EXTI15_10_IRQn},
    {GPIO_Pin_15, EXTI15_10_IRQn},
};

static struct os_pin_irq_hdr hk32_pin_irq_hdr_tab[] = {
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

static void hk32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    int _port = HK32_PORT(pin);
    int _pin  = HK32_PIN(pin);

    OS_ASSERT(_port < ARRAY_SIZE(hk32_gpio_table));

    const struct hk32_gpio *gpio = &hk32_gpio_table[_port];

    GPIO_WriteBit(gpio->base, 1 << _pin, value ? Bit_SET : Bit_RESET);
}

static int hk32_pin_read(struct os_device *dev, os_base_t pin)
{
    int _port = HK32_PORT(pin);
    int _pin  = HK32_PIN(pin);

    OS_ASSERT(_port < ARRAY_SIZE(hk32_gpio_table));

    const struct hk32_gpio *gpio = &hk32_gpio_table[_port];

    return GPIO_ReadInputDataBit(gpio->base, 1 << _pin);
}

static void hk32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    int _port = HK32_PORT(pin);
    int _pin  = HK32_PIN(pin);

    OS_ASSERT(_port < ARRAY_SIZE(hk32_gpio_table));

    const struct hk32_gpio *gpio = &hk32_gpio_table[_port];
    
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin   = 1 << _pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;

    if (mode == PIN_MODE_OUTPUT)
    {
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
    }

    RCC_APB2PeriphClockCmd(gpio->rcc, ENABLE);
    GPIO_Init(gpio->base, &GPIO_InitStructure);
}

static os_err_t
hk32_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    os_base_t level;

    int _pin  = HK32_PIN(pin);

    OS_ASSERT(_pin < ARRAY_SIZE(hk32_pin_irq));

    level = os_irq_lock();
    
    if (hk32_pin_irq_hdr_tab[_pin].pin == pin
     && hk32_pin_irq_hdr_tab[_pin].hdr == hdr
     && hk32_pin_irq_hdr_tab[_pin].mode == mode
     && hk32_pin_irq_hdr_tab[_pin].args == args)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
     
    if (hk32_pin_irq_hdr_tab[_pin].pin != -1)
    {
        os_irq_unlock(level);
        return OS_EBUSY;
    }
    
    hk32_pin_irq_hdr_tab[_pin].pin  = pin;
    hk32_pin_irq_hdr_tab[_pin].hdr  = hdr;
    hk32_pin_irq_hdr_tab[_pin].mode = mode;
    hk32_pin_irq_hdr_tab[_pin].args = args;
    
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t hk32_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    os_base_t level;

    int _pin  = HK32_PIN(pin);

    OS_ASSERT(_pin < ARRAY_SIZE(hk32_pin_irq));

    level = os_irq_lock();
    
    if (hk32_pin_irq_hdr_tab[_pin].pin == -1)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
    
    hk32_pin_irq_hdr_tab[_pin].pin  = -1;
    hk32_pin_irq_hdr_tab[_pin].hdr  = OS_NULL;
    hk32_pin_irq_hdr_tab[_pin].mode = 0;
    hk32_pin_irq_hdr_tab[_pin].args = OS_NULL;
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t hk32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    os_base_t level;
    const struct hk32_pin_irq *irqmap;    

    int _port = HK32_PORT(pin);
    int _pin  = HK32_PIN(pin);

    OS_ASSERT(_port < ARRAY_SIZE(hk32_gpio_table));
    OS_ASSERT(_pin < ARRAY_SIZE(hk32_pin_irq));

    if (enabled == PIN_IRQ_ENABLE)
    {
        level = os_irq_lock();

        if (hk32_pin_irq_hdr_tab[_pin].pin == -1)
        {
            os_irq_unlock(level);
            return OS_ENOSYS;
        }

        irqmap = &hk32_pin_irq[_pin];

        EXTI_InitTypeDef EXTI_InitStructure;
        EXTI_InitStructure.EXTI_Line = irqmap->pinbit;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;

        switch (hk32_pin_irq_hdr_tab[_pin].mode)
        {
        case PIN_IRQ_MODE_RISING:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
            break;
        case PIN_IRQ_MODE_FALLING:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
            break;
        case PIN_IRQ_MODE_RISING_FALLING:
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
            break;
        case PIN_IRQ_MODE_HIGH_LEVEL:
        case PIN_IRQ_MODE_LOW_LEVEL:
        default:
            os_irq_unlock(level);
            return OS_ERROR;
        }

        GPIO_EXTILineConfig(GPIO_PortSourceGPIOA + _port, GPIO_PinSource0 + _pin);

        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = irqmap->irqno;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        os_irq_unlock(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        level = os_irq_lock();

        EXTI_InitTypeDef EXTI_InitStructure;
        EXTI_InitStructure.EXTI_Line = irqmap->pinbit;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
        EXTI_Init(&EXTI_InitStructure);

        os_irq_unlock(level);
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}

const static struct os_pin_ops _hk32_pin_ops = {
    .pin_mode       = hk32_pin_mode,
    .pin_write      = hk32_pin_write,
    .pin_read       = hk32_pin_read,
    .pin_attach_irq = hk32_pin_attach_irq,
    .pin_detach_irq = hk32_pin_dettach_irq,
    .pin_irq_enable = hk32_pin_irq_enable,
};

int os_hw_pin_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    return os_device_pin_register(0, &_hk32_pin_ops, OS_NULL);
}

static void hk32_pin_irq_hdr(int irqno)
{
    if (hk32_pin_irq_hdr_tab[irqno].hdr)
    {
        hk32_pin_irq_hdr_tab[irqno].hdr(hk32_pin_irq_hdr_tab[irqno].args);
    }
}

static void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
    if (EXTI_GetITStatus(GPIO_Pin) != RESET)
    {
        EXTI_ClearITPendingBit(GPIO_Pin);
        hk32_pin_irq_hdr(os_ffs(GPIO_Pin) - 1);
    }
}

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_0);
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_1);
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_2);
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_3);
}

void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_4);
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_9);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_Pin_15);
}


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

#include "drv_gpio.h"
#include <pin/pin.h>
#include <driver.h>
#include <os_memory.h>

#ifdef OS_USING_PIN

#define DBG_TAG "drv_gpio"

os_uint32_t gpio_port_map = 0;
static os_uint32_t pin_irq_enable_mask = 0;
static struct pin_index *indexs[GPIO_PORT_MAX];
GPIO_TypeDef *gpio_port_base[GPIO_PORT_MAX];

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

#define GPIO_INFO_MAP(gpio)                                                                                               \
{                                                                                                                         \
    gpio_port_map |= (1 << GPIO_INDEX_##gpio);                                                                            \
    gpio_port_base[GPIO_INDEX_##gpio] = GPIO##gpio;                                                                       \
}

static os_err_t stm32_pin_dettach_irq(struct os_device *device, os_int32_t pin);

void __os_hw_pin_init(void)
{
#if defined(GPIOA)
    GPIO_INFO_MAP(A);
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
    __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
#endif
#if defined(GPIOB)
    GPIO_INFO_MAP(B);
#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
    __HAL_RCC_GPIOB_CLK_ENABLE();
#endif
#endif
#if defined(GPIOC)
    GPIO_INFO_MAP(C);
#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
    __HAL_RCC_GPIOC_CLK_ENABLE();
#endif
#endif
#if defined(GPIOD)
    GPIO_INFO_MAP(D);
#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    __HAL_RCC_GPIOD_CLK_ENABLE();
#endif
#endif
#if defined(GPIOE)
    GPIO_INFO_MAP(E);

#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
#endif
#endif
#if defined(GPIOF)
    GPIO_INFO_MAP(F);
#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
    __HAL_RCC_GPIOF_CLK_ENABLE();
#endif
#endif
#if defined(GPIOG)
    GPIO_INFO_MAP(G);
#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
#ifdef SERIES_STM32L4
    HAL_PWREx_EnableVddIO2();
#endif
    __HAL_RCC_GPIOG_CLK_ENABLE();
#endif
#endif
#if defined(GPIOH)
    GPIO_INFO_MAP(H);
#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
    __HAL_RCC_GPIOH_CLK_ENABLE();
#endif
#endif
#if defined(GPIOI)
    GPIO_INFO_MAP(I);
#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
    __HAL_RCC_GPIOI_CLK_ENABLE();
#endif
#endif
#if defined(GPIOJ)
    GPIO_INFO_MAP(J);
#if defined(__HAL_RCC_GPIOJ_CLK_ENABLE)
    __HAL_RCC_GPIOJ_CLK_ENABLE();
#endif
#endif
#if defined(GPIOK)
    GPIO_INFO_MAP(K);
#if defined(__HAL_RCC_GPIOK_CLK_ENABLE)
    __HAL_RCC_GPIOK_CLK_ENABLE();
#endif
#endif
#if defined(GPIOZ)
    GPIO_INFO_MAP(Z);
#if defined(__HAL_RCC_GPIOZ_CLK_ENABLE)
    __HAL_RCC_GPIOZ_CLK_ENABLE();
#endif
#endif
}

static const struct pin_irq_map pin_irq_map[] = {
#if defined(SERIES_STM32F0) || defined(SERIES_STM32L0) || defined(SERIES_STM32G0)
    {GPIO_PIN_0, EXTI0_1_IRQn},
    {GPIO_PIN_1, EXTI0_1_IRQn},
    {GPIO_PIN_2, EXTI2_3_IRQn},
    {GPIO_PIN_3, EXTI2_3_IRQn},
    {GPIO_PIN_4, EXTI4_15_IRQn},
    {GPIO_PIN_5, EXTI4_15_IRQn},
    {GPIO_PIN_6, EXTI4_15_IRQn},
    {GPIO_PIN_7, EXTI4_15_IRQn},
    {GPIO_PIN_8, EXTI4_15_IRQn},
    {GPIO_PIN_9, EXTI4_15_IRQn},
    {GPIO_PIN_10, EXTI4_15_IRQn},
    {GPIO_PIN_11, EXTI4_15_IRQn},
    {GPIO_PIN_12, EXTI4_15_IRQn},
    {GPIO_PIN_13, EXTI4_15_IRQn},
    {GPIO_PIN_14, EXTI4_15_IRQn},
    {GPIO_PIN_15, EXTI4_15_IRQn},
#elif defined(SERIES_STM32F3)
    {GPIO_PIN_0, EXTI0_IRQn},
    {GPIO_PIN_1, EXTI1_IRQn},
    {GPIO_PIN_2, EXTI2_TSC_IRQn},
    {GPIO_PIN_3, EXTI3_IRQn},
    {GPIO_PIN_4, EXTI4_IRQn},
    {GPIO_PIN_5, EXTI9_5_IRQn},
    {GPIO_PIN_6, EXTI9_5_IRQn},
    {GPIO_PIN_7, EXTI9_5_IRQn},
    {GPIO_PIN_8, EXTI9_5_IRQn},
    {GPIO_PIN_9, EXTI9_5_IRQn},
    {GPIO_PIN_10, EXTI15_10_IRQn},
    {GPIO_PIN_11, EXTI15_10_IRQn},
    {GPIO_PIN_12, EXTI15_10_IRQn},
    {GPIO_PIN_13, EXTI15_10_IRQn},
    {GPIO_PIN_14, EXTI15_10_IRQn},
    {GPIO_PIN_15, EXTI15_10_IRQn},
#elif defined(SERIES_STM32L5)
    {GPIO_PIN_0, EXTI0_IRQn},
    {GPIO_PIN_1, EXTI1_IRQn},
    {GPIO_PIN_2, EXTI2_IRQn},
    {GPIO_PIN_3, EXTI3_IRQn},
    {GPIO_PIN_4, EXTI4_IRQn},
    {GPIO_PIN_5, EXTI5_IRQn},
    {GPIO_PIN_6, EXTI6_IRQn},
    {GPIO_PIN_7, EXTI7_IRQn},
    {GPIO_PIN_8, EXTI8_IRQn},
    {GPIO_PIN_9, EXTI9_IRQn},
    {GPIO_PIN_10, EXTI10_IRQn},
    {GPIO_PIN_11, EXTI11_IRQn},
    {GPIO_PIN_12, EXTI12_IRQn},
    {GPIO_PIN_13, EXTI13_IRQn},
    {GPIO_PIN_14, EXTI14_IRQn},
    {GPIO_PIN_15, EXTI15_IRQn},
#elif defined(SERIES_STM32MP1)
    {GPIO_PIN_0, EXTI0_IRQn},
    {GPIO_PIN_1, EXTI1_IRQn},
    {GPIO_PIN_2, EXTI2_IRQn},
    {GPIO_PIN_3, EXTI3_IRQn},
    {GPIO_PIN_4, EXTI4_IRQn},
    {GPIO_PIN_5, EXTI5_IRQn},
    {GPIO_PIN_6, EXTI6_IRQn},
    {GPIO_PIN_7, EXTI7_IRQn},
    {GPIO_PIN_8, EXTI8_IRQn},
    {GPIO_PIN_9, EXTI9_IRQn},
    {GPIO_PIN_10, EXTI10_IRQn},
    {GPIO_PIN_11, EXTI11_IRQn},
    {GPIO_PIN_12, EXTI12_IRQn},
    {GPIO_PIN_13, EXTI13_IRQn},
    {GPIO_PIN_14, EXTI14_IRQn},
    {GPIO_PIN_15, EXTI15_IRQn},
#else
    {GPIO_PIN_0, EXTI0_IRQn},
    {GPIO_PIN_1, EXTI1_IRQn},
    {GPIO_PIN_2, EXTI2_IRQn},
    {GPIO_PIN_3, EXTI3_IRQn},
    {GPIO_PIN_4, EXTI4_IRQn},
    {GPIO_PIN_5, EXTI9_5_IRQn},
    {GPIO_PIN_6, EXTI9_5_IRQn},
    {GPIO_PIN_7, EXTI9_5_IRQn},
    {GPIO_PIN_8, EXTI9_5_IRQn},
    {GPIO_PIN_9, EXTI9_5_IRQn},
    {GPIO_PIN_10, EXTI15_10_IRQn},
    {GPIO_PIN_11, EXTI15_10_IRQn},
    {GPIO_PIN_12, EXTI15_10_IRQn},
    {GPIO_PIN_13, EXTI15_10_IRQn},
    {GPIO_PIN_14, EXTI15_10_IRQn},
    {GPIO_PIN_15, EXTI15_10_IRQn},
#endif
};

static struct os_pin_irq_hdr pin_irq_hdr_tab[] = {
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

static struct pin_index *get_pin_index(os_base_t pin)
{
    struct pin_index *index = OS_NULL;

    if(pin<GPIO_PIN_MAX)
    {
        index = indexs[__PORT_INDEX(pin)];
    }
    
    return index;
}

GPIO_TypeDef *get_pin_base(os_base_t pin)
{
    struct pin_index *index = get_pin_index(pin);

    if(index != OS_NULL)
    {
        return index->gpio_port;
    }
    return OS_NULL;
}

static void stm32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    const struct pin_index *index;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }

    HAL_GPIO_WritePin(index->gpio_port, PIN_OFFSET(pin), (GPIO_PinState)value);
}

static int stm32_pin_read(struct os_device *dev, os_base_t pin)
{
    int                     value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return value;
    }

    value = HAL_GPIO_ReadPin(index->gpio_port, PIN_OFFSET(pin));

    return value;
}

static void stm32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    os_base_t level;
    struct pin_index *index;
    GPIO_InitTypeDef        GPIO_InitStruct;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin   = PIN_OFFSET(pin);
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_DISABLE)
    {
        if (OS_EOK != stm32_pin_dettach_irq(dev, pin))
        {
            return;
        }
        
        HAL_GPIO_DeInit(index->gpio_port, GPIO_InitStruct.Pin);
        
        return;
    }

    /* remeber the pull state. */
    level = os_irq_lock();
    index->pin_pulls[__PIN_INDEX(pin)].pull_state = GPIO_InitStruct.Pull;
    os_irq_unlock(level);
    HAL_GPIO_Init(index->gpio_port, &GPIO_InitStruct);
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

static os_err_t
stm32_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    os_base_t               level;
    os_int32_t              irqindex = -1;
    GPIO_InitTypeDef        GPIO_InitStruct;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    irqindex = __PIN_INDEX(pin);
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


    if (pin_irq_hdr_tab[irqindex].pin == -1)
    {
        os_irq_unlock(level);
        return OS_ENOSYS;
    }
    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin   = PIN_OFFSET(pin);
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    switch (pin_irq_hdr_tab[irqindex].mode)
    {
    case PIN_IRQ_MODE_RISING:
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        break;
    case PIN_IRQ_MODE_FALLING:
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
        break;
    case PIN_IRQ_MODE_RISING_FALLING:
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
        break;
    case PIN_IRQ_MODE_HIGH_LEVEL:
    case PIN_IRQ_MODE_LOW_LEVEL:
    default:
        os_irq_unlock(level);
        return OS_ERROR;
    }
    GPIO_InitStruct.Pull = index->pin_pulls[__PIN_INDEX(pin)].pull_state;

    EXTI_HandleTypeDef      hexti;
    hexti.Line = irqindex;
    HAL_EXTI_ClearPending(&hexti,GPIO_InitStruct.Mode);

    HAL_GPIO_Init(index->gpio_port, &GPIO_InitStruct);
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t stm32_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    const struct pin_index *index;
    os_base_t               level;
    os_int32_t              irqindex = -1;
    GPIO_InitTypeDef          GPIO_InitStruct;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    irqindex = __PIN_INDEX(pin);
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

    GPIO_InitStruct.Pin   = PIN_OFFSET(pin);
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = index->pin_pulls[__PIN_INDEX(pin)].pull_state;
    HAL_GPIO_Init(index->gpio_port, &GPIO_InitStruct);
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t stm32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    const struct pin_index   *index;
    const struct pin_irq_map *irqmap;
    os_base_t                 level;
    os_int32_t                irqindex = -1;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        irqindex = __PIN_INDEX(pin);
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
        HAL_NVIC_SetPriority(irqmap->irqno, 5, 0);
        HAL_NVIC_EnableIRQ(irqmap->irqno);
        pin_irq_enable_mask |= irqmap->pinbit;
        os_irq_unlock(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        irqmap = &pin_irq_map[__PIN_INDEX(pin)];
        if (irqmap == OS_NULL)
        {
            return OS_ENOSYS;
        }

        level = os_irq_lock();

        pin_irq_enable_mask &= ~irqmap->pinbit;
#if defined(SERIES_STM32F0) || defined(SERIES_STM32G0)
        if ((irqmap->pinbit >= GPIO_PIN_0) && (irqmap->pinbit <= GPIO_PIN_1))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_0 | GPIO_PIN_1)))
            {
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else if ((irqmap->pinbit >= GPIO_PIN_2) && (irqmap->pinbit <= GPIO_PIN_3))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_2 | GPIO_PIN_3)))
            {
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else if ((irqmap->pinbit >= GPIO_PIN_4) && (irqmap->pinbit <= GPIO_PIN_15))
        {
            if(!(pin_irq_enable_mask&(GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|
                                      GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15)))
            {    
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else
        {
            HAL_NVIC_DisableIRQ(irqmap->irqno);
        }
#else
        if ((irqmap->pinbit >= GPIO_PIN_5) && (irqmap->pinbit <= GPIO_PIN_9))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9)))
            {
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else if ((irqmap->pinbit >= GPIO_PIN_10) && (irqmap->pinbit <= GPIO_PIN_15))
        {
            if (!(pin_irq_enable_mask &
                  (GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)))
            {
                HAL_NVIC_DisableIRQ(irqmap->irqno);
            }
        }
        else
        {
            HAL_NVIC_DisableIRQ(irqmap->irqno);
        }
#endif
        os_irq_unlock(level);
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}
const static struct os_pin_ops _stm32_pin_ops = {
    stm32_pin_mode,
    stm32_pin_write,
    stm32_pin_read,
    stm32_pin_attach_irq,
    stm32_pin_dettach_irq,
    stm32_pin_irq_enable,
};

OS_INLINE void pin_irq_hdr(int irqno)
{
    if (pin_irq_hdr_tab[irqno].hdr)
    {
        pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}

void HAL_GPIO_EXTI_Rising_Callback(os_uint16_t GPIO_Pin)
{
    pin_irq_hdr(bit2bitno(GPIO_Pin));
}

void HAL_GPIO_EXTI_Falling_Callback(os_uint16_t GPIO_Pin)
{
    pin_irq_hdr(bit2bitno(GPIO_Pin));
}

void HAL_GPIO_EXTI_Callback(os_uint16_t GPIO_Pin)
{
    pin_irq_hdr(bit2bitno(GPIO_Pin));
}

void EXTI0_1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}
void EXTI4_15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_TSC_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void EXTI5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}

void EXTI6_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
}
void EXTI7_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

void EXTI8_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
}
void EXTI9_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

void EXTI10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
}
void EXTI11_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
}

void EXTI12_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
}
void EXTI13_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void EXTI14_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
}
void EXTI15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
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
    struct pin_index *tmp = OS_NULL;
    os_uint8_t gpio_port = 0;
    os_uint8_t gpio_pin = 0;

    __os_hw_pin_init();

    for(gpio_port=0;gpio_port<GPIO_PORT_MAX;gpio_port++)
    {
        if((gpio_port_map & (1<<gpio_port)))
        {
            tmp = (struct pin_index *)os_calloc(1, sizeof(struct pin_index));
            if(tmp == OS_NULL)
            {
                LOG_E(DBG_TAG,"os_malloc error!!!");
                return OS_ENOMEM;
            }
            
            tmp->gpio_port = gpio_port_base[gpio_port];

            for(gpio_pin=0;gpio_pin<GPIO_PIN_PER_PORT;gpio_pin++)
            {
                tmp->pin_pulls[gpio_pin].pull_state = 0;
            }

            indexs[gpio_port] = tmp;
            
        }
        else
        {
            indexs[gpio_port] = OS_NULL;
        }
    }

    return os_device_pin_register(0, &_stm32_pin_ops, OS_NULL);
}

#endif /* OS_USING_PIN */

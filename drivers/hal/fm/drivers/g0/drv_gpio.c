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
 * @brief       This file implements gpio driver for htxx.
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

#define DBG_TAG "drv.gpio"
#include <dlog.h>

#ifdef OS_USING_PIN

os_uint32_t        gpio_port_map = 0;
static struct pin_index *indexs[GPIO_PORT_MAX];
GPIOx_Type *gpio_port_base[GPIO_PORT_MAX];
static os_list_node_t fm_gpio_irq_list = OS_LIST_INIT(fm_gpio_irq_list);

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

#define GPIO_INFO_MAP(gpio)                                                                                               \
{                                                                                                                         \
    gpio_port_map |= (1 << GPIO_INDEX_##gpio);                                                                            \
    gpio_port_base[GPIO_INDEX_##gpio] = GPIO##gpio;                                                                       \
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

#if defined(GPIOZ)
    GPIO_INFO_MAP(Z);
#endif
}

static void HAL_GPIO_WritePin(GPIOx_Type* GPIOx, uint32_t GPIO_Pin, os_uint8_t val)
{
    if(val)
    {
        GPIO_SetBits(GPIOx, GPIO_Pin);
    }
    else
    {
        GPIO_ResetBits(GPIOx, GPIO_Pin);
    }
}
static os_uint8_t HAL_GPIO_ReadPin(GPIOx_Type* GPIOx, uint32_t GPIO_Pin)
{
    if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/*GPIO_Pin_n*/
static struct pin_index *get_pin_index(os_base_t pin)
{
    struct pin_index *index = OS_NULL;

    if(pin<GPIO_PIN_MAX)
    {
        index = indexs[__PORT_INDEX(pin)];
    }
    
    return index;
}
/*GPIOx*/
GPIOx_Type *get_pin_base(os_base_t pin)
{
    struct pin_index *index = get_pin_index(pin);

    if(index != OS_NULL)
    {
        return index->gpio_port;
    }
    return OS_NULL;
}

static void fm_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    const struct pin_index *index;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }

    HAL_GPIO_WritePin(index->gpio_port, PIN_OFFSET(pin), value);
}

static int fm_pin_read(struct os_device *dev, os_base_t pin)
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

static void fm_exti_clk_enable(void)
{
    RCC_PERCLK_SetableEx(EXTI2CLK, ENABLE);
    RCC_PERCLK_SetableEx(EXTI1CLK, ENABLE);
    RCC_PERCLK_SetableEx(EXTI0CLK, ENABLE);
    RCC_PERCLK_SetableEx(PDCCLK, ENABLE);
}
static void fm_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    struct pin_index    *index;
    os_base_t            level;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }

    CloseeIO(index->gpio_port, PIN_OFFSET(pin));
    level = os_irq_lock();
    index->pin_pulls[__PIN_INDEX(pin)].pull_state = IN_NORMAL;
    os_irq_unlock(level);

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        OutputIO(index->gpio_port, PIN_OFFSET(pin), OUT_PUSHPULL);
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        InputtIO(index->gpio_port, PIN_OFFSET(pin), IN_NORMAL);
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        InputtIO(index->gpio_port, PIN_OFFSET(pin), IN_PULLUP);
        level = os_irq_lock();
        index->pin_pulls[__PIN_INDEX(pin)].pull_state = IN_PULLUP;
        os_irq_unlock(level);
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        LOG_E(DBG_TAG, "pull down not surrported.");
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        OutputIO(index->gpio_port, PIN_OFFSET(pin), OUT_OPENDRAIN);
    }
//    else if(mode == PIN_MODE_ANALOG)
//    {
//        /* analog setting: analog */
//        AnalogIO(index->gpio_port, PIN_OFFSET(pin));
//    }
    
}

static os_err_t fm_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index     *index;
    struct pin_irq_param       *irq;
    os_base_t                   level;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    level = os_irq_lock();
    os_list_for_each_entry(irq, &fm_gpio_irq_list, struct pin_irq_param, list)
    {
        if(irq->pin == pin) 
        {
            if(irq->hdr  == hdr   &&
               irq->mode == mode  &&
               irq->args == args)
            {
               os_irq_unlock(level);
               return OS_EOK;
            }
            else
            {
                os_irq_unlock(level);
                return OS_EBUSY;
            }
        }
    }

    irq = os_calloc(1, sizeof(struct pin_irq_param));
    OS_ASSERT(irq != OS_NULL);

    irq->pin  = pin;
    irq->hdr  = hdr;
    irq->mode = mode;
    irq->args = args; 

    os_list_add_tail(&fm_gpio_irq_list, &irq->list);
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t fm_pin_dettach_irq(struct os_device *device, os_int32_t pin)
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
    os_list_for_each_entry(irq, &fm_gpio_irq_list, struct pin_irq_param, list)
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

static os_err_t fm_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    const struct pin_index   *index;
    os_base_t                 level;
    struct pin_irq_param     *irq;

    fm_exti_clk_enable();

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
    
    if (enabled == PIN_IRQ_ENABLE)
    {
        level = os_irq_lock();

        os_list_for_each_entry(irq, &fm_gpio_irq_list, struct pin_irq_param, list)
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
                    GPIO_EXTI_Init(index->gpio_port, PIN_OFFSET(pin), EXTI_RISING);
                    break;
                case PIN_IRQ_MODE_FALLING:
                    GPIO_EXTI_Init(index->gpio_port, PIN_OFFSET(pin), EXTI_FALLING);
                    break;
                case PIN_IRQ_MODE_HIGH_LEVEL:
                case PIN_IRQ_MODE_LOW_LEVEL:
                    LOG_E(DBG_TAG, "level trig mode not supported.");
                    break;
                case PIN_IRQ_MODE_RISING_FALLING:
                    GPIO_EXTI_Init(index->gpio_port, PIN_OFFSET(pin), EXTI_BOTH);
                    break;
                default:
                    os_irq_unlock(level);
                    return OS_ENOSYS;
                }
                irq->irq_ref++;
                NVIC_EnableIRQ(GPIO_IRQn);
                os_irq_unlock(level);
            }
        }
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        level = os_irq_lock();
        os_list_for_each_entry(irq, &fm_gpio_irq_list, struct pin_irq_param, list)
        {
           if(irq->pin == pin) 
           {
              if(irq->irq_ref == 0) 
              {
                os_irq_unlock(level);
                return OS_EOK;
              }
              GPIO_EXTI_Close(index->gpio_port, PIN_OFFSET(pin));

              irq->irq_ref--;
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
const static struct os_pin_ops _fm_pin_ops = {
    fm_pin_mode,
    fm_pin_write,
    fm_pin_read,
    fm_pin_attach_irq,
    fm_pin_dettach_irq,
    fm_pin_irq_enable,
};

OS_INLINE void pin_irq_hdr(void)
{
    struct pin_index         *index;
    struct pin_irq_param     *irq;

    os_list_for_each_entry(irq, &fm_gpio_irq_list, struct pin_irq_param, list)
    {
        index = get_pin_index(irq->pin);

        if(SET == GPIO_EXTI_EXTIxIF_ChkEx(index->gpio_port, PIN_OFFSET(irq->pin)))
        {
            GPIO_EXTI_EXTIxIF_ClrEx(index->gpio_port, PIN_OFFSET(irq->pin));
            if(index->handler_mask == OS_TRUE)
            {
                index->handler_mask = OS_FALSE;
#if 0
                return;
#endif
            }
            if(irq->hdr != OS_NULL)
            {
                irq->hdr(irq->args);
            }
        }
    }
}


void GPIO_IRQHandler(void)
{
    pin_irq_hdr();
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
    struct pin_index *tmp       = OS_NULL;
    os_uint8_t        gpio_port = 0;
    os_uint8_t        gpio_pin  = 0;

    __os_hw_pin_init();

    for(gpio_port=0;gpio_port<GPIO_PORT_MAX;gpio_port++)
    {
        if((gpio_port_map & (1<<gpio_port)))
        {
            tmp = (struct pin_index *)os_malloc(sizeof(struct pin_index));
            if(tmp == OS_NULL)
            {
                os_kprintf(DBG_TAG, "os_malloc error!!!");
                return OS_ENOMEM;
            }
            
            tmp->gpio_port = gpio_port_base[gpio_port];
            /*Handler mask is a circumvention measure used to mask false interrupt 
            that is generated after the interrupt is enabled*/
            tmp->handler_mask  = OS_TRUE;

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

    return os_device_pin_register(0, &_fm_pin_ops, OS_NULL);
}

#endif /* OS_USING_PIN */

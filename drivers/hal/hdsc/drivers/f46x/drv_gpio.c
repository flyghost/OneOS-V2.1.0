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
 * 2021-06-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include "drv_gpio.h"

#define DBG_TAG "drv_gpio"

os_uint32_t gpio_port_map = 0;
static struct pin_index *indexs[GPIO_PORT_MAX];
en_port_t gpio_port_base[GPIO_PORT_MAX];
static os_list_node_t hc_gpio_irq_list = OS_LIST_INIT(hc_gpio_irq_list);

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

#define GPIO_INFO_MAP(gpio)                                                                                               \
{                                                                                                                         \
    gpio_port_map |= (1 << GPIO_INDEX_##gpio);                                                                            \
    gpio_port_base[GPIO_INDEX_##gpio] = Port##gpio;                                                                       \
}

static struct pin_irq_map hc_pin_irq_map[];

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

static struct pin_index *get_pin_index(os_base_t pin)
{
    struct pin_index *index = OS_NULL;

    if(pin < GPIO_PIN_MAX)
    {
        index = indexs[__PORT_INDEX(pin)];
    }

    return index;
}

en_port_t  get_pin_base(os_base_t pin)
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

    if(value != 0)
        PORT_SetBits(index->gpio_port, 1 << PIN_OFFSET(pin));
    else
        PORT_ResetBits(index->gpio_port, 1 << PIN_OFFSET(pin));
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

    value = PORT_GetBit(index->gpio_port, (en_pin_t)(1 << PIN_OFFSET(pin)));

    return value;
}

static void hc32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    struct pin_index *index;
    stc_port_init_t GPIO_Cfg;

    MEM_ZERO_STRUCT(GPIO_Cfg);

#if (PIN_PULL_STATE)
    os_base_t         level;
#endif

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }

    switch(mode) {
        case PIN_MODE_OUTPUT:
            /* output setting */
            GPIO_Cfg.enPinMode = Pin_Mode_Out;
            GPIO_Cfg.enPullUp = Enable;
            break;
        case PIN_MODE_INPUT:
            /* input setting: not pull. */
            GPIO_Cfg.enPinMode = Pin_Mode_In;
            break;

        case PIN_MODE_INPUT_PULLUP:
            /* input setting: pull up. */
            GPIO_Cfg.enPinMode = Pin_Mode_In;
            GPIO_Cfg.enPullUp = Enable;

            break;
        case PIN_MODE_INPUT_PULLDOWN:

            /* input setting: pull down. */
            GPIO_Cfg.enPinMode = Pin_Mode_In;
            GPIO_Cfg.enPullUp = Disable;
            break;
        case PIN_MODE_OUTPUT_OD:
            /* output setting: od. */
            GPIO_Cfg.enPinMode = Pin_Mode_Out;
            GPIO_Cfg.enLatch = Enable;
            GPIO_Cfg.enPinOType = Pin_OType_Od;
            GPIO_Cfg.enPinDrv = Pin_Drv_H;

            break;

        default:

            break;
    }
    
#if (PIN_PULL_STATE)
    /* remeber the pull state. */
    level = os_irq_lock();
    index->pin_pulls[__PIN_INDEX(pin)].pull_up = GPIO_Cfg.enPullUp;
    os_irq_unlock(level);
#endif

    PORT_Init(index->gpio_port, 1 << PIN_OFFSET(pin), &GPIO_Cfg);
}

static os_err_t
hc32_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    struct pin_index       *index = OS_NULL;
    os_base_t               level;
    struct pin_irq_param   *irq = OS_NULL;
    struct pin_irq_param   *tmp = OS_NULL;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    irq = (struct pin_irq_param *)os_calloc(1, sizeof(struct pin_irq_param));
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
            os_free(irq);
            return OS_EOK;
        }
    }
    os_irq_unlock(level);
    
    return OS_EEMPTY;
}

static os_err_t hc32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    struct pin_index   *index = OS_NULL;
    os_base_t                 level = 0;
    struct pin_irq_param     *irq = OS_NULL;
    
    stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);
    
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
                if(hc_pin_irq_map[__PIN_INDEX(pin)].pin_param == irq)
                {
                    os_irq_unlock(level);
                    return OS_EOK;
                }

                if(OS_NULL != hc_pin_irq_map[__PIN_INDEX(pin)].pin_param)
                {
                    os_irq_unlock(level);
                    return OS_EBUSY;
                }
                
                switch (irq->mode)
                {
                    case PIN_IRQ_MODE_RISING:
                        stcExtiConfig.enExtiLvl = ExIntRisingEdge;
                        break;
                    case PIN_IRQ_MODE_FALLING:
                        stcExtiConfig.enExtiLvl = ExIntFallingEdge;
                        break;
                    case PIN_IRQ_MODE_LOW_LEVEL:
                        stcExtiConfig.enExtiLvl = ExIntLowLevel;
                        break;
                    case PIN_IRQ_MODE_RISING_FALLING:
                        stcExtiConfig.enExtiLvl = ExIntBothEdge;
                    default:
                        os_irq_unlock(level);
                        return OS_ENOSYS;
                }

                stcExtiConfig.enExitCh = EXTI_CH(pin);
                stcExtiConfig.enFilterEn = Enable;
                stcExtiConfig.enFltClk = Pclk3Div8;
                EXINT_Init(&stcExtiConfig);

                stcPortInit.enExInt = Enable;
                stcPortInit.enPullUp = index->pin_pulls[__PIN_INDEX(pin)].pull_up;
                PORT_Init(index->gpio_port, 1 << PIN_OFFSET(pin), &stcPortInit);

                stcIrqRegiConf.enIntSrc = INT_SRC(pin);
                stcIrqRegiConf.enIRQn = hc_pin_irq_map[__PIN_INDEX(pin)].irqno;
                stcIrqRegiConf.pfnCallback = hc_pin_irq_map[__PIN_INDEX(pin)].pfnCallback;
                enIrqRegistration(&stcIrqRegiConf);

                NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
                NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
                NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

                hc_pin_irq_map[__PIN_INDEX(pin)].pin_param = irq;
            }
        }

        os_irq_unlock(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        level = os_irq_lock();
        
        os_list_for_each_entry(irq, &hc_gpio_irq_list, struct pin_irq_param, list)
        {
            if(irq->pin == pin)
            {
                if(hc_pin_irq_map[__PIN_INDEX(pin)].pin_param == OS_NULL)
                {
                    os_irq_unlock(level);
                    return OS_EOK;
                }

                switch (irq->mode)
                {
                    case PIN_IRQ_MODE_RISING:
                    case PIN_IRQ_MODE_FALLING:
                    case PIN_IRQ_MODE_HIGH_LEVEL:
                    case PIN_IRQ_MODE_LOW_LEVEL:
                    case PIN_IRQ_MODE_RISING_FALLING:
                        break;
                    default:
                        os_irq_unlock(level);
                        return OS_ENOSYS;
                }

                stcPortInit.enExInt = Disable;
                stcPortInit.enPullUp = index->pin_pulls[__PIN_INDEX(pin)].pull_up;
                PORT_Init(index->gpio_port, 1 << PIN_OFFSET(pin), &stcPortInit);

                stcExtiConfig.enExitCh = EXTI_CH(pin);
                stcExtiConfig.enFilterEn = Disable;
                stcExtiConfig.enFltClk = Pclk3Div8;
                EXINT_Init(&stcExtiConfig);

                stcIrqRegiConf.enIRQn = hc_pin_irq_map[__PIN_INDEX(pin)].irqno;
                enIrqResign(stcIrqRegiConf.enIRQn);

                NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
                NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
                
                hc_pin_irq_map[__PIN_INDEX(pin)].pin_param = OS_NULL;
            }
        }
        
        os_irq_unlock(level);
    }
    else
    {
        return OS_ENOSYS;
    }
    
    return OS_EOK;
}

static void HAL_GPIO_EXTI_IRQHandler(os_uint8_t index)
{
    if (Set == EXINT_IrqFlgGet(EXTI_CH(index)))
    {
        if (hc_pin_irq_map[index].pin_param->hdr)
        {
            hc_pin_irq_map[index].pin_param->hdr(hc_pin_irq_map[index].pin_param->args);
        }

        EXINT_IrqFlgClr(EXTI_CH(index));
    }

}

static void ExtiCh00_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(0);
}

static void ExtiCh01_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(1);
}

static void ExtiCh02_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(2);
}

static void ExtiCh03_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(3);
}

static void ExtiCh04_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(4);
}

static void ExtiCh05_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(5);
}
static void ExtiCh06_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(6);
}

static void ExtiCh07_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(7);
}

static void ExtiCh08_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(8);
}
static void ExtiCh09_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(9);
}

static void ExtiCh10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(10);
}

static void ExtiCh11_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(11);
}

static void ExtiCh12_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(12);
}

static void ExtiCh13_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(13);
}

static void ExtiCh14_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(14);
}

static void ExtiCh15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(15);
}

struct pin_irq_map hc_pin_irq_map[GPIO_PIN_PER_PORT] = 
{
    {OS_NULL, Int000_IRQn, ExtiCh00_IRQHandler},
    {OS_NULL, Int001_IRQn, ExtiCh01_IRQHandler},
    {OS_NULL, Int002_IRQn, ExtiCh02_IRQHandler},
    {OS_NULL, Int003_IRQn, ExtiCh03_IRQHandler},
    {OS_NULL, Int004_IRQn, ExtiCh04_IRQHandler},
    {OS_NULL, Int005_IRQn, ExtiCh05_IRQHandler},
    {OS_NULL, Int006_IRQn, ExtiCh06_IRQHandler},
    {OS_NULL, Int007_IRQn, ExtiCh07_IRQHandler},
    {OS_NULL, Int008_IRQn, ExtiCh08_IRQHandler},
    {OS_NULL, Int009_IRQn, ExtiCh09_IRQHandler},
    {OS_NULL, Int010_IRQn, ExtiCh10_IRQHandler},
    {OS_NULL, Int011_IRQn, ExtiCh11_IRQHandler},
    {OS_NULL, Int012_IRQn, ExtiCh12_IRQHandler},
    {OS_NULL, Int013_IRQn, ExtiCh13_IRQHandler},
    {OS_NULL, Int014_IRQn, ExtiCh14_IRQHandler},
    {OS_NULL, Int015_IRQn, ExtiCh15_IRQHandler},
};

const static struct os_pin_ops _hc32_pin_ops = {
    .pin_mode       = hc32_pin_mode,
    .pin_write      = hc32_pin_write,
    .pin_read       = hc32_pin_read,
    .pin_attach_irq = hc32_pin_attach_irq,
    .pin_detach_irq = hc32_pin_dettach_irq,
    .pin_irq_enable = hc32_pin_irq_enable,
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
int os_hw_pin_init(void)
{
    struct pin_index *tmp = OS_NULL;
    os_uint8_t gpio_port = 0;

#if (PIN_PULL_STATE)
    os_uint8_t gpio_pin = 0;
#endif

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
                tmp->pin_pulls[gpio_pin].pull_up   = Disable;
                tmp->pin_pulls[gpio_pin].pull_down = Disable;
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


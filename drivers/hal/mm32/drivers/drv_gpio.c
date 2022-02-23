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
 * @brief       This file implements gpio driver for MM32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <mm32_hal.h>
#include "drv_gpio.h"
#include <os_memory.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.gpio"
#include <drv_log.h>

static const pin_irq_table_t pin_irq_table[];
static struct os_pin_irq_hdr irq_hdr_table[];
static gpio_port_info_t port_info_table[];

static mm32_gpio_info_t mm32_gpio_info =
{
#if defined(SERIES_MM32F013XX) || defined(SERIES_MM32F027XX) || defined(SERIES_MM32F327XX)
    .gpio_rcc_clkcmd    = RCC_AHBPeriphClockCmd,
    .exti_rcc_clkcmd    = RCC_APB2PeriphClockCmd,
    .exti_periph        = RCC_APB2ENR_EXTI,
    .exti_lineconfig    = SYSCFG_EXTILineConfig,
#elif defined(SERIES_MM32F103XX) || defined(SERIES_MM32L3XX)
    .gpio_rcc_clkcmd    = RCC_APB2PeriphClockCmd,
    .exti_rcc_clkcmd    = RCC_APB2PeriphClockCmd,
    .exti_periph        = RCC_APB2Periph_AFIO,
    .exti_lineconfig    = GPIO_EXTILineConfig,
#elif defined(SERIES_MM32SPIN2XX)
    .gpio_rcc_clkcmd    = RCC_AHBPeriphClockCmd,
    .exti_rcc_clkcmd    = OS_NULL,
    .exti_periph        = 0,
    .exti_lineconfig    = OS_NULL,
#endif
#if defined(SERIES_MM32F327XX) || defined(SERIES_MM32F103XX) || defined(SERIES_MM32L3XX)
    .NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0,
    .NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0,
#elif defined(SERIES_MM32F013XX) || defined(SERIES_MM32F027XX) || defined(SERIES_MM32SPIN2XX)
    .NVIC_InitStructure.NVIC_IRQChannelPriority = 0,
#endif
    .NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE,
    .GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU,
    .GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz,
    .info_tab           = port_info_table,
    .irq_tab            = pin_irq_table,
    .irq_hdr_tab        = irq_hdr_table,
};

static const pin_irq_table_t pin_irq_table[] = 
{
#if defined(SERIES_MM32F103XX) || defined(SERIES_MM32F327XX) || defined(SERIES_MM32L3XX)
    {GPIO_Pin_0,  GPIO_PinSource0,  EXTI_Line0,  EXTI0_IRQn    },
    {GPIO_Pin_1,  GPIO_PinSource1,  EXTI_Line1,  EXTI1_IRQn    },
    {GPIO_Pin_2,  GPIO_PinSource2,  EXTI_Line2,  EXTI2_IRQn    },
    {GPIO_Pin_3,  GPIO_PinSource3,  EXTI_Line3,  EXTI3_IRQn    },
    {GPIO_Pin_4,  GPIO_PinSource4,  EXTI_Line4,  EXTI4_IRQn    },
    {GPIO_Pin_5,  GPIO_PinSource5,  EXTI_Line5,  EXTI9_5_IRQn  },
    {GPIO_Pin_6,  GPIO_PinSource6,  EXTI_Line6,  EXTI9_5_IRQn  },
    {GPIO_Pin_7,  GPIO_PinSource7,  EXTI_Line7,  EXTI9_5_IRQn  },
    {GPIO_Pin_8,  GPIO_PinSource8,  EXTI_Line8,  EXTI9_5_IRQn  },
    {GPIO_Pin_9,  GPIO_PinSource9,  EXTI_Line9,  EXTI9_5_IRQn  },
    {GPIO_Pin_10, GPIO_PinSource10, EXTI_Line10, EXTI15_10_IRQn},
    {GPIO_Pin_11, GPIO_PinSource11, EXTI_Line11, EXTI15_10_IRQn},
    {GPIO_Pin_12, GPIO_PinSource12, EXTI_Line12, EXTI15_10_IRQn},
    {GPIO_Pin_13, GPIO_PinSource13, EXTI_Line13, EXTI15_10_IRQn},
    {GPIO_Pin_14, GPIO_PinSource14, EXTI_Line14, EXTI15_10_IRQn},
    {GPIO_Pin_15, GPIO_PinSource15, EXTI_Line15, EXTI15_10_IRQn},
#elif defined(SERIES_MM32F013XX) || defined(SERIES_MM32F027XX) || defined(SERIES_MM32SPIN2XX)
    {GPIO_Pin_0,  GPIO_PinSource0,  EXTI_Line0,  EXTI0_1_IRQn },
    {GPIO_Pin_1,  GPIO_PinSource1,  EXTI_Line1,  EXTI0_1_IRQn },
    {GPIO_Pin_2,  GPIO_PinSource2,  EXTI_Line2,  EXTI2_3_IRQn },
    {GPIO_Pin_3,  GPIO_PinSource3,  EXTI_Line3,  EXTI2_3_IRQn },
    {GPIO_Pin_4,  GPIO_PinSource4,  EXTI_Line4,  EXTI4_15_IRQn},
    {GPIO_Pin_5,  GPIO_PinSource5,  EXTI_Line5,  EXTI4_15_IRQn},
    {GPIO_Pin_6,  GPIO_PinSource6,  EXTI_Line6,  EXTI4_15_IRQn},
    {GPIO_Pin_7,  GPIO_PinSource7,  EXTI_Line7,  EXTI4_15_IRQn},
    {GPIO_Pin_8,  GPIO_PinSource8,  EXTI_Line8,  EXTI4_15_IRQn},
    {GPIO_Pin_9,  GPIO_PinSource9,  EXTI_Line9,  EXTI4_15_IRQn},
    {GPIO_Pin_10, GPIO_PinSource10, EXTI_Line10, EXTI4_15_IRQn},
    {GPIO_Pin_11, GPIO_PinSource11, EXTI_Line11, EXTI4_15_IRQn},
    {GPIO_Pin_12, GPIO_PinSource12, EXTI_Line12, EXTI4_15_IRQn},
    {GPIO_Pin_13, GPIO_PinSource13, EXTI_Line13, EXTI4_15_IRQn},
    {GPIO_Pin_14, GPIO_PinSource14, EXTI_Line14, EXTI4_15_IRQn},
    {GPIO_Pin_15, GPIO_PinSource15, EXTI_Line15, EXTI4_15_IRQn},
#endif
};

static gpio_port_info_t port_info_table[] =
{
#ifdef GPIOA
    GPIO_PORT_INFO_MAP(A),
#endif
#ifdef GPIOB
    GPIO_PORT_INFO_MAP(B),
#endif
#ifdef GPIOC
    GPIO_PORT_INFO_MAP(C),
#endif
#ifdef GPIOD
    GPIO_PORT_INFO_MAP(D),
#endif
#ifdef GPIOE
#if defined(SERIES_MM32F027XX)
    GPIO_PORT_INFO_MAP_NODEF(E),
#else
    GPIO_PORT_INFO_MAP(E),
#endif
#endif
#ifdef GPIOF
#if defined(SERIES_MM32F027XX)
    GPIO_PORT_INFO_MAP_NODEF(F),
#else
    GPIO_PORT_INFO_MAP(F),
#endif
#endif
#ifdef GPIOG
    GPIO_PORT_INFO_MAP(G),
#endif
#ifdef GPIOH
    GPIO_PORT_INFO_MAP(H),
#endif
};

static struct os_pin_irq_hdr irq_hdr_table[] = {
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

static void mm32_pin_init(void)
{
    os_uint8_t i = 0;
    
    mm32_gpio_info.port_info_tab_size = GPIO_ITEM_NUM(port_info_table);
    mm32_gpio_info.irq_tab_size = GPIO_ITEM_NUM(pin_irq_table);
    mm32_gpio_info.irq_hdr_tab_size = GPIO_ITEM_NUM(irq_hdr_table);
    mm32_gpio_info.gpio_pin_max = mm32_gpio_info.port_info_tab_size * GPIO_PIN_PER_PORT;
    
    for (i = 0;i < mm32_gpio_info.port_info_tab_size;i++)
    {
        mm32_gpio_info.gpio_rcc_clkcmd(mm32_gpio_info.info_tab[i].periph, ENABLE);
    }

#if defined(SERIES_MM32F327XX)
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_15);  //Disable JTDI   AF to  AF15
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_15);   //Disable JTDO/TRACESWO   AF to  AF15
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_15);   //Disable NJRST   AF to  AF15
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_15);   //Disable AF Funtion   AF to  AF15
#elif defined(SERIES_MM32F103XX) || defined(SERIES_MM32L3XX)
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
#endif
}

static gpio_port_info_t *get_pin_port_info(os_base_t pin)
{
    gpio_port_info_t *info = OS_NULL;

    if (pin < mm32_gpio_info.gpio_pin_max)
    {
        info = &mm32_gpio_info.info_tab[__PORT_INDEX(pin)];
    }
    
    return info;
};

static const pin_irq_table_t *get_pin_irq_table(os_base_t pin)
{
    const pin_irq_table_t *info = OS_NULL;

    if (pin < mm32_gpio_info.gpio_pin_max)
    {
        info = &mm32_gpio_info.irq_tab[__PIN_INDEX(pin)];
    }
    
    return info;
};

static void MM32_pin_write(struct os_device *dev, os_base_t pin, os_base_t value)
{
    gpio_port_info_t *info = OS_NULL;
    const pin_irq_table_t  *irq_table = OS_NULL;
    
    info = get_pin_port_info(pin);
    irq_table = get_pin_irq_table(pin);
    if ((info == OS_NULL) || (irq_table == OS_NULL))
        return;

    GPIO_WriteBit(info->port, irq_table->irqbit, (BitAction)value);
}

static int MM32_pin_read(struct os_device *dev, os_base_t pin)
{
    int value = PIN_LOW;

    gpio_port_info_t *info = OS_NULL;
    const pin_irq_table_t  *irq_table = OS_NULL;
    
    info = get_pin_port_info(pin);
    irq_table = get_pin_irq_table(pin);
    if ((info == OS_NULL) || (irq_table == OS_NULL))
        return value;

    if (GPIO_ReadInputDataBit(info->port, irq_table->irqbit) == Bit_RESET)
    {
        value = PIN_LOW;
    }
    else
    {
        value = PIN_HIGH;
    }  

    return value;
}

static void MM32_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    os_base_t               level = 0;
    os_int32_t              pin_index = -1;
    
    GPIO_InitTypeDef        GPIO_InitStruct;
    
    gpio_port_info_t *info = OS_NULL;
    const pin_irq_table_t  *irq_table = OS_NULL;
    
    info = get_pin_port_info(pin);
    irq_table = get_pin_irq_table(pin);
    if ((info == OS_NULL) || (irq_table == OS_NULL))
        return;

    pin_index = __PIN_INDEX(pin);
    if (pin_index < 0 || pin_index >= mm32_gpio_info.irq_tab_size)
        return;
    
    GPIO_InitStruct.GPIO_Pin   = irq_table->irqbit;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    if (mode == PIN_MODE_OUTPUT)
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; 
    else if (mode == PIN_MODE_INPUT)
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    else if (mode == PIN_MODE_INPUT_PULLUP)
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    else if (mode == PIN_MODE_OUTPUT_OD)
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    else
        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPD;
   
    level = os_irq_lock();
    info->pin_pulls[__PIN_INDEX(pin)].pull_state = GPIO_InitStruct.GPIO_Mode;
    os_irq_unlock(level);
    GPIO_Init(info->port, &GPIO_InitStruct);
}

static os_err_t
MM32_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    os_base_t               level;
    os_int32_t              pin_index = -1;
    
    gpio_port_info_t *info;

    info = get_pin_port_info(pin);
    if (info == OS_NULL)
    {
        return OS_ENOSYS;
    }
    pin_index = __PIN_INDEX(pin);
    if (pin_index < 0 || pin_index >= mm32_gpio_info.irq_tab_size)
    {
        return OS_ENOSYS;
    }

    level = os_irq_lock();
    if (mm32_gpio_info.irq_hdr_tab[pin_index].pin == pin   &&
        mm32_gpio_info.irq_hdr_tab[pin_index].hdr == hdr   &&
        mm32_gpio_info.irq_hdr_tab[pin_index].mode == mode &&
        mm32_gpio_info.irq_hdr_tab[pin_index].args == args)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
    if (mm32_gpio_info.irq_hdr_tab[pin_index].pin != -1)
    {
        os_irq_unlock(level);
        return OS_EBUSY;
    }
    mm32_gpio_info.irq_hdr_tab[pin_index].pin  = pin;
    mm32_gpio_info.irq_hdr_tab[pin_index].hdr  = hdr;
    mm32_gpio_info.irq_hdr_tab[pin_index].mode = mode;
    mm32_gpio_info.irq_hdr_tab[pin_index].args = args;
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t MM32_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    gpio_port_info_t *info;
    os_base_t               level;
    os_int32_t              pin_index = -1;

    info = get_pin_port_info(pin);
    if (info == OS_NULL)
    {
        return OS_ENOSYS;
    }
    pin_index = __PIN_INDEX(pin);
    if (pin_index < 0 || pin_index >= mm32_gpio_info.irq_tab_size)
    {
        return OS_ENOSYS;
    }

    level = os_irq_lock();
    if (mm32_gpio_info.irq_hdr_tab[pin_index].pin == -1)
    {
        os_irq_unlock(level);
        return OS_EOK;
    }
    mm32_gpio_info.irq_hdr_tab[pin_index].pin  = -1;
    mm32_gpio_info.irq_hdr_tab[pin_index].hdr  = OS_NULL;
    mm32_gpio_info.irq_hdr_tab[pin_index].mode = 0;
    mm32_gpio_info.irq_hdr_tab[pin_index].args = OS_NULL;
    os_irq_unlock(level);

    return OS_EOK;
}

static os_err_t MM32_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    gpio_port_info_t       *info = OS_NULL;
    const pin_irq_table_t  *irq_table = OS_NULL;
    os_base_t               level = 0;
    os_int32_t              pin_index = -1;
    
    EXTI_InitTypeDef EXTI_InitStructure;

    if (mm32_gpio_info.exti_rcc_clkcmd == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "not support gpio exti irq!");
        return OS_ERROR;
    }
    info = get_pin_port_info(pin);
    if (info == OS_NULL)
    {
        return OS_ENOSYS;
    }

    pin_index = __PIN_INDEX(pin);
    if (pin_index < 0 || pin_index >= mm32_gpio_info.irq_tab_size)
    {
        return OS_ENOSYS;
    }
    irq_table = &mm32_gpio_info.irq_tab[pin_index];
    
    if (mm32_gpio_info.irq_hdr_tab[pin_index].pin == -1)
    {
        return OS_ENOSYS;
    }
    
    if (enabled == PIN_IRQ_ENABLE)
    {
        level = os_irq_lock();
        mm32_gpio_info.GPIO_InitStructure.GPIO_Pin   = irq_table->pinbit;
        GPIO_Init(info->port, &mm32_gpio_info.GPIO_InitStructure);
        mm32_gpio_info.NVIC_InitStructure.NVIC_IRQChannel = irq_table->irqno;
        NVIC_Init(&mm32_gpio_info.NVIC_InitStructure);
        
        mm32_gpio_info.exti_rcc_clkcmd(mm32_gpio_info.exti_periph, ENABLE);
        mm32_gpio_info.exti_lineconfig(info->port_source, irq_table->pin_source);
        EXTI_InitStructure.EXTI_Line = irq_table->irqbit;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        switch (mm32_gpio_info.irq_hdr_tab[pin_index].mode)
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
        
        EXTI_ClearITPendingBit(irq_table->irqbit);
        
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        os_irq_unlock(level);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        EXTI_InitStructure.EXTI_Line = irq_table->irqbit;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
        EXTI_Init(&EXTI_InitStructure);
    }
    else
    {
        return OS_ENOSYS;
    }

    return OS_EOK;
}
const static struct os_pin_ops _MM32_pin_ops = 
{
    MM32_pin_mode,
    MM32_pin_write,
    MM32_pin_read,
    MM32_pin_attach_irq,
    MM32_pin_dettach_irq,
    MM32_pin_irq_enable,
};

void pin_irq_hdr(int irq_index)
{
    EXTI_ClearITPendingBit(mm32_gpio_info.irq_tab[irq_index].irqbit);

    if (mm32_gpio_info.irq_hdr_tab[irq_index].hdr)
    {
        mm32_gpio_info.irq_hdr_tab[irq_index].hdr(mm32_gpio_info.irq_hdr_tab[irq_index].args);
    }
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
    mm32_pin_init();

    return os_device_pin_register(0, &_MM32_pin_ops, OS_NULL);
}


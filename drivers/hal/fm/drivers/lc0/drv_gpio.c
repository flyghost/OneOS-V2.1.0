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

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.gpio"
#include <drv_log.h>

#ifdef OS_USING_PIN

os_uint32_t        gpio_port_map = 0;
static os_uint32_t pin_irq_enable_mask = 0;

static struct pin_index *indexs[GPIO_PORT_MAX];
GPIO_Type *gpio_port_base[GPIO_PORT_MAX];


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
    
static const struct pin_irq_map pin_irq_map[] = 
{
    {GET_PIN(A, 0 ), FL_GPIO_EXTI_LINE_0,  FL_GPIO_EXTI_LINE_0_PA0 },
    {GET_PIN(A, 1 ), FL_GPIO_EXTI_LINE_0,  FL_GPIO_EXTI_LINE_0_PA1 },
    {GET_PIN(A, 2 ), FL_GPIO_EXTI_LINE_0,  FL_GPIO_EXTI_LINE_0_PA2 },
    {GET_PIN(A, 3 ), FL_GPIO_EXTI_LINE_0,  FL_GPIO_EXTI_LINE_0_PA3 },
                                          
    {GET_PIN(A, 4 ), FL_GPIO_EXTI_LINE_1,  FL_GPIO_EXTI_LINE_1_PA4 },
    {GET_PIN(A, 5 ), FL_GPIO_EXTI_LINE_1,  FL_GPIO_EXTI_LINE_1_PA5 },
    {GET_PIN(A, 6 ), FL_GPIO_EXTI_LINE_1,  FL_GPIO_EXTI_LINE_1_PA6 },
    {GET_PIN(A, 7 ), FL_GPIO_EXTI_LINE_1,  FL_GPIO_EXTI_LINE_1_PA7 },
                                          
    {GET_PIN(A, 8 ), FL_GPIO_EXTI_LINE_2,  FL_GPIO_EXTI_LINE_2_PA8 },
    {GET_PIN(A, 9 ), FL_GPIO_EXTI_LINE_2,  FL_GPIO_EXTI_LINE_2_PA9 },
    {GET_PIN(A, 10), FL_GPIO_EXTI_LINE_2,  FL_GPIO_EXTI_LINE_2_PA10},
    {GET_PIN(A, 11), FL_GPIO_EXTI_LINE_2,  FL_GPIO_EXTI_LINE_2_PA11},
                                          
    {GET_PIN(A, 12), FL_GPIO_EXTI_LINE_3,  FL_GPIO_EXTI_LINE_3_PA12},
    {GET_PIN(A, 13), FL_GPIO_EXTI_LINE_3,  FL_GPIO_EXTI_LINE_3_PA13},
    {GET_PIN(A, 14), FL_GPIO_EXTI_LINE_3,  FL_GPIO_EXTI_LINE_3_PA14},
    {GET_PIN(A, 15), FL_GPIO_EXTI_LINE_3,  FL_GPIO_EXTI_LINE_3_PA15},
                                          
    {GET_PIN(B, 0 ), FL_GPIO_EXTI_LINE_4,  FL_GPIO_EXTI_LINE_4_PB0 },
    {GET_PIN(B, 1 ), FL_GPIO_EXTI_LINE_4,  FL_GPIO_EXTI_LINE_4_PB1 },
    {GET_PIN(B, 2 ), FL_GPIO_EXTI_LINE_4,  FL_GPIO_EXTI_LINE_4_PB2 },
    {GET_PIN(B, 3 ), FL_GPIO_EXTI_LINE_4,  FL_GPIO_EXTI_LINE_4_PB3 },
                                          
    {GET_PIN(B, 4 ), FL_GPIO_EXTI_LINE_5,  FL_GPIO_EXTI_LINE_5_PB4 },
    {GET_PIN(B, 5 ), FL_GPIO_EXTI_LINE_5,  FL_GPIO_EXTI_LINE_5_PB5 },
    {GET_PIN(B, 6 ), FL_GPIO_EXTI_LINE_5,  FL_GPIO_EXTI_LINE_5_PB6 },
    {GET_PIN(B, 7 ), FL_GPIO_EXTI_LINE_5,  FL_GPIO_EXTI_LINE_5_PB7 },
                                          
    {GET_PIN(B, 8 ), FL_GPIO_EXTI_LINE_6,  FL_GPIO_EXTI_LINE_6_PB8 },
    {GET_PIN(B, 9 ), FL_GPIO_EXTI_LINE_6,  FL_GPIO_EXTI_LINE_6_PB9 },
    {GET_PIN(B, 10), FL_GPIO_EXTI_LINE_6,  FL_GPIO_EXTI_LINE_6_PB10},
    {GET_PIN(B, 11), FL_GPIO_EXTI_LINE_6,  FL_GPIO_EXTI_LINE_6_PB11},
                                          
    {GET_PIN(B, 12), FL_GPIO_EXTI_LINE_7,  FL_GPIO_EXTI_LINE_7_PB12},
    {GET_PIN(B, 13), FL_GPIO_EXTI_LINE_7,  FL_GPIO_EXTI_LINE_7_PB13},
    {GET_PIN(B, 14), FL_GPIO_EXTI_LINE_7,  FL_GPIO_EXTI_LINE_7_PB14},
    {GET_PIN(B, 15), FL_GPIO_EXTI_LINE_7,  FL_GPIO_EXTI_LINE_7_PB15},
                                          
    {GET_PIN(C, 0 ), FL_GPIO_EXTI_LINE_8,  FL_GPIO_EXTI_LINE_8_PC0 },
    {GET_PIN(C, 1 ), FL_GPIO_EXTI_LINE_8,  FL_GPIO_EXTI_LINE_8_PC1 },
    {GET_PIN(C, 2 ), FL_GPIO_EXTI_LINE_8,  FL_GPIO_EXTI_LINE_8_PC2 },
    {GET_PIN(C, 3 ), FL_GPIO_EXTI_LINE_8,  FL_GPIO_EXTI_LINE_8_PC3 },
                                          
    {GET_PIN(C, 4 ), FL_GPIO_EXTI_LINE_9,  FL_GPIO_EXTI_LINE_9_PC4 },
    {GET_PIN(C, 5 ), FL_GPIO_EXTI_LINE_9,  FL_GPIO_EXTI_LINE_9_PC5 },
    {GET_PIN(C, 6 ), FL_GPIO_EXTI_LINE_9,  FL_GPIO_EXTI_LINE_9_PC6 },
    {GET_PIN(C, 7 ), FL_GPIO_EXTI_LINE_9,  FL_GPIO_EXTI_LINE_9_PC7 },

    {GET_PIN(C, 8 ), FL_GPIO_EXTI_LINE_10, FL_GPIO_EXTI_LINE_10_PC8 },
    {GET_PIN(C, 9 ), FL_GPIO_EXTI_LINE_10, FL_GPIO_EXTI_LINE_10_PC9 },
    {GET_PIN(C, 10), FL_GPIO_EXTI_LINE_10, FL_GPIO_EXTI_LINE_10_PC10},
    {GET_PIN(C, 11), FL_GPIO_EXTI_LINE_10, FL_GPIO_EXTI_LINE_10_PC11},

    {GET_PIN(D, 0 ), FL_GPIO_EXTI_LINE_12, FL_GPIO_EXTI_LINE_12_PD0 },
    {GET_PIN(D, 1 ), FL_GPIO_EXTI_LINE_12, FL_GPIO_EXTI_LINE_12_PD1 },
    {GET_PIN(D, 2 ), FL_GPIO_EXTI_LINE_12, FL_GPIO_EXTI_LINE_12_PD2 },
    {GET_PIN(D, 3 ), FL_GPIO_EXTI_LINE_12, FL_GPIO_EXTI_LINE_12_PD3 },

    {GET_PIN(D, 4 ), FL_GPIO_EXTI_LINE_13, FL_GPIO_EXTI_LINE_13_PD4 },
    {GET_PIN(D, 5 ), FL_GPIO_EXTI_LINE_13, FL_GPIO_EXTI_LINE_13_PD5 },
    {GET_PIN(D, 6 ), FL_GPIO_EXTI_LINE_13, FL_GPIO_EXTI_LINE_13_PD6 },
    {GET_PIN(D, 7 ), FL_GPIO_EXTI_LINE_13, FL_GPIO_EXTI_LINE_13_PD7 },

    {GET_PIN(D, 8 ), FL_GPIO_EXTI_LINE_14, FL_GPIO_EXTI_LINE_14_PD8 },
    {GET_PIN(D, 9 ), FL_GPIO_EXTI_LINE_14, FL_GPIO_EXTI_LINE_14_PD9 },
    {GET_PIN(D, 10), FL_GPIO_EXTI_LINE_14, FL_GPIO_EXTI_LINE_14_PD10},
    {GET_PIN(D, 11), FL_GPIO_EXTI_LINE_14, FL_GPIO_EXTI_LINE_14_PD11},
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
};

static void HAL_GPIO_WritePin(GPIO_Type* GPIOx, uint32_t GPIO_Pin, os_uint8_t val)
{
    if(val)
    {
        FL_GPIO_SetOutputPin(GPIOx, GPIO_Pin);
    }
    else
    {
        FL_GPIO_ResetOutputPin(GPIOx, GPIO_Pin);  
    }
}
static os_uint8_t HAL_GPIO_ReadPin(GPIO_Type* GPIOx, uint32_t GPIO_Pin)
{
    if(FL_GPIO_GetInputPin(GPIOx, GPIO_Pin))
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
GPIO_Type *get_pin_base(os_base_t pin)
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

static void fm_pin_mode(struct os_device *dev, os_base_t pin, os_base_t mode)
{
    struct pin_index *index;
    FL_GPIO_InitTypeDef  GPIO_InitStruct;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return;
    }
    
    FL_GPIO_DeInit(index->gpio_port, PIN_OFFSET(pin));
    
    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.pin          = PIN_OFFSET(pin);
    GPIO_InitStruct.mode         = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull         = FL_DISABLE;
    GPIO_InitStruct.remapPin     = FL_DISABLE;
    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        GPIO_InitStruct.mode         = FL_GPIO_MODE_OUTPUT;
        GPIO_InitStruct.outputType   = FL_GPIO_OUTPUT_PUSHPULL;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        GPIO_InitStruct.mode        = FL_GPIO_MODE_INPUT;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        GPIO_InitStruct.mode         = FL_GPIO_MODE_INPUT;
        GPIO_InitStruct.pull         = FL_ENABLE;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        os_kprintf(DBG_EXT_TAG, "pull down not surrported.");
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        GPIO_InitStruct.mode         = FL_GPIO_MODE_OUTPUT;
        GPIO_InitStruct.outputType   = FL_GPIO_OUTPUT_OPENDRAIN;
    }

    FL_GPIO_Init(index->gpio_port, &GPIO_InitStruct);
    /* remeber the pull state. */

    index->pin_pulls[__PIN_INDEX(pin)].pull_state = FL_GPIO_IsEnabledPinPullup(index->gpio_port, PIN_OFFSET(pin));
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
    os_uint32_t i;

    for(i = 0; i < ITEM_NUM(pin_irq_map); i++)
    {
        if(pinbit == pin_irq_map[i].pinbit)
        {
            return &pin_irq_map[i];
        }
    }

    return OS_NULL;
};

OS_INLINE const struct pin_irq_map *get_line_irq_map(os_uint32_t line)
{
    os_uint32_t i;

    for(i = 0; i < ITEM_NUM(pin_irq_map); i++)
    {
        if(line == pin_irq_map[i].exti_line)
        {
            return &pin_irq_map[i];
        }
    }

    return OS_NULL;
};

OS_INLINE os_int32_t get_pin_irq_hdr_index(os_uint32_t pinbit)
{
    os_int32_t           i;
    const struct pin_irq_map * irq_map;

    irq_map = get_pin_irq_map(pinbit);

    if(irq_map == OS_NULL)
    {
        return OS_EINVAL;
    }
    
    for(i = 0; i < ITEM_NUM(pin_irq_hdr_tab); i++)
    {
        if(irq_map->pinbit == pin_irq_hdr_tab[i].pin)
        {
            return i;
        }
    }

    return OS_EINVAL;
}


static exti_fun get_exti_set_fun(os_uint32_t exti_line)
{
    exti_fun func = OS_NULL;
    
    switch(exti_line)
        {
            case FL_GPIO_EXTI_LINE_0:
                func = FL_GPIO_SetExtiLine0;
                break;
            case FL_GPIO_EXTI_LINE_1:
                func = FL_GPIO_SetExtiLine1;
                break;
            case FL_GPIO_EXTI_LINE_2:
                func = FL_GPIO_SetExtiLine2;
                break;
            case FL_GPIO_EXTI_LINE_3:
                func = FL_GPIO_SetExtiLine3;
                break;
            case FL_GPIO_EXTI_LINE_4:
                func = FL_GPIO_SetExtiLine5;
                break;
            case FL_GPIO_EXTI_LINE_5:
                func = FL_GPIO_SetExtiLine5;
                break;
            case FL_GPIO_EXTI_LINE_6:
                func = FL_GPIO_SetExtiLine6;
                break;
            case FL_GPIO_EXTI_LINE_7:
                func = FL_GPIO_SetExtiLine7;
                break;
            case FL_GPIO_EXTI_LINE_8:
                func = FL_GPIO_SetExtiLine8;
                break;
            case FL_GPIO_EXTI_LINE_9:
                func = FL_GPIO_SetExtiLine9;
                break;
            case FL_GPIO_EXTI_LINE_10:
                func = FL_GPIO_SetExtiLine10;
                break;
             case FL_GPIO_EXTI_LINE_11:
                func = FL_GPIO_SetExtiLine11;
                break;
             case FL_GPIO_EXTI_LINE_12:
                func = FL_GPIO_SetExtiLine12;
                break;
             case FL_GPIO_EXTI_LINE_13:
                func = FL_GPIO_SetExtiLine13;
                break;
             case FL_GPIO_EXTI_LINE_14:
                func = FL_GPIO_SetExtiLine14;
                break;
             case FL_GPIO_EXTI_LINE_15:
                break;
             default:
                break;           
                
        }
    return func;
}
static os_err_t fm_pin_attach_irq(struct os_device *device, os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index    *index;
    const struct pin_irq_map * irq_map;
    os_int32_t                 irqindex = -1;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    irq_map = get_pin_irq_map(pin);
    if(irq_map == OS_NULL)
    {
       return OS_ENOSYS; 
    }
    
    irqindex = bit2bitno(irq_map->exti_line);
    if(irqindex == -1)

    {
        return OS_ENOSYS;
    }
    
    if (pin_irq_hdr_tab[irqindex].pin  == pin  &&
        pin_irq_hdr_tab[irqindex].hdr  == hdr  &&
        pin_irq_hdr_tab[irqindex].mode == mode &&
        pin_irq_hdr_tab[irqindex].args == args)
    {
        return OS_EOK;
    }
    if (pin_irq_hdr_tab[irqindex].pin != -1)
    {
        return OS_EBUSY;
    }
    pin_irq_hdr_tab[irqindex].pin  = pin;
    pin_irq_hdr_tab[irqindex].hdr  = hdr;
    pin_irq_hdr_tab[irqindex].mode = mode;
    pin_irq_hdr_tab[irqindex].args = args;

    return OS_EOK;
}

static os_err_t fm_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    const struct pin_index    *index;
    const struct pin_irq_map * irq_map;
    os_int32_t                 irqindex = -1;

    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    irq_map = get_pin_irq_map(pin);
    if(irq_map == OS_NULL)
    {
       return OS_ENOSYS; 
    }

    irqindex = bit2bitno(irq_map->exti_line);
    if(irqindex == -1)

    {
        return OS_ENOSYS;      
    }

    if (pin_irq_hdr_tab[irqindex].pin == -1)
    {
        return OS_EOK;
    }
    pin_irq_hdr_tab[irqindex].pin  = -1;
    pin_irq_hdr_tab[irqindex].hdr  = OS_NULL;
    pin_irq_hdr_tab[irqindex].mode = 0;
    pin_irq_hdr_tab[irqindex].args = OS_NULL;

    return OS_EOK;
}

static os_err_t fm_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{

    struct pin_index         *index;
    const struct pin_irq_map *irqmap;
    os_int32_t                irqindex = -1;
    exti_fun                  func;
    FL_GPIO_InitTypeDef       GPIO_InitStruct;
    
    index = get_pin_index(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        irqindex = get_pin_irq_hdr_index(pin);

        if(irqindex == OS_EINVAL)
        {
            return OS_ENOSYS;
        }
        
        index->handler_mask = OS_TRUE;

        if (pin_irq_hdr_tab[irqindex].pin == -1)
        {
            return OS_ENOSYS;
        }

        irqmap = get_pin_irq_map(pin);
        
        FL_RCC_EnableEXTIOnSleep();
        FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI);
        
        /* Configure GPIO_InitStructure */                
        GPIO_InitStruct.pin         = PIN_OFFSET(pin);
        GPIO_InitStruct.mode        = FL_GPIO_MODE_INPUT;
        GPIO_InitStruct.outputType  = FL_GPIO_OUTPUT_PUSHPULL;
        
        if(index->pin_pulls[__PIN_INDEX(pin)].pull_state)
            GPIO_InitStruct.pull    = FL_ENABLE;
        else
            GPIO_InitStruct.pull    = FL_DISABLE;   
        
        GPIO_InitStruct.remapPin    = FL_DISABLE;
        
        FL_GPIO_Init(index->gpio_port, &GPIO_InitStruct);
                
        FL_GPIO_SetTriggerEdge(GPIO,irqmap->exti_line,FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE);

        func = get_exti_set_fun(irqmap->exti_line);
        if(func != OS_NULL)
        {
            func(GPIO,irqmap->exti_pin);
        }
        FL_GPIO_EnableDigitalFilter(GPIO, irqmap->exti_line);
        

        switch (pin_irq_hdr_tab[irqindex].mode)
        {
        case PIN_IRQ_MODE_RISING:
            FL_GPIO_SetTriggerEdge(GPIO,irqmap->exti_line,FL_GPIO_EXTI_TRIGGER_EDGE_RISING);
            break;
        case PIN_IRQ_MODE_FALLING:
           FL_GPIO_SetTriggerEdge(GPIO,irqmap->exti_line,FL_GPIO_EXTI_TRIGGER_EDGE_FALLING);
            break;
        case PIN_IRQ_MODE_RISING_FALLING:
             FL_GPIO_SetTriggerEdge(GPIO,irqmap->exti_line,FL_GPIO_EXTI_TRIGGER_EDGE_BOTH);
            break;
        case PIN_IRQ_MODE_HIGH_LEVEL:
        case PIN_IRQ_MODE_LOW_LEVEL:
        default:
            os_kprintf(DBG_EXT_TAG, "level trig not supported.\r\n");
            return OS_ERROR;
        }
        FL_GPIO_ClearFlag_EXTI(GPIO,irqmap->exti_line);
        pin_irq_enable_mask |= irqmap->pinbit;
        NVIC_EnableIRQ(GPIO_IRQn);
    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        irqindex = get_pin_irq_hdr_index(pin);

        if(irqindex == OS_EINVAL)
        {
            return OS_ENOSYS;
        }
        
        irqmap = get_pin_irq_map(pin);
        if (irqmap == OS_NULL)
        {
            return OS_ENOSYS;
        }
      
        FL_GPIO_SetTriggerEdge(GPIO,irqmap->exti_line,FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE);
        FL_GPIO_ClearFlag_EXTI(GPIO,irqmap->exti_line);
        pin_irq_enable_mask &= ~irqmap->pinbit;        
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

OS_INLINE void pin_irq_hdr(int exti_line)
{

    struct pin_index         *index = OS_NULL;
    const struct pin_irq_map *irqmap;
    struct pin_irq_map       *irqmap_ptr;
    os_int32_t                irq_index;
    os_uint8_t                i;
    
    irqmap    = get_line_irq_map(exti_line);
    
    /*Traverse the four pins corresponding to each exti-line to determine 
    which pin generated the interrupt*/
    irqmap_ptr = (struct pin_irq_map *)irqmap;
    for (i = 0; i < 4; i++)
    {
        irq_index = get_pin_irq_hdr_index(irqmap_ptr->pinbit);
        if(irq_index != OS_EINVAL)
        {
            break;
        }
        irqmap_ptr++;
    }
    
    index     = get_pin_index(irqmap_ptr->pinbit);

    if(index->handler_mask == OS_TRUE)
    {
        index->handler_mask = OS_FALSE;
#if 0
        return;
#endif
    }

    if (pin_irq_hdr_tab[irq_index].hdr)
    {
        pin_irq_hdr_tab[irq_index].hdr(pin_irq_hdr_tab[irq_index].args);
    }

}



void GPIO_IRQHandler(void)
{
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_0))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_0);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_0);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_1))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_1);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_1);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_2))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_2);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_2);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_3))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_3);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_3);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_4))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_4);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_4);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_5))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_5);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_5);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_6))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_6);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_6);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_7))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_7);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_7);
    }

    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_8))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_8);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_8);
    }
    
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_9))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_9);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_9);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_10))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_10);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_10);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_11))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_11);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_11);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_12))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_12);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_12);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_13))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_13);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_13);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_14))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_14);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_14);
    }
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_15))
    {
        pin_irq_hdr(FL_GPIO_EXTI_LINE_15);
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_15);
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
                os_kprintf(DRV_EXT_TAG,"os_malloc error!!!");
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

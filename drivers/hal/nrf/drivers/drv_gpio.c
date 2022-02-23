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

#ifdef OS_USING_PIN
static const struct pin_index pins[] = 
{
    __NRF5X_PIN(0 ,  0, 0 ),
    __NRF5X_PIN(1 ,  0, 1 ),
    __NRF5X_PIN(2 ,  0, 2 ),
    __NRF5X_PIN(3 ,  0, 3 ),
    __NRF5X_PIN(4 ,  0, 4 ),
    __NRF5X_PIN(5 ,  0, 5 ),
    __NRF5X_PIN(6 ,  0, 6 ),
    __NRF5X_PIN(7 ,  0, 7 ),
    __NRF5X_PIN(8 ,  0, 8 ),
    __NRF5X_PIN(9 ,  0, 9 ),
    __NRF5X_PIN(10,  0, 10),
    __NRF5X_PIN(11,  0, 11),
    __NRF5X_PIN(12,  0, 12),
    __NRF5X_PIN(13,  0, 13),
    __NRF5X_PIN(14,  0, 14),
    __NRF5X_PIN(15,  0, 15),
    __NRF5X_PIN(16,  0, 16),
    __NRF5X_PIN(17,  0, 17),
    __NRF5X_PIN(18,  0, 18),
    __NRF5X_PIN(19,  0, 19),
    __NRF5X_PIN(20,  0, 20),
    __NRF5X_PIN(21,  0, 21),
    __NRF5X_PIN(22,  0, 22),
    __NRF5X_PIN(23,  0, 23),
    __NRF5X_PIN(24,  0, 24),
    __NRF5X_PIN(25,  0, 25),
    __NRF5X_PIN(26,  0, 26),
    __NRF5X_PIN(27,  0, 27),
    __NRF5X_PIN(28,  0, 28),
    __NRF5X_PIN(29,  0, 29),
    __NRF5X_PIN(30,  0, 30),
    __NRF5X_PIN(31,  0, 31),
#ifdef SOC_NRF52840    
    __NRF5X_PIN(32,  1, 0 ),
    __NRF5X_PIN(33,  1, 1 ),
    __NRF5X_PIN(34,  1, 2 ),
    __NRF5X_PIN(35,  1, 3 ),
    __NRF5X_PIN(36,  1, 4 ),
    __NRF5X_PIN(37,  1, 5 ),
    __NRF5X_PIN(38,  1, 6 ),
    __NRF5X_PIN(39,  1, 7 ),
    __NRF5X_PIN(40,  1, 8 ),
    __NRF5X_PIN(41,  1, 9 ),
    __NRF5X_PIN(42,  1, 10),
    __NRF5X_PIN(43,  1, 11),
    __NRF5X_PIN(44,  1, 12),
    __NRF5X_PIN(45,  1, 13),
    __NRF5X_PIN(46,  1, 14),
    __NRF5X_PIN(47,  1, 15),
#endif /* SOC_NRF52840 */
};

/* EVENTS_IN[n](n=0..7) and EVENTS_PORT */
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
};

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

/* pin: the number of pins */
const struct pin_index *get_pin(uint8_t pin)
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

static void nrf5_pin_mode(os_device_t *dev, os_base_t pin, os_base_t mode)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        nrf_gpio_cfg_output(pin);
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLDOWN);
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        nrf_gpio_cfg(
        pin,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_S0D1,
        NRF_GPIO_PIN_NOSENSE);
    }

}

static void nrf5_pin_write(os_device_t *dev, os_base_t pin, os_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return;
    }
    
    nrf_gpio_pin_write(pin, value);
}

static int nrf5_pin_read(os_device_t *dev, os_base_t pin)
{
    int value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return value;
    }

    value = nrf_gpio_pin_read(pin);

    return value;
}

extern void set_gpio_int_event_flag(void);

static void pin_irq_hdr(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    int i;
    int irq_quantity;

    #ifdef OS_USING_LPMGR
    set_gpio_int_event_flag();
    #endif

    irq_quantity = ITEM_NUM(pin_irq_hdr_tab);
    for(i = 0; i < irq_quantity; i++)
    {
        if(pin_irq_hdr_tab[i].pin == pin)
        {
            pin_irq_hdr_tab[i].hdr(pin_irq_hdr_tab[i].args);
        }
    }
}

static os_err_t nrf5_pin_attach_irq(struct os_device   *device,
                                    os_int32_t          pin,
                                    os_uint32_t         mode,
                                    void (*hdr)(void *args), void *args)
{
      const struct pin_index *index;
      os_int32_t irqindex = -1;
      os_base_t level;
      nrfx_err_t err_code;
      int i;
      int irq_quantity;
      
      index = get_pin(pin);
      if (index == OS_NULL)
      {
          return OS_ENOSYS;
      }
      
      irq_quantity = ITEM_NUM(pin_irq_hdr_tab);
      for(i = 0; i < irq_quantity; i++)
      {
          if(pin_irq_hdr_tab[i].pin != -1)
          {
              irqindex = -1;
              continue;
          }
          else
          {
              irqindex = i;
              break;
          }
      }
      if(irqindex == -1)
      {
          return OS_ENOMEM;
      }
      
      level = os_irq_lock();  
      pin_irq_hdr_tab[irqindex].pin  = pin;
      pin_irq_hdr_tab[irqindex].hdr  = hdr;
      pin_irq_hdr_tab[irqindex].mode = mode;
      pin_irq_hdr_tab[irqindex].args = args;
      
    if(mode == PIN_IRQ_MODE_RISING)
      {
          nrfx_gpiote_in_config_t inConfig = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
          inConfig.pull = NRF_GPIO_PIN_PULLDOWN;  
          err_code = nrfx_gpiote_in_init(pin, &inConfig, pin_irq_hdr);
      }
          
      else if(mode == PIN_IRQ_MODE_FALLING)
      {
          nrfx_gpiote_in_config_t inConfig = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
          inConfig.pull = NRF_GPIO_PIN_PULLUP;  
          err_code = nrfx_gpiote_in_init(pin, &inConfig, pin_irq_hdr);
      }
      
      else if(mode == PIN_IRQ_MODE_RISING_FALLING)
      {
          nrfx_gpiote_in_config_t inConfig = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
          inConfig.pull = NRF_GPIO_PIN_PULLUP;
          err_code = nrfx_gpiote_in_init(pin, &inConfig, pin_irq_hdr);
      }
      
      os_irq_unlock(level);
      
      switch(err_code) 
      {
          case NRFX_ERROR_BUSY:
              return OS_EBUSY;
          case NRFX_SUCCESS:
              return OS_EOK;
          case NRFX_ERROR_NO_MEM:
              return OS_ENOMEM;
          default:
              return OS_ERROR;
      }

}

static os_err_t nrf5_pin_dettach_irq(struct os_device *device, os_int32_t pin)
{
    const struct pin_index *index;
    os_base_t level;
    int i;
    int irq_quantity;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }
        
    irq_quantity = ITEM_NUM(pin_irq_hdr_tab);
    for(i = 0; i < irq_quantity; i++)
    {
        if(pin_irq_hdr_tab[i].pin == pin)
        {
            level = os_irq_lock();
            pin_irq_hdr_tab[i].pin  = -1;
            pin_irq_hdr_tab[i].hdr  = OS_NULL;
            pin_irq_hdr_tab[i].mode = 0;
            pin_irq_hdr_tab[i].args = OS_NULL;
            nrfx_gpiote_in_uninit(pin);
            os_irq_unlock(level);
            break;
        }
    }
    if(i >= irq_quantity)
    {
        return OS_ENOSYS;
    }
    return OS_EOK;

}

static os_err_t nrf5_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    const struct pin_index *index;  
    os_base_t level;
    int i;
    int irq_quantity;

    index = get_pin(pin);
    if (index == OS_NULL)
    {
        return OS_ENOSYS;
    }

    irq_quantity = ITEM_NUM(pin_irq_hdr_tab);
    for(i = 0; i < irq_quantity; i++)
    {
        if(pin_irq_hdr_tab[i].pin == pin)
        {
            level = os_irq_lock();
            if(enabled == PIN_IRQ_ENABLE)
            {
                nrfx_gpiote_in_event_enable(pin,enabled);
            }
            else if(enabled == PIN_IRQ_DISABLE)
            {
                nrfx_gpiote_in_event_disable(pin);
            }
            os_irq_unlock(level);
            break;
        }
    }
    
    if(i >= irq_quantity)
    {
        return OS_ENOSYS;
    }
    return OS_EOK;

}

const static struct os_pin_ops _nrf5_pin_ops = {
    nrf5_pin_mode,
    nrf5_pin_write,
    nrf5_pin_read,
    nrf5_pin_attach_irq,
    nrf5_pin_dettach_irq,
    nrf5_pin_irq_enable,
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
   nrfx_err_t err_code;
   err_code = os_device_pin_register(0, &_nrf5_pin_ops, OS_NULL);
   err_code = nrfx_gpiote_init();
    
    switch(err_code) 
    {
        case NRFX_ERROR_INVALID_STATE:
            return OS_EINVAL;
        case NRFX_SUCCESS:
            return OS_EOK;
        default:
            return OS_ERROR;;
    }
}
#endif

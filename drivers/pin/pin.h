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
 * @file        pin.h
 *
 * @brief       this file implements pin related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef PIN_H__
#define PIN_H__

#include <os_task.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct os_device_pin
{
    struct os_device         parent;
    const struct os_pin_ops *ops;
    
    int base;
}os_device_pin_t;

#define PIN_LOW  0x00
#define PIN_HIGH 0x01

#define PIN_MODE_OUTPUT         0x00
#define PIN_MODE_INPUT          0x01
#define PIN_MODE_INPUT_PULLUP   0x02
#define PIN_MODE_INPUT_PULLDOWN 0x03
#define PIN_MODE_OUTPUT_OD      0x04
#define PIN_MODE_DISABLE        0x08

#define PIN_IRQ_MODE_RISING         0x00
#define PIN_IRQ_MODE_FALLING        0x01
#define PIN_IRQ_MODE_RISING_FALLING 0x02
#define PIN_IRQ_MODE_HIGH_LEVEL     0x03
#define PIN_IRQ_MODE_LOW_LEVEL      0x04

#define PIN_IRQ_DISABLE 0x00
#define PIN_IRQ_ENABLE  0x01

#define PIN_IRQ_PIN_NONE -1

struct os_device_pin_mode
{
    os_uint16_t pin;
    os_uint16_t mode;
};
struct os_device_pin_status
{
    os_uint16_t pin;
    os_uint16_t status;
};
struct os_pin_irq_hdr
{
    os_int16_t  pin;
    os_uint16_t mode;
    void (*hdr)(void *args);
    void *args;
};
struct os_pin_ops
{
    void (*pin_mode)(struct os_device *device, os_base_t pin, os_base_t mode);
    void (*pin_write)(struct os_device *device, os_base_t pin, os_base_t value);
    int (*pin_read)(struct os_device *device, os_base_t pin);

    /* TODO: add GPIO interrupt */
    os_err_t (*pin_attach_irq)(struct os_device *device,
                               os_int32_t        pin,
                               os_uint32_t       mode,
                               void (*hdr)(void *args),
                               void *args);
    os_err_t (*pin_detach_irq)(struct os_device *device, os_int32_t pin);
    os_err_t (*pin_irq_enable)(struct os_device *device, os_base_t pin, os_uint32_t enabled);
};

int os_device_pin_register(int pin_base, const struct os_pin_ops *ops, void *user_data);

void     os_pin_mode(os_base_t pin, os_base_t mode);
void     os_pin_write(os_base_t pin, os_base_t value);
int      os_pin_read(os_base_t pin);
os_err_t os_pin_attach_irq(os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args);
os_err_t os_pin_detach_irq(os_int32_t pin);
os_err_t os_pin_irq_enable(os_base_t pin, os_uint32_t enabled);

#ifdef __cplusplus
}
#endif

#endif

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
 * @file        pin.c
 *
 * @brief       this file implements pin related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdlib.h>
#include <string.h>
#include <os_assert.h>
#include <os_errno.h>
#include <pin/pin.h>

#ifdef OS_USING_SHELL
#include <drv_log.h>
#include <shell.h>
#endif

#ifndef OS_PIN_MAX_CHIP
#define OS_PIN_MAX_CHIP (1)
#endif

#define DBG_TAG "pin"

static struct os_device_pin gs_pin_table[OS_PIN_MAX_CHIP];

static os_size_t _pin_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_device_pin_status *status;

    struct os_device_pin *pin = (struct os_device_pin *)dev;

    /* check parameters */
    OS_ASSERT(pin != OS_NULL);

    status = (struct os_device_pin_status *)buffer;
    if (status == OS_NULL || size != sizeof(*status))
        return 0;

    status->status = pin->ops->pin_read(dev, status->pin - pin->base);
    return size;
}

static os_size_t _pin_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_device_pin_status *status;

    struct os_device_pin *pin = (struct os_device_pin *)dev;

    /* check parameters */
    OS_ASSERT(pin != OS_NULL);

    status = (struct os_device_pin_status *)buffer;
    if (status == OS_NULL || size != sizeof(*status))
        return 0;

    pin->ops->pin_write(dev, (os_base_t)status->pin - pin->base, (os_base_t)status->status);

    return size;
}

static os_err_t _pin_control(os_device_t *dev, int cmd, void *args)
{
    struct os_device_pin_mode *mode;

    struct os_device_pin *pin = (struct os_device_pin *)dev;

    /* check parameters */
    OS_ASSERT(pin != OS_NULL);

    mode = (struct os_device_pin_mode *)args;
    if (mode == OS_NULL)
        return OS_ERROR;

    pin->ops->pin_mode(dev, (os_base_t)mode->pin - pin->base, (os_base_t)mode->mode);

    return 0;
}

const static struct os_device_ops pin_ops = {
    .read    = _pin_read,
    .write   = _pin_write,
    .control = _pin_control,
};

/**
 ***********************************************************************************************************************
 * @brief           register pin device
 *
 * @param[in]       name            pointer of pin name
 * @param[in]       ops             pointer of pin operation function set
 * @param[in]       user_data       not used
 *
 * @return          os_err_t
 * @retval          0               run successfully
 ***********************************************************************************************************************
 */
int os_device_pin_register(int pin_base, const struct os_pin_ops *ops, void *user_data)
{
    int  pin_index;
    char name[12];

    if ((pin_base % 1000) != 0)
    {
        os_kprintf("pin base invalid[%d], must be 0, 1000, 2000, ...\r\n", pin_base);
        return -1;
    }

    pin_index = pin_base / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("%d outof chip number range:(%d) >= (%d)\r\n", pin_base, pin_index, OS_PIN_MAX_CHIP);
        return -1;
    }

    struct os_device_pin *pin = &gs_pin_table[pin_index];

    if (pin->parent.type == OS_DEVICE_TYPE_MISCELLANEOUS)
    {
        os_kprintf("pin %d exist.\r\n", pin_base);
        return -1;
    }

    pin->parent.type        = OS_DEVICE_TYPE_MISCELLANEOUS;
    pin->parent.ops = &pin_ops;
    
    pin->ops  = ops;
    pin->base = pin_base;

    pin->parent.user_data = user_data;

    os_snprintf(name, sizeof(name) - 1, "pin_%d", pin_base);
    os_device_register(&pin->parent, name);

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           attach irq of pin
 *
 * @param[in]       pin            	pin number
 * @param[in]       mode           	irq mode
 * @param[in]       hdr            	callback function for pin irq
 * @param[in]       args            pin number
 *
 * @return          os_err_t
 * @retval          OS_EOK          attch successfully
 * @retval          OS_ENOSYS       attch failed: no valid operation function set or pin
 * @retval          OS_EBUSY        pin has been used
 ***********************************************************************************************************************
 */
os_err_t os_pin_attach_irq(os_int32_t pin, os_uint32_t mode, void (*hdr)(void *args), void *args)
{
    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return OS_EINVAL;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    if (ppin->ops->pin_attach_irq)
    {
        return ppin->ops->pin_attach_irq((struct os_device *)ppin, pin - ppin->base, mode, hdr, args);
    }
    return OS_ENOSYS;
}

/**
 ***********************************************************************************************************************
 * @brief           detach irq of pin
 *
 * @param[in]       pin            	pin number
 *
 * @return          os_err_t
 * @retval          OS_EOK          detach successfully
 * @retval          OS_ENOSYS       detach failed: no valid operation function set or pin
 ***********************************************************************************************************************
 */
os_err_t os_pin_detach_irq(os_int32_t pin)
{
    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return OS_EINVAL;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    if (ppin->ops->pin_detach_irq)
    {
        return ppin->ops->pin_detach_irq((struct os_device *)ppin, pin - ppin->base);
    }
    return OS_ENOSYS;
}

/**
 ***********************************************************************************************************************
 * @brief           enable irq of pin
 *
 * @param[in]       pin            	pin number
 * @param[in]       enabled         pin enable command
 *
 * @return          os_err_t
 * @retval          OS_EOK          enable successfully
 * @retval          OS_ENOSYS       enable failed: no valid operation function set or pin
 ***********************************************************************************************************************
 */
os_err_t os_pin_irq_enable(os_base_t pin, os_uint32_t enabled)
{
    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return OS_EINVAL;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    if (ppin->ops->pin_irq_enable)
    {
        return ppin->ops->pin_irq_enable((struct os_device *)ppin, pin - ppin->base, enabled);
    }
    return OS_ENOSYS;
}

/**
 ***********************************************************************************************************************
 * @brief           set mode of pin
 *
 * @param[in]       pin            	pin number
 * @param[in]       mode            pin mode
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_pin_mode(os_base_t pin, os_base_t mode)
{
    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    ppin->ops->pin_mode((struct os_device *)ppin, pin - ppin->base, mode);
}

/**
 ***********************************************************************************************************************
 * @brief           write status of pin
 *
 * @param[in]       pin            	pin number
 * @param[in]       value           pin status
 *
 * @return          no return value
 ***********************************************************************************************************************
 */
void os_pin_write(os_base_t pin, os_base_t value)
{
    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    ppin->ops->pin_write((struct os_device *)ppin, pin - ppin->base, value);
}

/**
 ***********************************************************************************************************************
 * @brief           read status of pin
 *
 * @param[in]       pin            	pin number
 *
 * @return          int
 * @retval          value           pin status:high or low
 ***********************************************************************************************************************
 */
int os_pin_read(os_base_t pin)
{
    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return OS_EINVAL;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    return ppin->ops->pin_read((struct os_device *)ppin, pin - ppin->base);
}

#ifdef OS_USING_SHELL
static os_err_t sh_pin_mode(os_int32_t argc, char **argv)
{
    os_base_t pin;
    os_base_t mode;
    
    if (argc != 3)
    {
        //LOG_E(DBG_TAG, "parameter error, use pinMode pin_num 0/1");
        return OS_ERROR;
    }
    pin  = atoi(argv[1]);
    mode = atoi(argv[2]);

    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return OS_EINVAL;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    ppin->ops->pin_mode((struct os_device *)ppin, pin - ppin->base, mode);

    return OS_EOK;
}
SH_CMD_EXPORT(pinMode, sh_pin_mode, "set hardware pin mode");

static os_err_t sh_pin_write(os_int32_t argc, char **argv)
{
    os_base_t pin;
    os_base_t value;
    
    if (argc != 3)
    {
        //LOG_E(DBG_TAG, "parameter error, use pinWrite pin_num 0/1");
        return OS_ERROR;
    }
    pin   = atoi(argv[1]);
    value = atoi(argv[2]);

    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return OS_EINVAL;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    ppin->ops->pin_write((struct os_device *)ppin, pin - ppin->base, value);

    return OS_EOK;
}
SH_CMD_EXPORT(pinWrite, sh_pin_write, "write value to hardware pin");

static os_err_t sh_pin_read(os_int32_t argc, char **argv)
{
    os_base_t pin;
    
    if (argc != 2)
    {
        //LOG_E(DBG_TAG, "parameter error, use pinRead pin_num");
        return OS_ERROR;
    }
    pin = atoi(argv[1]);

    int pin_index = pin / 1000;

    if (pin_index >= OS_PIN_MAX_CHIP)
    {
        os_kprintf("invalide pin %d.\r\n", pin);
        return OS_EINVAL;
    }

    struct os_device_pin *ppin = &gs_pin_table[pin_index];
    
    os_kprintf("Pin: %d, Value:%d\r\n", pin, ppin->ops->pin_read((struct os_device *)ppin, pin - ppin->base));

    return OS_EOK;
}
SH_CMD_EXPORT(pinRead, sh_pin_read, "read status from hardware pin");
#endif

#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_util.h>
#include <os_memory.h>
#include <string.h>
#include <stdio.h>
#include <drv_gpio.h>

#define DBG_TAG "pin.pcf8574"

typedef struct pcf8574 {
    struct os_i2c_client client;
    int int_pin;
}pcf8574_t;

pcf8574_t *pcf8574_init(const char *bus_name, os_uint16_t addr, int int_pin)
{
    pcf8574_t *pcf8574;

    LOG_I(DBG_TAG,"pcf8574:[%s][0x%02x][%d]", bus_name, addr, int_pin);

    pcf8574 = os_calloc(1, sizeof(pcf8574_t));
    if (pcf8574 == OS_NULL)
    {
        LOG_E(DBG_TAG,"pcf8574 amlloc faile");
        return NULL;
    }

    pcf8574->client.bus = os_i2c_bus_device_find(bus_name);
    if (pcf8574->client.bus == OS_NULL)
    {
        LOG_E(DBG_TAG,"pcf8574 i2c invalid [%s].", bus_name);
        os_free(pcf8574);
        return OS_NULL;
    }

    pcf8574->client.client_addr = addr;
    pcf8574->int_pin = int_pin;
    
    return pcf8574;
}

int pcf8574_write(struct os_i2c_client *client, int pin, int status)
{
    os_uint8_t data;

    os_i2c_client_read(client, 0, 0, &data, 1);

    if (status)
    {
        data |= 1 << pin;
    }
    else
    {
        data &= ~(1 << pin);
    }
    
    os_i2c_client_write(client, data, 1, OS_NULL, 0);
    return 0;
}

int pcf8574_read(struct os_i2c_client *client, int pin)
{
    os_uint8_t data;

    os_i2c_client_read(client, 0, 0, &data, 1);

    return (data >> pin) & 1;
}

void pcf8574_pin_mode(struct os_device *device, os_base_t pin, os_base_t mode)
{

}

void pcf8574_pin_write(struct os_device *device, os_base_t pin, os_base_t value)
{
    pcf8574_t *pcf8574 = (pcf8574_t *)device->user_data;
    
    pcf8574_write(&pcf8574->client, pin, value);
}

int pcf8574_pin_read(struct os_device *device, os_base_t pin)
{
    pcf8574_t *pcf8574 = (pcf8574_t *)device->user_data;
    
    return pcf8574_read(&pcf8574->client, pin);
}

os_err_t pcf8574_pin_attach_irq(struct os_device *device,
                           os_int32_t        pin,
                           os_uint32_t       mode,
                           void (*hdr)(void *args),
                           void *args)
{
    return OS_ENOSYS;
}

os_err_t pcf8574_pin_detach_irq(struct os_device *device, os_int32_t pin)
{
    return OS_ENOSYS;
}

os_err_t pcf8574_pin_irq_enable(struct os_device *device, os_base_t pin, os_uint32_t enabled)
{
    return OS_ENOSYS;
}

const static struct os_pin_ops pcf8574_pin_ops = {
    .pin_mode  = pcf8574_pin_mode,
    .pin_write = pcf8574_pin_write,
    .pin_read  = pcf8574_pin_read,
    
    .pin_attach_irq = pcf8574_pin_attach_irq,
    .pin_detach_irq = pcf8574_pin_detach_irq,
    .pin_irq_enable = pcf8574_pin_irq_enable,
};

static int os_hw_pcf8574_init(void)
{
    pcf8574_t *pcf8574;

    pcf8574 = pcf8574_init(OS_PCF8574_I2C_BUS_NAME, OS_PCF8574_I2C_BUS_ADDR, OS_PCF8574_INT_PIN);
    if (pcf8574 == OS_NULL)
    {
        LOG_E(DBG_TAG,"pcf8574 init failed.");
        return OS_ERROR;
    }

    return os_device_pin_register(OS_PCF8574_PIN_BASE, &pcf8574_pin_ops, pcf8574);
}

OS_INIT_EXPORT(os_hw_pcf8574_init, "3", OS_INIT_SUBLEVEL_LOW);


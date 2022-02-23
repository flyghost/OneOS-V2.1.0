#include "device.h"
#include <console.h>
#include <drv_cfg.h>
#include <shell.h>
#include <stdlib.h>

os_err_t device_ops_notify_callback(os_device_t *dev, os_ubase_t event, os_ubase_t args)
{
    if(!strcmp(dev->name, OS_CONSOLE_DEVICE_NAME))
    {
        return OS_EOK;
    }
    switch(event)
    {
        case ION_GENERIC_REGISTER:
            os_kprintf("%s register: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_UNREGISTER:
            os_kprintf("%s unregister: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_OPEN:
            os_kprintf("%s open: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_CLOSE:
            os_kprintf("%s close: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_READ_BLOCK:
            os_kprintf("%s read block: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_READ_NONBLOCK:
            os_kprintf("%s read nonblock: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_WRITE_BLOCK:
            os_kprintf("%s write block: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_WRITE_NONBLOCK:
            os_kprintf("%s write nonblock: %d\r\n", dev->name, args);
            break;
        case ION_GENERIC_CONTROL:
            os_kprintf("%s control: %d\r\n", dev->name, args);
            break;
        default:
            break;
    }
    return OS_EOK;
}

os_err_t device_ops_notify_init(void)
{
    os_device_notify_register(OS_NULL, device_ops_notify_callback, OS_NULL);
        
    return OS_EOK;
}

//OS_POSTCORE_INIT(device_ops_notify_init, OS_INIT_SUBLEVEL_HIGH);

int device_notify_test(int argc, char **argv)
{
    os_err_t    ret = OS_EOK;

    os_device_t *dev = OS_NULL;
    
    if (argc != 2)
    {
        os_kprintf("usage: device_notify_test name  \r\n");
        os_kprintf("       device_notify_test adc1  \r\n");
        return -1;
    }

    /* find device */
    dev = os_device_find(argv[1]);
    if (dev == OS_NULL)
    {
        os_kprintf("dev name %s not find! \r\n", argv[1]);
        return -1;
    }
    
    os_kprintf("dev notify register! \r\n");
    os_device_notify_register(dev, device_ops_notify_callback, OS_NULL);
    
    os_device_open(dev);
    os_device_close(dev);
    
    os_kprintf("dev notify unregister! \r\n");
    os_device_notify_unregister(dev, device_ops_notify_callback, OS_NULL);
    
    os_device_open(dev);
    os_device_close(dev);
    
    os_kprintf("all dev notify register! \r\n");
    os_device_notify_register(OS_NULL, device_ops_notify_callback, OS_NULL);
    
    os_device_open(dev);
    os_device_close(dev);
    
    os_kprintf("all dev notify unregister! \r\n");
    os_device_notify_unregister(OS_NULL, device_ops_notify_callback, OS_NULL);
    
    os_device_open(dev);
    os_device_close(dev);
    
    return ret;
}

SH_CMD_EXPORT(device_notify_test, device_notify_test, "test set adc");

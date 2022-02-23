#include "usr_misc.h"
#include "device.h"
#include "usr_general.h"


int mpy_usr_driver_open(const char *dev_name)
{
    int ret = -1;
    os_device_t *device = NULL;
    
    device = os_device_find(dev_name);
    if (NULL == device)
    {
        mp_err("Find device: %s failed.", dev_name);
        return MP_ERROR;
    }

    ret = os_device_open(device);
    if (0 != ret)
    {
        mp_err("Open device: %s failed[%d].", dev_name, ret);
        return MP_ERROR;
    }
    
    return MP_EOK;
}

int mpy_usr_driver_close(const char *dev_name)
{
    int32_t ret = -1;
    os_device_t *device = NULL;
    
    device = os_device_find(dev_name);
    if (NULL == device)
    {
        mp_err("Find device: %s failed.", dev_name);
        return MP_ERROR;
    }

    ret = os_device_close(device);
    if (0 != ret)
    {
        mp_err("Close device: %s failed[%d].", dev_name, ret);
        return MP_ERROR;
    }
    
    return MP_EOK;
}

int mpy_usr_driver_read(const char *dev_name, uint32_t offset, 
                                  void *buf, uint32_t bufsize, uint32_t read_type)
{
    int read_size = -1;
    os_device_t *device = NULL;
    
    device = os_device_find(dev_name);
    if(NULL == dev_name)
    {
        mp_err("Find device: %s failed.");
        return read_size;
    }

    if (read_type == MP_USR_DRIVER_READ_NONBLOCK)
    {
        read_size = os_device_read_nonblock(device, offset, buf, bufsize);
    }
    else if (read_type == MP_USR_DRIVER_READ_BLOCK)
    {
        read_size = os_device_read_block(device, offset, buf, bufsize);
    }
    
    return read_size;
}

int mpy_usr_driver_write(const char *dev_name, uint32_t offset, 
                                        void *buf, uint32_t bufsize, uint32_t write_type)
{
    int32_t write_size = -1;
    os_device_t *device = NULL;
    
    device = os_device_find(dev_name);
    if (NULL == device) 
    {
        mp_err("Find device: %s failed.", dev_name);
        return write_size;
    }

    if (write_type == MP_USR_DRIVER_WRITE_NONBLOCK)
    {
        write_size = os_device_write_nonblock(device, offset, buf, bufsize);
    }
    else if (write_type == MP_USR_DRIVER_WRITE_BLOCK)
    {
        write_size = os_device_write_block(device, offset, buf, bufsize);
    }

    return write_size;
}



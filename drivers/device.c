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
 * @file        os_device.c
 *
 * @brief       This file implements the device functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-27   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <device.h>
#include <os_memory.h>
#include <os_errno.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <os_util.h>
#include <os_assert.h>
#include <string.h>
#include <driver.h>

static os_sem_t dev_sem;

static os_list_node_t os_device_list = OS_LIST_INIT(os_device_list);

#ifdef OS_USING_VFS_DEVFS

#include <fcntl.h>
#include <sys/errno.h>
#include <vfs_devfs.h>

static int dev_fops_open(struct vfs_file *fp)
{
    struct os_device *device;

    device = (struct os_device *)fp->private;
    OS_ASSERT(device != OS_NULL);
    
    return os_device_open(device);
}

static int dev_fops_close(struct vfs_file *fp)
{
    struct os_device *device;

    device = (struct os_device *)fp->private;
    OS_ASSERT(device != OS_NULL);

    return os_device_close(device);
}

static int dev_fops_read(struct vfs_file *fp, off_t pos, void *buf, size_t count)
{
    int size = 0;
    struct os_device *device;

    device = (struct os_device *)fp->private;
    OS_ASSERT(device != OS_NULL);

    if (fp->flags & O_NONBLOCK)
    {
        size = os_device_read_nonblock(device, pos, buf, count);
        if (size <= 0)
        {
            return -EAGAIN;
        }
        else
        {
            return size;
        }
    }
    else
    {
        return os_device_read_block(device, pos, buf, count);
    }
}

static int dev_fops_write(struct vfs_file *fp, off_t pos, const void *buf, size_t count)
{
    int size = 0;
    struct os_device *device;

    device = (struct os_device *)fp->private;
    OS_ASSERT(device != OS_NULL);

    if (fp->flags & O_NONBLOCK)
    {
        size = os_device_write_nonblock(device, pos, buf, count);
        if (size <= 0)
        {
            return -EAGAIN;
        }
        else
        {
            return size;
        }
    }
    else
    {
        return os_device_write_block(device, pos, buf, count);
    }
}

static int dev_fops_ioctl(struct vfs_file *fp, unsigned long cmd, void *args)
{
    struct os_device *device;

    device = (struct os_device *)fp->private;
    OS_ASSERT(device != OS_NULL);

    return os_device_control(device, cmd, args);
}

#if defined(OS_USING_IO_MULTIPLEXING)

static void dev_poll_wakeup(struct os_device *device, int key)
{
    struct vfs_pollfd *fds;
    struct vfs_pollfd *tmp;

    os_list_for_each_entry_safe(fds, tmp, &device->poll_list, struct vfs_pollfd, node)
    {
        if (fds->events & key)
        {
            devfs_poll_notify(fds, key);
        }
    }
}

static int dev_fops_poll_init(struct vfs_file *fp, struct vfs_pollfd *fds)
{
    int mask  = 0;
    int flags = 0;

    struct os_device *device;

    device = (struct os_device *)fp->private;
    OS_ASSERT(device != OS_NULL);

    flags = fp->flags & O_ACCMODE;

    /* POLLIN */
    if ((fds->events & POLLIN) && (flags == O_RDONLY || flags == O_RDWR))
    {
        if (device->rx_count != 0)
        {
            mask |= POLLIN;
        }
    }

    /* POLLOUT */
    if ((fds->events & POLLOUT) && (flags == O_WRONLY || flags == O_RDWR))
    {
        if (device->tx_count != device->tx_size)
        {
            mask |= POLLOUT;
        }
    }

    os_list_add_tail(&device->poll_list, &fds->node);

    if (mask != 0)
    {
        devfs_poll_notify(fds, mask);
    }

    return mask;
}

static int dev_fops_poll_deinit(struct vfs_file *fp, struct vfs_pollfd *fds)
{
    os_list_del(&fds->node);
    return 0;
}

static int dev_fops_poll(struct vfs_file *fp, struct vfs_pollfd *fds, os_bool_t poll_setup)
{
    OS_UNUSED struct os_device *device;

    device = (struct os_device *)fp->private;

    OS_ASSERT(device != OS_NULL);

    if (poll_setup == OS_TRUE)
    {
        return dev_fops_poll_init(fp, fds);
    }
    else
    {
        return dev_fops_poll_deinit(fp, fds);
    }
}

#endif

const static struct dev_file_ops dfops =
{
    .open     = dev_fops_open,
    .close    = dev_fops_close,
    .read     = dev_fops_read,
    .write    = dev_fops_write,
    .ioctl    = dev_fops_ioctl,
#if defined(OS_USING_IO_MULTIPLEXING)
    .poll     = dev_fops_poll,
#endif
};

#endif

#ifdef OS_USING_DEVICE_NOTIFY
void os_device_notify(os_device_t *dev, os_ubase_t event, os_ubase_t args)
{
    struct os_device_notify_cb_info *notify_info;

    os_list_for_each_entry(notify_info, &os_device_notify_list, struct os_device_notify_cb_info, list)
    {
        if (notify_info->callback != OS_NULL)
        {
            if (notify_info->filter != OS_NULL)
            {
                if (notify_info->filter(dev, event, args) != OS_EOK)
                {
                    return;
                }
            }
            
            notify_info->callback(dev, event, args);
        }
    }

    os_list_for_each_entry(notify_info, &dev->notify_list, struct os_device_notify_cb_info, list)
    {
        if (notify_info->callback != OS_NULL)
        {
            if (notify_info->filter != OS_NULL)
            {
                if (notify_info->filter(dev, event, args) != OS_EOK)
                {
                    return;
                }
            }
            
            notify_info->callback(dev, event, args);
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function registers a notify callback.
 *
 * @param[in]       struct os_notify_cb_info *info.
 *
 * @return          Regist result.
 * @retval          OS_EOK          Successful.
 * @retval          OS_EINVAL       Fail.
 ***********************************************************************************************************************
 */
os_err_t os_device_notify_register(os_device_t *dev, device_notify_callback callback, device_notify_filter filter)
{
    os_base_t level;
    struct os_device_notify_cb_info *cb_info;
    struct os_device_notify_cb_info *node;
    struct os_device_notify_cb_info *tmp;
    
    OS_ASSERT(OS_NULL != callback);
    
    cb_info = (struct os_device_notify_cb_info *)os_calloc(1, sizeof(struct os_device_notify_cb_info));
    
    cb_info->callback   = callback;
    cb_info->filter     = filter;
    
    level = os_irq_lock();
    
    if (dev == OS_NULL)
    {
        os_list_for_each_entry_safe(node, tmp, &os_device_notify_list, struct os_device_notify_cb_info, list)
        {
            if ((callback == node->callback) && (filter == node->filter))
            {
                os_irq_unlock(level);
                return OS_EOK;
            }
        }
        
        os_list_add(&os_device_notify_list, &cb_info->list);
    }
    else
    {
        os_list_for_each_entry_safe(node, tmp, &dev->notify_list, struct os_device_notify_cb_info, list)
        {
            if ((callback == node->callback) && (filter == node->filter))
            {
                os_irq_unlock(level);
                return OS_EOK;
            }
        }
        
        os_list_add(&dev->notify_list, &cb_info->list);
    }
    
    os_irq_unlock(level);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will remove a previously registered notify callback.
 *
 * @param[in]       void *arg            para.
 *
 * @return          Regist result.
 * @retval          OS_EOK          Successful.
 * @retval          OS_EINVAL       Fail.
 ***********************************************************************************************************************
 */
os_err_t os_device_notify_unregister(os_device_t *dev, device_notify_callback callback, device_notify_filter filter)
{
    os_base_t level;
    struct os_device_notify_cb_info *node;
    struct os_device_notify_cb_info *tmp;
    
    OS_ASSERT(OS_NULL != callback);
    
    level = os_irq_lock();

    if (dev == OS_NULL)
    {
        os_list_for_each_entry_safe(node, tmp, &os_device_notify_list, struct os_device_notify_cb_info, list)
        {
            if ((callback == node->callback) && (filter == node->filter))
            {
                os_list_del(&node->list);
                os_irq_unlock(level);
                os_free(node);
                return OS_EOK;
            }
        }
    }
    else
    {
        os_list_for_each_entry_safe(node, tmp, &dev->notify_list, struct os_device_notify_cb_info, list)
        {
            if ((callback == node->callback) && (filter == node->filter))
            {
                os_list_del(&node->list);
                os_irq_unlock(level);
                os_free(node);
                return OS_EOK;
            }
        }
    }

    os_irq_unlock(level);

    return OS_ERROR;
}
#endif
/**
 ***********************************************************************************************************************
 * @brief           This function registers a device and places it on the list of device object.
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       name            Pointer to device name string.
 * @param[in]       flag            Flags of device.
 *
 * @return          Regist result.
 * @retval          OS_EOK          Successful.
 * @retval          OS_EINVAL       Fail.
 ***********************************************************************************************************************
 */
os_err_t os_device_register(os_device_t *dev, const char *name)
{
    int i;
    os_base_t level;

    if (OS_NULL == dev)
    {
        return OS_EINVAL;
    }

    if ((OS_NULL == name) || (OS_NULL != os_device_find(name)))
    {
        return OS_EINVAL;
    }

    memset(device_name(dev), 0, sizeof(device_name(dev)));
    strncpy(device_name(dev), name, OS_NAME_MAX);

    dev->ref_count = 0;
    dev->tx_timeout   = OS_TICK_MAX;
    dev->rx_timeout   = OS_TICK_MAX;

    os_sem_init(&dev->sem, name, 1, 1);
    os_sem_init(&dev->tx_sem, name, 1, 1);
    os_sem_init(&dev->rx_sem, name, 0, 1);

    for (i = 0; i < OS_DEVICE_CB_TYPE_NUM; i++)
        os_list_init(&dev->cb_heads[i]);

    level = os_irq_lock();
    os_list_add(&os_device_list, &dev->list);
    os_irq_unlock(level);

#ifdef OS_USING_VFS_DEVFS
#ifdef OS_USING_IO_MULTIPLEXING
    os_list_init(&dev->poll_list);
#endif
    devfs_register_device(name, (struct dev_file_ops *)&dfops, dev);
#endif

#ifdef OS_USING_DEVICE_NOTIFY
    os_list_init(&dev->notify_list);
#endif

    os_device_notify(dev, ION_GENERIC_REGISTER, 0);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will remove a previously registered device.
 *
 * @param[in]       dev             The descriptor of device control block.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t os_device_unregister(os_device_t *dev)
{
    os_base_t level;
    
    OS_ASSERT(OS_NULL != dev);

    os_device_notify(dev, ION_GENERIC_UNREGISTER, 0);

#ifdef OS_USING_VFS_DEVFS
    devfs_unregister_device(device_name(dev));
#endif

    os_sem_deinit(&dev->sem);
    os_sem_deinit(&dev->tx_sem);
    os_sem_deinit(&dev->rx_sem);
    
    
    level = os_irq_lock();
    os_list_del(&dev->list);
    os_irq_unlock(level);
    
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Find device by name on the device object list .
 *
 * @details         This function will find device by name on the device object list.
 *
 * @param[in]       name            Pointer to device name string.
 * 
 * @return          On success, return a device control block descriptor; on error, OS_NULL is returned.
 * @retval          not OS_NULL     Return a task control block descriptor.
 * @retval          OS_NULL         No task to be found.
 ***********************************************************************************************************************
 */
os_device_t *os_device_find(const char *name)
{
    os_device_t *dev;

    os_sem_wait(&dev_sem, OS_WAIT_FOREVER);

    os_list_for_each_entry(dev, &os_device_list, os_device_t, list)
    {
        if (0 == strncmp(device_name(dev), name, OS_NAME_MAX))
        {
            os_sem_post(&dev_sem);
            return dev;
        }
    }
    
    os_sem_post(&dev_sem);
    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will open the device through the open function installed on the device.
 *
 * @param[in]       dev             The descriptor of device control block.
 *
 * @return          Open result.
 ***********************************************************************************************************************
 */
os_err_t os_device_open(os_device_t *dev)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(OS_NULL != dev);
    
    os_sem_wait(&dev->sem, OS_WAIT_FOREVER);
    
    if (dev->ref_count == 0)
    {
        if (dev->ops != OS_NULL && dev->ops->init != OS_NULL)
        {
            result = dev->ops->init(dev);
            if (result != OS_EOK)
            {
                os_kprintf("Initialize device:%s failed. The error code is %d",
                            device_name(dev), 
                            result);
                goto end;
            }
        }
    }
    
    dev->ref_count++;

end:    
    os_sem_post(&dev->sem);

    os_device_notify(dev, ION_GENERIC_OPEN, 0);
    
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will close the device through the close function installed on the device.
 *
 * @param[in]       dev             The descriptor of device control block.
 *
 * @return          Close result.
 ***********************************************************************************************************************
 */
os_err_t os_device_close(os_device_t *dev)
{
    int i;
    os_err_t result = OS_EOK;
    struct os_device_cb_info *node;
    struct os_device_cb_info *tmp;

    OS_ASSERT(OS_NULL != dev);
    
    os_sem_wait(&dev->sem, OS_WAIT_FOREVER);

    if (dev->ref_count == 0)
    {
        result = OS_ERROR;
        goto end;
    }

    if (--dev->ref_count > 0)
    {
        goto end;
    }

    os_device_notify(dev, ION_GENERIC_CLOSE, 0);

    if (dev->ops != OS_NULL && dev->ops->deinit != OS_NULL)
    {
        dev->ops->deinit(dev);
    }
    
    for (i = 0; i < OS_DEVICE_CB_TYPE_NUM; i++)
    {
        os_list_for_each_entry_safe(node, tmp, &dev->cb_heads[i], struct os_device_cb_info, list)
        {
            os_list_del(&node->list);
            os_free(node);
        }
    }

end:
    os_sem_post(&dev->sem);
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will read some data from a device. 
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       pos             The position of reading.
 * @param[out]      buffer          The data buffer to save read data.
 * @param[in]       size            The size of buffer.
 * 
 * @return          The actually read size on successful, otherwise negative returned.
 ***********************************************************************************************************************
 */
os_size_t os_device_read_block(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_size_t count;
    
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(dev->ref_count != 0);


    if (dev->ops == OS_NULL || dev->ops->read == OS_NULL)
        return 0;

    if (size == 0)
        return 0;

    /* block */    
    os_sem_wait(&dev->sem, OS_WAIT_FOREVER);
    while (dev->rx_size != 0 && dev->rx_count == 0)
    {
        os_sem_post(&dev->sem);

        if (os_sem_wait(&dev->rx_sem, dev->rx_timeout) != OS_EOK)
            return 0;

        os_sem_wait(&dev->sem, OS_WAIT_FOREVER);
    }

    /* read */
    count = dev->ops->read(dev, pos, buffer, size);
    
    os_device_notify(dev, ION_GENERIC_READ_BLOCK, count);

    /* wake up other thread */
     if (dev->rx_count != 0)
    {
        os_sem_post(&dev->rx_sem);
    }

    os_sem_post(&dev->sem);
    return count;
}

os_size_t os_device_read_nonblock(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_size_t count;
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(dev->ref_count != 0);


    if (dev->ops == OS_NULL || dev->ops->read == OS_NULL)
        return 0;

    if (size == 0)
        return 0;

    if (dev->rx_size != 0 && dev->rx_count == 0)
        return 0;

    /* nonblock */
    count = dev->ops->read(dev, pos, buffer, size);
    
    os_device_notify(dev, ION_GENERIC_READ_NONBLOCK, count);

    return count;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will write some data to a device. 
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       pos             The position of written.
 * @param[out]      buffer          The data buffer to be written to device.
 * @param[in]       size            The size of buffer.
 * 
 * @return          The actually written size on successful, otherwise negative returned.
 ***********************************************************************************************************************
 */
os_size_t os_device_write_block(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_size_t count;
    
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(dev->ref_count != 0);

    if (dev->ops == OS_NULL || dev->ops->write == OS_NULL)
        return 0;

    if (size == 0)
        return 0;

    /* block */    
    os_sem_wait(&dev->sem, OS_WAIT_FOREVER);
    while (dev->tx_size != 0 && dev->tx_count == dev->tx_size)
    {
        os_sem_post(&dev->sem);

        if (os_sem_wait(&dev->tx_sem, dev->tx_timeout) != OS_EOK)
            return 0;

        os_sem_wait(&dev->sem, OS_WAIT_FOREVER);
    }

    /* write */
    count = dev->ops->write(dev, pos, buffer, size);
    
    os_device_notify(dev, ION_GENERIC_WRITE_BLOCK, count);

    /* wake up other thread */
    if (dev->tx_count != dev->tx_size)
    {
        os_sem_post(&dev->tx_sem);
    }

    os_sem_post(&dev->sem);
    return count;
}

os_size_t os_device_write_nonblock(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_size_t count;
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(dev->ref_count != 0);


    if (dev->ops == OS_NULL || dev->ops->write == OS_NULL)
        return 0;

    if (size == 0)
        return 0;

    if (dev->tx_size != 0 && dev->tx_count == dev->tx_size)
        return 0;

    /* nonblock */
    count = dev->ops->write(dev, pos, buffer, size);
    os_device_notify(dev, ION_GENERIC_WRITE_NONBLOCK, count);
    
    return count;
}

/**
 ***********************************************************************************************************************
 * @brief           Control device
 *
 * @details         This function control or change the properties of the device.
 *
 * @param[in]       dev             The descriptor of device control block.
 * @param[in]       cmd             The command sent to device.
 * @param[in]       arg             Control argments.
 *
 * @return          Control result.
 ***********************************************************************************************************************
 */
os_err_t os_device_control(os_device_t *dev, int cmd, void *arg)
{
    os_err_t  ret = OS_EOK;
    os_base_t level;
    
    OS_ASSERT(OS_NULL != dev);
    
    os_device_notify(dev, ION_GENERIC_CONTROL, cmd);

    switch (cmd)
    {
        case OS_DEVICE_CTRL_SET_CB:
        {
            OS_ASSERT(OS_NULL != arg);
            
            struct os_device_cb_info *info = os_calloc(1, sizeof(struct os_device_cb_info));
            struct os_device_cb_info *node;
            struct os_device_cb_info *tmp;
            
            OS_ASSERT(info);

            *info = *(struct os_device_cb_info *)arg;

            level = os_irq_lock();

            os_list_for_each_entry_safe(node, tmp, &dev->cb_heads[info->type], struct os_device_cb_info, list)
            {
                if (info->cb == node->cb)
                {
                    os_irq_unlock(level);
                    os_kprintf("device callback allready exist %p\r\n", info->cb);
                    os_free(info);
                    ret = OS_EBUSY;
                    goto end;
                }
            }

            os_list_add_tail(&dev->cb_heads[info->type], &info->list);

            os_irq_unlock(level);
            goto end;
        }

        case OS_DEVICE_CTRL_RM_CB:
        {
            OS_ASSERT(OS_NULL != arg);

            struct os_device_cb_info *info = arg;
            struct os_device_cb_info *node;
            struct os_device_cb_info *tmp;

            level = os_irq_lock();

            os_list_for_each_entry_safe(node, tmp, &dev->cb_heads[info->type], struct os_device_cb_info, list)
            {
                if (info->cb == node->cb)
                {
                    os_list_del(&node->list);
                    os_irq_unlock(level);
                    os_free(node);
                    goto end;
                }
            }

            os_irq_unlock(level);
            ret = OS_ERROR;
            goto end;
        }

        case OS_DEVICE_CTRL_SET_RX_TIMEOUT:
            dev->rx_timeout = *(os_ubase_t *)arg;
            goto end;

        case OS_DEVICE_CTRL_SET_TX_TIMEOUT:
            dev->tx_timeout = *(os_ubase_t *)arg;
            goto end;
    }

    if (OS_NULL == dev->ops || OS_NULL == dev->ops->control)
    {
        ret = OS_ENOSYS;
        goto end;
    }
    
    os_sem_wait(&dev->sem, OS_WAIT_FOREVER);

    ret = dev->ops->control(dev, cmd, arg);

    os_sem_post(&dev->sem);

end:
    return ret;
}

void os_device_recv_notify(os_device_t *dev)
{
    struct os_device_cb_info *info;

    /* wake up block thread */
    os_sem_post(&dev->rx_sem);

    /* invoke rx callback */
    os_list_for_each_entry(info, &dev->cb_heads[OS_DEVICE_CB_TYPE_RX], struct os_device_cb_info, list)
    {
        if (info != OS_NULL && info->cb != OS_NULL)
        {
            info->size = dev->rx_count;
            info->cb(dev, info);
        }
    }

#ifdef OS_USING_IO_MULTIPLEXING
    dev_poll_wakeup(dev, POLLIN);
#endif
}

void os_device_send_notify(os_device_t *dev)
{
    struct os_device_cb_info *info;

    /* wake up block thread */
    os_sem_post(&dev->tx_sem);
    
    /* invoke tx callback */
    os_list_for_each_entry(info, &dev->cb_heads[OS_DEVICE_CB_TYPE_TX], struct os_device_cb_info, list)
    {
        if (info != OS_NULL && info->cb != OS_NULL)
        {
            info->size = dev->tx_size - dev->tx_count;
            info->cb(dev, info);
        }
    }

#ifdef OS_USING_IO_MULTIPLEXING
    dev_poll_wakeup(dev, POLLOUT);
#endif
}

os_int32_t os_device_for_each(os_err_t (*func)(os_device_t *dev, void *data), void *data)
{
    os_device_t *dev;
    os_device_t *tmp;
    os_int32_t   cnt = 0;

    os_sem_wait(&dev_sem, OS_WAIT_FOREVER);
    
    os_list_for_each_entry_safe(dev, tmp, &os_device_list, os_device_t, list)
    {
        if (OS_EOK != func(dev, data))
        {
            os_sem_post(&dev_sem);
            return cnt;
        }
        cnt++;
    }
    
    os_sem_post(&dev_sem);
    
    return cnt;
}

static os_err_t device_core_init(void)
{
    return os_sem_init(&dev_sem, "dev_sem", 1, 1);
}
OS_CORE_INIT(device_core_init, OS_INIT_SUBLEVEL_LOW);

#ifdef OS_USING_SHELL

#include <shell.h>

static char *const gs_device_type_str[] =
{
    "Character Device",     /* OS_DEVICE_TYPE_CHAR */
    "Block Device",         /* OS_DEVICE_TYPE_BLOCK */
    "Network Interface",    /* OS_DEVICE_TYPE_NETIF */
    "MTD Device",           /* OS_DEVICE_TYPE_MTD */
    "CAN Device",           /* OS_DEVICE_TYPE_CAN */
    "RTC",                  /* OS_DEVICE_TYPE_RTC */
    "Sound Device",         /* OS_DEVICE_TYPE_SOUND */
    "Graphic Device",       /* OS_DEVICE_TYPE_GRAPHIC */
    "I2C Bus",              /* OS_DEVICE_TYPE_I2CBUS */
    "USB Slave Device",     /* OS_DEVICE_TYPE_USBDEVICE */
    "USB Host Bus",         /* OS_DEVICE_TYPE_USBHOST */
    "SPI Bus",              /* OS_DEVICE_TYPE_SPIBUS */
    "SPI Device",           /* OS_DEVICE_TYPE_SPIDEVICE */
    "SDIO Bus",             /* OS_DEVICE_TYPE_SDIO */
    "PM Pseudo Device",     /* OS_DEVICE_TYPE_PM */
    "Pipe",                 /* OS_DEVICE_TYPE_PIPE */
    "Portal Device",        /* OS_DEVICE_TYPE_PORTAL */
    "ClockSource Device",   /* OS_DEVICE_TYPE_CLOCKSOURCE */
    "ClockEvent Device",    /* OS_DEVICE_TYPE_CLOCKEVENT */
    "Miscellaneous Device", /* OS_DEVICE_TYPE_MISCELLANEOUS */
    "Sensor Device",        /* OS_DEVICE_TYPE_SENSOR */
    "Touch Device",         /* OS_DEVICE_TYPE_TOUCH */
    "Infrared Device",      /* OS_DEVICE_TYPE_INFRARED */
    "Wlan Device",          /* OS_DEVICE_TYPE_WLAN */
    "PWM Device",           /* OS_DEVICE_TYPE_PWM */
    "ENCODER Device",       /* OS_DEVICE_TYPE_ENCODER */
    "Genric",               /* OS_DEVICE_TYPE_GENERIC */
};

static os_err_t sh_print_device_info(os_device_t *dev, void *data)
{
    os_kprintf("%-*.*s %-20s %-8d\r\n",
               OS_NAME_MAX, 
               OS_NAME_MAX,
               device_name(dev),
               (dev->type <= OS_DEVICE_TYPE_GENERIC) ? gs_device_type_str[dev->type] : gs_device_type_str[OS_DEVICE_TYPE_GENERIC],
               dev->ref_count);

    return OS_EOK;
}

/**
***********************************************************************************************************************
* @brief           Show all device on the list of device object
*
* @param[in]       argc                argment count
* @param[in]       argv                argment list
*
* @return          Will only return OS_EOK     
***********************************************************************************************************************
*/
os_err_t sh_list_device(os_int32_t argc, char **argv)
{
    os_int32_t   i;

    os_kprintf("%-*.s         type         ref count\r\n", OS_NAME_MAX, "device");
    for (i = 0; i < OS_NAME_MAX; i++)
    {
        os_kprintf("-");
    }
    os_kprintf(" -------------------------------\r\n");

    os_device_for_each(sh_print_device_info, OS_NULL);
    
    return OS_EOK;
}

SH_CMD_EXPORT(device, sh_list_device, "show device information");
#endif 


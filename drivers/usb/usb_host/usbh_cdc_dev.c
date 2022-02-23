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
 * @file        cdc_dev.c
 *
 * @brief       This file provides functions for cdc_dev.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <os_util.h>
#include <os_assert.h>
#include <drv_cfg.h>
#include "usbh_cdc_dev.h"


#define DBG_TAG "cdc_dev"

#ifdef OS_USING_VFS_DEVFS
#include <vfs_posix.h>

#ifdef OS_USING_POSIX_TERMIOS
#include <posix_termios.h>
#endif

/* It's possible the 'getc/putc' is defined by stdio.h in gcc/newlib. */
#ifdef getc
#undef getc
#endif

#ifdef putc
#undef putc
#endif

#endif

#ifdef OS_USBH_CDC_IDLE_TIMER

#define OS_USBH_CDC_RX_TIMER_STATUS_NONE   0
#define OS_USBH_CDC_RX_TIMER_STATUS_ON     1
#define OS_USBH_CDC_RX_TIMER_STATUS_OFF    2

static void _usbh_cdc_rx_timer(void *parameter)
{
    os_base_t level;
    struct os_usbh_cdc_dev *cdc_dev;

    OS_ASSERT(parameter);
    cdc_dev  = (struct os_usbh_cdc_dev *)parameter;

    level = os_irq_lock();

    if (cdc_dev->rx_timer_status != OS_USBH_CDC_RX_TIMER_STATUS_ON)
    {
        os_irq_unlock(level);
        return;
    }

    int count = cdc_dev->ops->recv_count(cdc_dev);

    if (cdc_dev->rx_count == count && count != 0)
    {
        cdc_dev->ops->stop_recv(cdc_dev);
        os_hw_usbh_cdc_isr_rxdone(cdc_dev, count);
    }
    else
    {
        cdc_dev->rx_count = count;
    }
    
    os_irq_unlock(level);
}

#endif

static void _usbh_cdc_start_recv(struct os_usbh_cdc_dev *cdc_dev)
{
    cdc_dev->rx_count = 0;
    cdc_dev->ops->start_recv(cdc_dev, cdc_dev->rx_fifo->line_buff, cdc_dev->config.rx_bufsz);

#ifdef OS_USBH_CDC_IDLE_TIMER
    cdc_dev->rx_timer_status = OS_USBH_CDC_RX_TIMER_STATUS_ON;
#endif
}

static int _usbh_cdc_rx(struct os_usbh_cdc_dev *cdc_dev, os_uint8_t *data, int length)
{
    int size;
    os_base_t level;

    if (length <= 0)
        return 0;

    OS_ASSERT(cdc_dev != OS_NULL);

    level = os_irq_lock();

    OS_ASSERT(cdc_dev->rx_fifo != OS_NULL);

    size = rb_ring_buff_get(&cdc_dev->rx_fifo->rbuff, data, length);
    cdc_dev->parent.rx_count = rb_ring_buff_data_len(&cdc_dev->rx_fifo->rbuff);
    if (size != 0)
    {
        _usbh_cdc_start_recv(cdc_dev);
    }

    os_irq_unlock(level);
    return size;
}

static int _usbh_cdc_tx(struct os_usbh_cdc_dev *cdc_dev, const os_uint8_t *data, int length)
{
    int count;
    os_base_t level;
    struct os_usbh_cdc_tx_fifo *tx_fifo;

    level = os_irq_lock();

    tx_fifo = cdc_dev->tx_fifo;

    length = rb_ring_buff_put(&tx_fifo->rbuff, data, length);

    if (length == 0)
        goto end;
    
    if (tx_fifo->line_buff_count != 0)
        goto end;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, cdc_dev->config.tx_bufsz);

    cdc_dev->flag |= OS_USBH_CDC_FLAG_TX_BUSY;
    tx_fifo->line_buff_count = cdc_dev->ops->start_send(cdc_dev, tx_fifo->line_buff, count);
    OS_ASSERT(tx_fifo->line_buff_count > 0);

    if (tx_fifo->line_buff_count < count)
    {
        rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
    }

end:
    OS_ASSERT(tx_fifo->line_buff_count != 0);
    cdc_dev->parent.tx_count = rb_ring_buff_data_len(&tx_fifo->rbuff);
    os_irq_unlock(level);
    return length;
}

static int _usbh_cdc_poll_tx(struct os_usbh_cdc_dev *cdc_dev, const os_uint8_t *data, int length)
{
    int count;
    int send_index = 0;
    
    OS_ASSERT(cdc_dev != OS_NULL);    

    while (send_index < length)
    {
        count = cdc_dev->ops->poll_send(cdc_dev, data + send_index, length - send_index);

        if (count <= 0)
        {
            break;
        }
        
        send_index += count;
    }

    return send_index;
}

#ifdef OS_USBH_CDC_IDLE_TIMER
#ifdef OS_USING_TICKLESS_LPMGR
static os_err_t os_usbh_cdc_control(struct os_device *dev, int cmd, void *args);

int usbh_cdc_suspend(void *priv, os_uint8_t mode)
{
    return os_usbh_cdc_control((struct os_device *)priv, OS_DEVICE_CTRL_SUSPEND, NULL);
}

void usbh_cdc_resume(void *priv, os_uint8_t mode)
{
    os_usbh_cdc_control((struct os_device *)priv, OS_DEVICE_CTRL_RESUME, NULL);
}

static struct os_lpmgr_device_ops usbh_cdc_lpmgr_ops =
{
    usbh_cdc_suspend,
    usbh_cdc_resume,
};

#endif
#endif

static os_err_t os_usbh_cdc_init(struct os_device *dev)
{
    os_err_t result = OS_EOK;
    struct os_usbh_cdc_dev *cdc_dev;

    OS_ASSERT(dev != OS_NULL);
    
    cdc_dev = (struct os_usbh_cdc_dev *)dev;

    OS_ASSERT(cdc_dev->tx_fifo == OS_NULL);
    OS_ASSERT(cdc_dev->rx_fifo == OS_NULL);

    /* sem */
    os_sem_init(&cdc_dev->sem, dev->name, 1, 1);

    /* rx buff */
    cdc_dev->rx_fifo = (struct os_usbh_cdc_rx_fifo *)os_calloc(1, sizeof(struct os_usbh_cdc_rx_fifo) + cdc_dev->config.rx_bufsz * 2);
    OS_ASSERT(cdc_dev->rx_fifo != OS_NULL);
    cdc_dev->rx_fifo->line_buff = (os_uint8_t *)(cdc_dev->rx_fifo + 1);            
    rb_ring_buff_init(&cdc_dev->rx_fifo->rbuff, cdc_dev->rx_fifo->line_buff + cdc_dev->config.rx_bufsz, cdc_dev->config.rx_bufsz);
    dev->rx_size = cdc_dev->config.rx_bufsz;
    dev->rx_count = 0;
    /* tx buff */
    if (cdc_dev->ops->start_send != OS_NULL)
    {
        struct os_usbh_cdc_tx_fifo *tx_fifo;

        tx_fifo = (struct os_usbh_cdc_tx_fifo *)os_calloc(1, sizeof(struct os_usbh_cdc_tx_fifo) + cdc_dev->config.tx_bufsz * 2);
        OS_ASSERT(tx_fifo != OS_NULL);

        tx_fifo->line_buff = (os_uint8_t *)(tx_fifo + 1);
        tx_fifo->line_buff_count = 0;
        rb_ring_buff_init(&tx_fifo->rbuff, tx_fifo->line_buff + cdc_dev->config.tx_bufsz, cdc_dev->config.tx_bufsz);
        
        cdc_dev->tx_fifo = tx_fifo;
    }
    dev->tx_size = cdc_dev->config.tx_bufsz;
    dev->tx_count = 0;
    /* lowlevel initialization */
    if (cdc_dev->ops->init)
        result = cdc_dev->ops->init(cdc_dev, &cdc_dev->config);

#ifdef OS_USBH_CDC_IDLE_TIMER
    if (cdc_dev->ops->recv_count != OS_NULL)
    {
        if (cdc_dev->rx_timer_status == OS_USBH_CDC_RX_TIMER_STATUS_NONE)
        {
            cdc_dev->rx_timer_status = OS_USBH_CDC_RX_TIMER_STATUS_OFF;
            os_timer_init(&cdc_dev->rx_timer, 
                      device_name(&cdc_dev->parent),
                      _usbh_cdc_rx_timer, 
                      cdc_dev, 
                      ((OS_TICK_PER_SECOND / 100) != 0) ? (OS_TICK_PER_SECOND / 100) : 1,
                      OS_TIMER_FLAG_PERIODIC);

            os_timer_start(&cdc_dev->rx_timer);
#ifdef OS_USING_TICKLESS_LPMGR
            os_lpmgr_device_register(dev, &usbh_cdc_lpmgr_ops);
#endif
        }
    }
#endif

    _usbh_cdc_start_recv(cdc_dev);

    return result;
}

static os_err_t os_usbh_cdc_deinit(struct os_device *dev)
{
    os_base_t level;
    struct os_usbh_cdc_dev *cdc_dev;

    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(dev->ref_count == 0);
    
    cdc_dev = (struct os_usbh_cdc_dev *)dev;
    
    level = os_irq_lock();

#ifdef OS_USBH_CDC_DELAY_CLOSE
    if (cdc_dev->flag & OS_USBH_CDC_FLAG_TX_BUSY)
    {
        os_irq_unlock(level);
        return OS_EBUSY;
    }
#endif

    if (cdc_dev->ops->stop_send)
        cdc_dev->ops->stop_send(cdc_dev);

    if (cdc_dev->ops->stop_recv)
        cdc_dev->ops->stop_recv(cdc_dev);

    if(cdc_dev->ops->deinit)
        cdc_dev->ops->deinit(cdc_dev);

    os_irq_unlock(level);

    os_sem_wait(&cdc_dev->sem, OS_WAIT_FOREVER);

#ifdef OS_USBH_CDC_IDLE_TIMER
    if (cdc_dev->rx_timer_status != OS_USBH_CDC_RX_TIMER_STATUS_NONE)
    {
        os_timer_deinit(&cdc_dev->rx_timer);
        cdc_dev->rx_timer_status = OS_USBH_CDC_RX_TIMER_STATUS_NONE;
#ifdef OS_USING_TICKLESS_LPMGR
        os_lpmgr_device_unregister(dev, &usbh_cdc_lpmgr_ops);
#endif
    }
#endif

    /* rx fifo */
    if (cdc_dev->rx_fifo != OS_NULL)
    {
        os_free(cdc_dev->rx_fifo);
        cdc_dev->rx_fifo = OS_NULL;
    }

    /* tx fifo */
    if (cdc_dev->tx_fifo != OS_NULL)
    {
        os_free(cdc_dev->tx_fifo);
        cdc_dev->tx_fifo = OS_NULL;
    }

    os_sem_post(&cdc_dev->sem);

    os_sem_deinit(&cdc_dev->sem);

    return OS_EOK;
}

static os_size_t os_usbh_cdc_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_usbh_cdc_dev *cdc_dev;

    OS_ASSERT(dev != OS_NULL);
    
    if (size == 0)
        return 0;

    cdc_dev = (struct os_usbh_cdc_dev *)dev;

    return _usbh_cdc_rx(cdc_dev, (os_uint8_t *)buffer, size);
}

static os_size_t os_usbh_cdc_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_usbh_cdc_dev *cdc_dev;

    OS_ASSERT(dev != OS_NULL);
    if (size == 0)
        return 0;

    cdc_dev = (struct os_usbh_cdc_dev *)dev;

    if (os_console_get_device() == dev)
        return _usbh_cdc_poll_tx(cdc_dev, (const os_uint8_t *)buffer, size);

    if (cdc_dev->ops->start_send != OS_NULL)
        return _usbh_cdc_tx(cdc_dev, (const os_uint8_t *)buffer, size);

    return _usbh_cdc_poll_tx(cdc_dev, (const os_uint8_t *)buffer, size);
}

#ifdef OS_USING_POSIX_TERMIOS
struct speed_baudrate_item
{
    speed_t speed;
    int     baudrate;
};

const static struct speed_baudrate_item _tbl[] =
{
    {B2400, BAUD_RATE_2400},
    {B4800, BAUD_RATE_4800},
    {B9600, BAUD_RATE_9600},
    {B19200, BAUD_RATE_19200},
    {B38400, BAUD_RATE_38400},
    {B57600, BAUD_RATE_57600},
    {B115200, BAUD_RATE_115200},
    {B230400, BAUD_RATE_230400},
    {B460800, BAUD_RATE_460800},
    {B921600, BAUD_RATE_921600},
    {B2000000, BAUD_RATE_2000000},
    {B3000000, BAUD_RATE_3000000},
};

static speed_t _get_speed(int baudrate)
{
    int index;

    for (index = 0; index < sizeof(_tbl) / sizeof(_tbl[0]); index++)
    {
        if (_tbl[index].baudrate == baudrate)
            return _tbl[index].speed;
    }

    return B0;
}

static int _get_baudrate(speed_t speed)
{
    int index;

    for (index = 0; index < sizeof(_tbl) / sizeof(_tbl[0]); index++)
    {
        if (_tbl[index].speed == speed)
            return _tbl[index].baudrate;
    }

    return 0;
}

static void _tc_flush(struct os_usbh_cdc_dev *cdc_dev, int queue)
{
    os_base_t level;
    os_uint8_t c;
    int ch = -1;

    struct os_device *        device  = OS_NULL;

    OS_ASSERT(cdc_dev != OS_NULL);

    device  = &(cdc_dev->parent);

    switch (queue)
    {
    case TCIFLUSH:
    case TCIOFLUSH:

        OS_ASSERT(cdc_dev->rx_fifo != OS_NULL);
        level = os_irq_lock();
        rb_ring_buff_reset(&cdc_dev->rx_fifo->rbuff);
        os_irq_unlock(level);

        break;

    case TCOFLUSH:
        break;
    }
}

#endif

static os_err_t os_usbh_cdc_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t ret = OS_EOK;
    struct os_usbh_cdc_dev *cdc_dev;

    OS_ASSERT(dev != OS_NULL);
    cdc_dev = (struct os_usbh_cdc_dev *)dev;

    switch (cmd)
    {
#ifdef OS_USBH_CDC_IDLE_TIMER
    case OS_DEVICE_CTRL_SUSPEND:
        if (cdc_dev->rx_timer_status != OS_USBH_CDC_RX_TIMER_STATUS_NONE)
        {
            os_timer_stop(&cdc_dev->rx_timer);
        }
        break;

    case OS_DEVICE_CTRL_RESUME:
        if (cdc_dev->rx_timer_status != OS_USBH_CDC_RX_TIMER_STATUS_NONE)
        {
            os_timer_start(&cdc_dev->rx_timer);
        }
        break;
#endif

    case OS_DEVICE_CTRL_CONFIG:
        if (args)
        {
            struct usbh_cdc_configure *pconfig = (struct usbh_cdc_configure *)args;
            if ((pconfig->rx_bufsz != cdc_dev->config.rx_bufsz || pconfig->tx_bufsz != cdc_dev->config.tx_bufsz) &&
                cdc_dev->parent.ref_count)
            {
                /*Can not change buffer size*/
                return OS_EBUSY;
            }
            /* Set cdc_dev configure */
            cdc_dev->config = *pconfig;
            if (cdc_dev->parent.ref_count)
            {
                if(cdc_dev->ops->deinit)
                    cdc_dev->ops->deinit(cdc_dev);

                /* Serial device has been opened, to configure it */
                if (cdc_dev->ops->init)
                {
                    ret = cdc_dev->ops->init(cdc_dev, (struct usbh_cdc_configure *)args);
                    if (ret == OS_EOK)
                    {
                        _usbh_cdc_start_recv(cdc_dev);
                    }
                    return ret;
                }
                else
                {
                    return OS_EOK;
                }
            }
        }
        break;

#ifdef OS_USING_POSIX_TERMIOS
    case TCGETA:
    {
        struct termios *tio = (struct termios *)args;
        if (tio == OS_NULL)
            return OS_EINVAL;

        tio->c_iflag = 0;
        tio->c_oflag = 0;
        tio->c_lflag = 0;

        /* Update oflag for console device */
        if (os_console_get_device() == dev)
            tio->c_oflag = OPOST | ONLCR;

        /* Set cflag */
        tio->c_cflag = 0;
        if (cdc_dev->config.data_bits == DATA_BITS_5)
            tio->c_cflag = CS5;
        else if (cdc_dev->config.data_bits == DATA_BITS_6)
            tio->c_cflag = CS6;
        else if (cdc_dev->config.data_bits == DATA_BITS_7)
            tio->c_cflag = CS7;
        else if (cdc_dev->config.data_bits == DATA_BITS_8)
            tio->c_cflag = CS8;

        if (cdc_dev->config.stop_bits == STOP_BITS_2)
            tio->c_cflag |= CSTOPB;

        if (cdc_dev->config.parity == PARITY_EVEN)
            tio->c_cflag |= PARENB;
        else if (cdc_dev->config.parity == PARITY_ODD)
            tio->c_cflag |= (PARODD | PARENB);

        cfsetospeed(tio, _get_speed(cdc_dev->config.baud_rate));
    }
    break;

    case TCSETAW:
    case TCSETAF:
    case TCSETA:
    {
        int baudrate;
        struct usbh_cdc_configure config;

        struct termios *tio = (struct termios *)args;
        if (tio == OS_NULL)
            return OS_EINVAL;

        config = cdc_dev->config;

        baudrate         = _get_baudrate(cfgetospeed(tio));
        config.baud_rate = baudrate;

        switch (tio->c_cflag & CSIZE)
        {
        case CS5:
            config.data_bits = DATA_BITS_5;
            break;
        case CS6:
            config.data_bits = DATA_BITS_6;
            break;
        case CS7:
            config.data_bits = DATA_BITS_7;
            break;
        default:
            config.data_bits = DATA_BITS_8;
            break;
        }

        if (tio->c_cflag & CSTOPB)
            config.stop_bits = STOP_BITS_2;
        else
            config.stop_bits = STOP_BITS_1;

        if (tio->c_cflag & PARENB)
        {
            if (tio->c_cflag & PARODD)
                config.parity = PARITY_ODD;
            else
                config.parity = PARITY_EVEN;
        }
        else
            config.parity = PARITY_NONE;

        if (cdc_dev->ops->init)
            cdc_dev->ops->init(cdc_dev, &config);
    }
    break;
    case TCFLSH:
    {
        int queue = (int)args;

        _tc_flush(cdc_dev, queue);
    }

    break;
    case TCXONC:
        break;
#endif
#if 0 && defined(OS_USING_VFS_DEVFS)
    case FIONREAD:
    {
        os_size_t recved = 0;
        os_base_t level;

        level  = os_irq_lock();
        recved = rb_ring_buff_data_len(&cdc_dev->rx_fifo->rbuff);
        os_irq_unlock(level);

        *(os_size_t *)args = recved;
    }
    break;
#endif
    default:
        /* Control device */
        ret = OS_ENOSYS;
        break;
    }

    return ret;
}

const static struct os_device_ops usbh_cdc_ops = 
{
    .init    = os_usbh_cdc_init,
    .deinit  = os_usbh_cdc_deinit,
    .read    = os_usbh_cdc_read,
    .write   = os_usbh_cdc_write,
    .control = os_usbh_cdc_control
};

#ifdef OS_USBH_CDC_DELAY_CLOSE
static void __os_usbh_cdc_deinit(void *data)
{
    struct os_device *dev;

    dev = (struct os_device *)data;
    os_sem_wait(&dev->sem, OS_WAIT_FOREVER);
    os_usbh_cdc_deinit(dev);
    dev->state = OS_DEVICE_STATE_CLOSE;
    os_sem_post(&dev->sem);
}
#endif

os_err_t os_hw_usbh_cdc_register(struct os_usbh_cdc_dev *cdc_dev, const char *name, void *data)
{
    os_err_t          ret;
    struct os_device *device;
    OS_ASSERT(cdc_dev != OS_NULL);

    OS_ASSERT(cdc_dev->ops->start_recv != OS_NULL);

    device = &(cdc_dev->parent);

    device->type        = OS_DEVICE_TYPE_CHAR;

    device->ops = &usbh_cdc_ops;
    device->user_data = data;

    /* Register a character device */
    ret = os_device_register(device, name);

#ifdef OS_USBH_CDC_DELAY_CLOSE
    os_work_init(&cdc_dev->work, __os_usbh_cdc_deinit, &cdc_dev->parent);
#endif

    return ret;
}

os_err_t os_hw_usbh_cdc_unregister(struct os_usbh_cdc_dev *cdc_dev)
{
    os_err_t          ret;
    struct os_device *device;
    OS_ASSERT(cdc_dev != OS_NULL);

    device = &(cdc_dev->parent);

    /* unregister a character device */
    ret = os_device_unregister(device);

    return ret;
}

void os_hw_usbh_cdc_isr_rxdone(struct os_usbh_cdc_dev *cdc_dev, int count)
{
    int count_put;
    os_device_t *dev;

    OS_ASSERT(cdc_dev);
    OS_ASSERT(cdc_dev->rx_fifo != OS_NULL);
    OS_ASSERT(count != 0);

#ifdef OS_USBH_CDC_IDLE_TIMER
    cdc_dev->rx_timer_status = OS_USBH_CDC_RX_TIMER_STATUS_OFF;
#endif

    dev = &cdc_dev->parent;
    count_put = rb_ring_buff_put(&cdc_dev->rx_fifo->rbuff, cdc_dev->rx_fifo->line_buff, count);
    dev->rx_count = rb_ring_buff_data_len(&cdc_dev->rx_fifo->rbuff);
    os_device_recv_notify(dev);

    if (count_put == count)
    {
        _usbh_cdc_start_recv(cdc_dev);
    }
    else
    {
        /* drop some data */
    }
}

void os_hw_usbh_cdc_isr_txdone(struct os_usbh_cdc_dev *cdc_dev)
{
    int count;
    os_device_t *dev;
    struct os_usbh_cdc_tx_fifo *tx_fifo;

    dev = &cdc_dev->parent;

    tx_fifo = (struct os_usbh_cdc_tx_fifo *)cdc_dev->tx_fifo;

    OS_ASSERT(tx_fifo->line_buff_count != 0);

    tx_fifo->line_buff_count = 0;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, cdc_dev->config.tx_bufsz);

    if (count == 0)
    {
        cdc_dev->flag &= ~OS_USBH_CDC_FLAG_TX_BUSY;
        
#ifdef OS_USBH_CDC_DELAY_CLOSE
        if (cdc_dev->parent.state == OS_DEVICE_STATE_CLOSING)
        {
            os_submit_work(&cdc_dev->work, 0);
        }
#endif
        dev->tx_count = 0;
        os_device_send_notify(dev);
    }
    else
    {
        tx_fifo->line_buff_count = cdc_dev->ops->start_send(cdc_dev, tx_fifo->line_buff, count);
        OS_ASSERT(tx_fifo->line_buff_count > 0);

        if (tx_fifo->line_buff_count < count)
        {
            rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
        }
        
        dev->tx_count = rb_ring_buff_data_len(&tx_fifo->rbuff);
    }
}


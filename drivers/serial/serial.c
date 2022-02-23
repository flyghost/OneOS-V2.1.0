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
 * @file        serial.c
 *
 * @brief       This file provides functions for serial.
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

#define DBG_TAG "serial"

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

static int _serial_rx(struct os_serial_device *serial, os_uint8_t *data, int length)
{
    int count;
    os_base_t level;

    if (length <= 0)
        return 0;

    OS_ASSERT(serial != OS_NULL);

    level = os_irq_lock();

    OS_ASSERT(serial->rx_fifo != OS_NULL);

    count = min(ring_count(&serial->rx_fifo->ring), length);
    
    if (count != 0)
    {
        copy_ring_to_line(&serial->rx_fifo->ring, data, count);
    }

    serial->parent.rx_count = ring_count(&serial->rx_fifo->ring);

    os_irq_unlock(level);
    
    return count;
}

static int _serial_tx(struct os_serial_device *serial, const os_uint8_t *data, int length)
{
    int count;
    os_base_t level;
    struct os_serial_tx_fifo *tx_fifo;

    level = os_irq_lock();

    tx_fifo = serial->tx_fifo;

    length = rb_ring_buff_put(&tx_fifo->rbuff, data, length);

    if (length == 0)
        goto end;
    
    if (tx_fifo->line_buff_count != 0)
        goto end;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, serial->config.tx_bufsz);

#ifdef OS_SERIAL_DELAY_CLOSE
    os_sem_wait(&serial->delay_sem, OS_NO_WAIT);
#endif
    
    tx_fifo->line_buff_count = serial->ops->start_send(serial, tx_fifo->line_buff, count);
    OS_ASSERT(tx_fifo->line_buff_count > 0);

    if (tx_fifo->line_buff_count < count)
    {
        rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
    }

end:
    OS_ASSERT(tx_fifo->line_buff_count != 0);
    serial->parent.tx_count = rb_ring_buff_data_len(&tx_fifo->rbuff);
    os_irq_unlock(level);
    return length;
}

static int _serial_poll_tx(struct os_serial_device *serial, const os_uint8_t *data, int length)
{
    int count;
    int send_index = 0;
    
    OS_ASSERT(serial != OS_NULL);    

    while (send_index < length)
    {
        count = serial->ops->poll_send(serial, data + send_index, length - send_index);

        if (count <= 0)
        {
            break;
        }
        
        send_index += count;
    }

    return send_index;
}

static os_err_t os_serial_init(struct os_device *dev)
{
    os_err_t    result = OS_EOK;
    os_uint32_t rx_bufsz = 0;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    
    serial = (struct os_serial_device *)dev;

    os_sem_wait(&serial->sem, OS_WAIT_FOREVER);

    OS_ASSERT(serial->rx_fifo == OS_NULL);
    OS_ASSERT(serial->tx_fifo == OS_NULL);

    /* rx buff */    
    rx_bufsz = OS_ALIGN_UP(serial->config.rx_bufsz + 1, 4);
    serial->rx_fifo = (struct os_serial_rx_fifo *)os_dma_malloc_align(sizeof(struct os_serial_rx_fifo) + rx_bufsz, 32);
    OS_ASSERT(serial->rx_fifo != OS_NULL);

    serial->rx_fifo->ring.buff = (os_uint8_t *)(serial->rx_fifo + 1);
    serial->rx_fifo->ring.size = rx_bufsz;
    
    /* tx buff */
    if (serial->ops->start_send != OS_NULL)
    {
        serial->tx_fifo = (struct os_serial_tx_fifo *)os_dma_malloc_align(sizeof(struct os_serial_tx_fifo) + serial->config.tx_bufsz * 2, 32);
        OS_ASSERT(serial->tx_fifo != OS_NULL);
    
        serial->tx_fifo->line_buff = (os_uint8_t *)(serial->tx_fifo + 1);
        serial->tx_fifo->line_buff_count = 0;
        
        rb_ring_buff_init(&serial->tx_fifo->rbuff, serial->tx_fifo->line_buff + serial->config.tx_bufsz, serial->config.tx_bufsz);
    }

    dev->rx_size = serial->config.rx_bufsz;
    dev->rx_count = 0;
    
    dev->tx_size = serial->config.tx_bufsz;
    dev->tx_count = 0;
    
    /* lowlevel initialization */
    result = serial->ops->init(serial, &serial->config);

#ifdef OS_SERIAL_DELAY_CLOSE
    os_sem_post(&serial->delay_sem);
#endif

    os_sem_post(&serial->sem);

    return result;
}

static os_err_t os_serial_deinit(struct os_device *dev)
{
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    
    serial = (struct os_serial_device *)dev;

    os_sem_wait(&serial->sem, OS_WAIT_FOREVER);

#ifdef OS_SERIAL_DELAY_CLOSE
    os_sem_wait(&serial->delay_sem, OS_WAIT_FOREVER);
#endif

    serial->ops->deinit(serial);

    /* rx fifo */
    if (serial->rx_fifo != OS_NULL)
    {
        os_dma_free_align(serial->rx_fifo);
        serial->rx_fifo = OS_NULL;
    }

    /* tx fifo */
    if (serial->tx_fifo != OS_NULL)
    {
        os_dma_free_align(serial->tx_fifo);
        serial->tx_fifo = OS_NULL;
    }

    os_sem_post(&serial->sem);

    return OS_EOK;
}

static os_size_t os_serial_read(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    
    if (size == 0)
        return 0;

    serial = (struct os_serial_device *)dev;

    return _serial_rx(serial, (os_uint8_t *)buffer, size);
}

static os_size_t os_serial_write(struct os_device *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    if (size == 0)
        return 0;

    serial = (struct os_serial_device *)dev;

    if (os_console_get_device() == dev)
        return _serial_poll_tx(serial, (const os_uint8_t *)buffer, size);

    if (serial->ops->start_send != OS_NULL)
        return _serial_tx(serial, (const os_uint8_t *)buffer, size);

    return _serial_poll_tx(serial, (const os_uint8_t *)buffer, size);
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

static void _tc_flush(struct os_serial_device *serial, int queue)
{
    os_base_t level;
    os_uint8_t c;
    int ch = -1;

    struct os_device *        device  = OS_NULL;

    OS_ASSERT(serial != OS_NULL);

    device  = &(serial->parent);

    switch (queue)
    {
    case TCIFLUSH:
    case TCIOFLUSH:

        OS_ASSERT(serial->rx_fifo != OS_NULL);
        level = os_irq_lock();
        serial->rx_fifo->ring.count = 0;
        serial->rx_fifo->ring.size  = 0;
        serial->rx_fifo->ring.head  = 0;
        serial->rx_fifo->ring.tail  = 0;
        os_irq_unlock(level);

        break;

    case TCOFLUSH:
        break;
    }
}

#endif

static os_err_t os_serial_control(struct os_device *dev, int cmd, void *args)
{
    os_err_t ret = OS_EOK;
    struct os_serial_device *serial;

    OS_ASSERT(dev != OS_NULL);
    serial = (struct os_serial_device *)dev;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_CONFIG:
        if (args)
        {
            serial->config = *(struct serial_configure *)args;
            
            if (serial->parent.ref_count)
            {
                os_serial_deinit(dev);
                return os_serial_init(dev);
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
        if (serial->config.data_bits == DATA_BITS_5)
            tio->c_cflag = CS5;
        else if (serial->config.data_bits == DATA_BITS_6)
            tio->c_cflag = CS6;
        else if (serial->config.data_bits == DATA_BITS_7)
            tio->c_cflag = CS7;
        else if (serial->config.data_bits == DATA_BITS_8)
            tio->c_cflag = CS8;

        if (serial->config.stop_bits == STOP_BITS_2)
            tio->c_cflag |= CSTOPB;

        if (serial->config.parity == PARITY_EVEN)
            tio->c_cflag |= PARENB;
        else if (serial->config.parity == PARITY_ODD)
            tio->c_cflag |= (PARODD | PARENB);

        cfsetospeed(tio, _get_speed(serial->config.baud_rate));
    }
    break;

    case TCSETAW:
    case TCSETAF:
    case TCSETA:
    {
        int baudrate;
        struct serial_configure config;

        struct termios *tio = (struct termios *)args;
        if (tio == OS_NULL)
            return OS_EINVAL;

        config = serial->config;

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

        if (serial->ops->init)
            serial->ops->init(serial, &config);
    }
    break;
    case TCFLSH:
    {
        int queue = (int)args;

        _tc_flush(serial, queue);
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
        recved = ring_count(&serial->rx_fifo->ring);
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

const static struct os_device_ops serial_ops = 
{
    .init    = os_serial_init,
    .deinit  = os_serial_deinit,
    .read    = os_serial_read,
    .write   = os_serial_write,
    .control = os_serial_control
};

os_err_t os_hw_serial_register(struct os_serial_device *serial, const char *name, void *data)
{
    os_err_t          ret;
    struct os_device *device;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(serial->ops != OS_NULL);
    OS_ASSERT(serial->ops->init != OS_NULL);
    OS_ASSERT(serial->ops->deinit != OS_NULL);
    OS_ASSERT(serial->ops->poll_send != OS_NULL);

    device = &(serial->parent);

    device->type = OS_DEVICE_TYPE_CHAR;
    device->ops  = &serial_ops;
    device->user_data = data;

    /* Register a character device */
    ret = os_device_register(device, name);

    /* sem */
    os_sem_init(&serial->sem, OS_NULL, 1, 1);

#ifdef OS_SERIAL_DELAY_CLOSE
    os_sem_init(&serial->delay_sem, OS_NULL, 1, 1);
#endif

    return ret;
}

os_err_t os_hw_serial_unregister(struct os_serial_device *serial)
{
    return OS_EOK;
}

void os_hw_serial_isr_rxdone(struct os_serial_device *serial)
{
    os_device_t *dev;

    OS_ASSERT(serial);
    OS_ASSERT(serial->rx_fifo != OS_NULL);

    dev = &serial->parent;

    OS_ASSERT(dev != OS_NULL);
    
    dev->rx_count = ring_count(&serial->rx_fifo->ring);

    OS_ASSERT(dev->rx_count > 0);
    
    os_device_recv_notify(dev);
}

void os_hw_serial_isr_txdone(struct os_serial_device *serial)
{
    int count;
    os_device_t *dev;
    struct os_serial_tx_fifo *tx_fifo;

    dev = &serial->parent;

    tx_fifo = (struct os_serial_tx_fifo *)serial->tx_fifo;

    OS_ASSERT(tx_fifo->line_buff_count != 0);

    tx_fifo->line_buff_count = 0;

    count = rb_ring_buff_get(&tx_fifo->rbuff, tx_fifo->line_buff, serial->config.tx_bufsz);

    if (count == 0)
    {
#ifdef OS_SERIAL_DELAY_CLOSE
        os_sem_post(&serial->delay_sem);
#endif
        
        dev->tx_count = 0;
        os_device_send_notify(dev);
    }
    else
    {
        tx_fifo->line_buff_count = serial->ops->start_send(serial, tx_fifo->line_buff, count);
        OS_ASSERT(tx_fifo->line_buff_count > 0);

        if (tx_fifo->line_buff_count < count)
        {
            rb_ring_buff_put(&tx_fifo->rbuff, tx_fifo->line_buff + tx_fifo->line_buff_count, count - tx_fifo->line_buff_count);
        }
        
        dev->tx_count = rb_ring_buff_data_len(&tx_fifo->rbuff);
    }
}


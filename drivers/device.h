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
 * @file        os_device.h
 *
 * @brief       Header file for device interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-27   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __OS_DEVICE_H__
#define __OS_DEVICE_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_list.h>
#include <os_sem.h>
#include <os_errno.h>

#ifdef OS_USING_DEVICE

#ifdef __cplusplus
extern "C" {
#endif

enum os_device_type
{
    OS_DEVICE_TYPE_CHAR = 0,        /* character device. */
    OS_DEVICE_TYPE_BLOCK,           /* block device. */
    OS_DEVICE_TYPE_NETIF,           /* Net interface. */
    OS_DEVICE_TYPE_MTD,             /* Memory device. */
    OS_DEVICE_TYPE_CAN,             /* CAN device. */
    OS_DEVICE_TYPE_RTC,             /* RTC device. */
    OS_DEVICE_TYPE_SOUND,           /* Sound device. */
    OS_DEVICE_TYPE_GRAPHIC,         /* Graphic device. */
    OS_DEVICE_TYPE_I2CBUS,          /* I2C bus device. */
    OS_DEVICE_TYPE_USBDEVICE,       /* USB slave device. */
    OS_DEVICE_TYPE_USBHOST,         /* USB host bus. */
    OS_DEVICE_TYPE_SPIBUS,          /* SPI bus device. */
    OS_DEVICE_TYPE_SPIDEVICE,       /* SPI device. */
    OS_DEVICE_TYPE_SDIO,            /* SDIO bus device. */
    OS_DEVICE_TYPE_PM,              /* PM pseudo device. */
    OS_DEVICE_TYPE_PIPE,            /* Pipe device. */
    OS_DEVICE_TYPE_PORTAL,          /* Portal device. */
    OS_DEVICE_TYPE_CLOCKSOURCE,     /* ClockSource device. */
    OS_DEVICE_TYPE_CLOCKEVENT,      /* ClockEvent device. */
    OS_DEVICE_TYPE_MISCELLANEOUS,   /* Miscellaneous device. */
    OS_DEVICE_TYPE_SENSOR,          /* Sensor device. */
    OS_DEVICE_TYPE_TOUCH,           /* Touch device. */
    OS_DEVICE_TYPE_INFRARED,        /* Infrared device. */
    OS_DEVICE_TYPE_WLAN,            /* Wlan device. */
    OS_DEVICE_TYPE_PWM,             /* PWM device. */
    OS_DEVICE_TYPE_ENCODER,         /* Encoder device. */
    OS_DEVICE_TYPE_GENERIC          /* Generic device. */
};

/* device io control commands */
#define IOC_CHAR(cmd)           ((OS_DEVICE_TYPE_CHAR << 8) | (cmd))
#define IOC_BLOCK(cmd)          ((OS_DEVICE_TYPE_BLOCK << 8) | (cmd))
#define IOC_NETIF(cmd)          ((OS_DEVICE_TYPE_NETIF << 8) | (cmd))
#define IOC_MTD(cmd)            ((OS_DEVICE_TYPE_MTD << 8) | (cmd))
#define IOC_CAN(cmd)            ((OS_DEVICE_TYPE_CAN << 8) | (cmd))
#define IOC_RTC(cmd)            ((OS_DEVICE_TYPE_RTC << 8) | (cmd))
#define IOC_SOUND(cmd)          ((OS_DEVICE_TYPE_SOUND << 8) | (cmd))
#define IOC_GRAPHIC(cmd)        ((OS_DEVICE_TYPE_GRAPHIC << 8) | (cmd))
#define IOC_I2CBUS(cmd)         ((OS_DEVICE_TYPE_I2CBUS << 8) | (cmd))
#define IOC_USBDEVICE(cmd)      ((OS_DEVICE_TYPE_USBDEVICE << 8) | (cmd))
#define IOC_USBHOST(cmd)        ((OS_DEVICE_TYPE_USBHOST << 8) | (cmd))
#define IOC_SPIBUS(cmd)         ((OS_DEVICE_TYPE_SPIBUS << 8) | (cmd))
#define IOC_SPIDEVICE(cmd)      ((OS_DEVICE_TYPE_SPIDEVICE << 8) | (cmd))
#define IOC_SDIO(cmd)           ((OS_DEVICE_TYPE_SDIO << 8) | (cmd))
#define IOC_PM(cmd)             ((OS_DEVICE_TYPE_PM << 8) | (cmd))
#define IOC_PIPE(cmd)           ((OS_DEVICE_TYPE_PIPE << 8) | (cmd))
#define IOC_PORTAL(cmd)         ((OS_DEVICE_TYPE_PORTAL << 8) | (cmd))
#define IOC_CLOCKSOURCE(cmd)    ((OS_DEVICE_TYPE_CLOCKSOURCE << 8) | (cmd))
#define IOC_CLOCKEVENT(cmd)     ((OS_DEVICE_TYPE_CLOCKEVENT << 8) | (cmd))
#define IOC_MISCELLANEOUS(cmd)  ((OS_DEVICE_TYPE_MISCELLANEOUS << 8) | (cmd))
#define IOC_SENSOR(cmd)         ((OS_DEVICE_TYPE_SENSOR << 8) | (cmd))
#define IOC_TOUCH(cmd)          ((OS_DEVICE_TYPE_TOUCH << 8) | (cmd))
#define IOC_INFRARED(cmd)       ((OS_DEVICE_TYPE_INFRARED << 8) | (cmd))
#define IOC_WLAN(cmd)           ((OS_DEVICE_TYPE_WLAN << 8) | (cmd))
#define IOC_PWM(cmd)            ((OS_DEVICE_TYPE_PWM << 8) | (cmd))
#define IOC_ENCODER(cmd)        ((OS_DEVICE_TYPE_ENCODER << 8) | (cmd))
#define IOC_GENERIC(cmd)        ((OS_DEVICE_TYPE_GENERIC << 8) | (cmd))

/* general commands */
#define OS_DEVICE_CTRL_FLUSH            IOC_GENERIC(1)
#define OS_DEVICE_CTRL_SEEK             IOC_GENERIC(2)
#define OS_DEVICE_CTRL_SET_CB           IOC_GENERIC(3)
#define OS_DEVICE_CTRL_RM_CB            IOC_GENERIC(4)
#define OS_DEVICE_CTRL_RESUME           IOC_GENERIC(5)
#define OS_DEVICE_CTRL_SUSPEND          IOC_GENERIC(6)
#define OS_DEVICE_CTRL_CONFIG           IOC_GENERIC(7)
#define OS_DEVICE_CTRL_SET_RX_TIMEOUT   IOC_GENERIC(8)
#define OS_DEVICE_CTRL_SET_TX_TIMEOUT   IOC_GENERIC(9)

/* special commands */
#define OS_DEVICE_CTRL_CHAR_STREAM      0x10        /* Stream mode on char device. */
#define OS_DEVICE_CTRL_NETIF_GETMAC     0x10        /* Get mac address. */
#define OS_DEVICE_CTRL_MTD_FORMAT       0x10        /* Format a MTD device. */

#ifdef OS_USING_DEVICE_NOTIFY
/* general notify */
#define ION_CHAR(cmd)           ((OS_DEVICE_TYPE_CHAR << 8) | (cmd))
#define ION_BLOCK(cmd)          ((OS_DEVICE_TYPE_BLOCK << 8) | (cmd))
#define ION_NETIF(cmd)          ((OS_DEVICE_TYPE_NETIF << 8) | (cmd))
#define ION_MTD(cmd)            ((OS_DEVICE_TYPE_MTD << 8) | (cmd))
#define ION_CAN(cmd)            ((OS_DEVICE_TYPE_CAN << 8) | (cmd))
#define ION_RTC(cmd)            ((OS_DEVICE_TYPE_RTC << 8) | (cmd))
#define ION_SOUND(cmd)          ((OS_DEVICE_TYPE_SOUND << 8) | (cmd))
#define ION_GRAPHIC(cmd)        ((OS_DEVICE_TYPE_GRAPHIC << 8) | (cmd))
#define ION_I2CBUS(cmd)         ((OS_DEVICE_TYPE_I2CBUS << 8) | (cmd))
#define ION_USBDEVICE(cmd)      ((OS_DEVICE_TYPE_USBDEVICE << 8) | (cmd))
#define ION_USBHOST(cmd)        ((OS_DEVICE_TYPE_USBHOST << 8) | (cmd))
#define ION_SPIBUS(cmd)         ((OS_DEVICE_TYPE_SPIBUS << 8) | (cmd))
#define ION_SPIDEVICE(cmd)      ((OS_DEVICE_TYPE_SPIDEVICE << 8) | (cmd))
#define ION_SDIO(cmd)           ((OS_DEVICE_TYPE_SDIO << 8) | (cmd))
#define ION_PM(cmd)             ((OS_DEVICE_TYPE_PM << 8) | (cmd))
#define ION_PIPE(cmd)           ((OS_DEVICE_TYPE_PIPE << 8) | (cmd))
#define ION_PORTAL(cmd)         ((OS_DEVICE_TYPE_PORTAL << 8) | (cmd))
#define ION_CLOCKSOURCE(cmd)    ((OS_DEVICE_TYPE_CLOCKSOURCE << 8) | (cmd))
#define ION_CLOCKEVENT(cmd)     ((OS_DEVICE_TYPE_CLOCKEVENT << 8) | (cmd))
#define ION_MISCELLANEOUS(cmd)  ((OS_DEVICE_TYPE_MISCELLANEOUS << 8) | (cmd))
#define ION_SENSOR(cmd)         ((OS_DEVICE_TYPE_SENSOR << 8) | (cmd))
#define ION_TOUCH(cmd)          ((OS_DEVICE_TYPE_TOUCH << 8) | (cmd))
#define ION_INFRARED(cmd)       ((OS_DEVICE_TYPE_INFRARED << 8) | (cmd))
#define ION_WLAN(cmd)           ((OS_DEVICE_TYPE_WLAN << 8) | (cmd))
#define ION_GENERIC(cmd)        ((OS_DEVICE_TYPE_GENERIC << 8) | (cmd))

#define ION_GENERIC_NONE                ION_GENERIC(0)
#define ION_GENERIC_REGISTER            ION_GENERIC(1)
#define ION_GENERIC_UNREGISTER          ION_GENERIC(2)
#define ION_GENERIC_OPEN                ION_GENERIC(3)
#define ION_GENERIC_CLOSE               ION_GENERIC(4)
#define ION_GENERIC_READ_BLOCK          ION_GENERIC(5)
#define ION_GENERIC_READ_NONBLOCK       ION_GENERIC(6)
#define ION_GENERIC_WRITE_BLOCK         ION_GENERIC(7)
#define ION_GENERIC_WRITE_NONBLOCK      ION_GENERIC(8)
#define ION_GENERIC_CONTROL             ION_GENERIC(9)
#define ION_GENERIC_NUM                 ION_GENERIC(10)
#endif

#define device_name(dev)     ((dev)->name)

typedef struct os_device os_device_t;

typedef os_err_t (*device_notify_callback)(os_device_t *dev, os_ubase_t event, os_ubase_t args);
typedef os_err_t (*device_notify_filter)(os_device_t *dev, os_ubase_t event, os_ubase_t args);

struct os_device_notify_cb_info
{
    device_notify_callback  callback;
    device_notify_filter    filter;
    os_list_node_t          list;
};

struct os_device_ops
{
    os_err_t  (*init)   (os_device_t *dev);
    os_err_t  (*deinit) (os_device_t *dev);
    os_size_t (*read)   (os_device_t *dev, os_off_t pos, void *buffer, os_size_t size);
    os_size_t (*write)  (os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size);
    os_err_t  (*control)(os_device_t *dev, os_int32_t cmd, void *args);
};

enum os_device_cb_type
{
    OS_DEVICE_CB_TYPE_RX = 0,
    OS_DEVICE_CB_TYPE_TX,
    OS_DEVICE_CB_TYPE_NUM,
};

struct os_device_cb_info
{
    int   type;
    int   size;
    void *data;
    os_err_t (*cb)(os_device_t *dev, struct os_device_cb_info *info);

    os_list_node_t list;
};

struct os_device
{
    char name[OS_NAME_MAX + 1];
    
    enum os_device_type         type;
    os_int32_t                  ref_count;

    const struct os_device_ops *ops;

    os_list_node_t list;

    os_list_node_t cb_heads[OS_DEVICE_CB_TYPE_NUM];

    os_sem_t sem;
    
    os_sem_t tx_sem;
    os_sem_t rx_sem;

    os_int32_t tx_count;
    os_int32_t rx_count;

    os_int32_t tx_size;
    os_int32_t rx_size;

    os_tick_t  tx_timeout;
    os_tick_t  rx_timeout;
    
    void *user_data;

#ifdef OS_USING_IO_MULTIPLEXING
    os_list_node_t poll_list;
#endif

#ifdef OS_USING_DEVICE_NOTIFY
    os_list_node_t notify_list;
#endif
};

#ifdef OS_USING_DEVICE_NOTIFY
static os_list_node_t os_device_notify_list = OS_LIST_INIT(os_device_notify_list);

void os_device_notify(os_device_t *dev, os_ubase_t event, os_ubase_t args);
os_err_t     os_device_notify_register(os_device_t *dev, device_notify_callback callback, device_notify_filter filter);
os_err_t     os_device_notify_unregister(os_device_t *dev, device_notify_callback         callback, device_notify_filter filter);
#else
#define os_device_notify(...)
#define os_device_notify_register(...)
#define os_device_notify_unregister(...)
#endif

os_err_t     os_device_register(os_device_t *dev, const char *name);
os_err_t     os_device_unregister(os_device_t *dev);
os_device_t *os_device_find(const char *name);
os_err_t     os_device_open(os_device_t *dev);
os_err_t     os_device_close(os_device_t *dev);
os_size_t    os_device_read_block(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size);
os_size_t    os_device_read_nonblock(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size);
os_size_t    os_device_write_block(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size);
os_size_t    os_device_write_nonblock(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size);
os_err_t     os_device_control(os_device_t *dev, int cmd, void *arg);

void os_device_recv_notify(os_device_t *dev);
void os_device_send_notify(os_device_t *dev);

os_int32_t os_device_for_each(os_err_t (*func)(os_device_t *dev, void *data), void *data);

#ifdef __cplusplus
}
#endif

#endif

#endif


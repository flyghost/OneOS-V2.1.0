#include <os_types.h>
#include <os_errno.h>
#include <os_assert.h>
#include <serial/serial.h>
#include "misc_evt.h"
#include "wiresharkdump.h"

static os_err_t rx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    return 0;
}

static os_err_t tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
    if (wsk_frame_tx_get_rest_len(dev->tx_size) > 0)
    {
        wsk_frame_tx_next_slice(dev->tx_size);
    }
    else
    {
        wsk_frame_tx_done();
    }

    return 0;
}

os_device_t *wsk_trans_dev_init(char *dev_name)
{
    os_err_t ret;
    os_device_t *dev;

    dev = os_device_find(dev_name);
    OS_ASSERT(dev);

    /* nonblock callback */
    struct os_device_cb_info cb_info;

    cb_info.type = OS_DEVICE_CB_TYPE_TX;
    cb_info.cb = tx_done;
    os_device_control(dev, OS_DEVICE_CTRL_SET_CB, &cb_info);

    cb_info.type = OS_DEVICE_CB_TYPE_RX;
    cb_info.cb = rx_done;
    os_device_control(dev, OS_DEVICE_CTRL_SET_CB, &cb_info);

    /* dev config */
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_921600;
    ret = os_device_control(dev, OS_DEVICE_CTRL_CONFIG, &config);
    OS_ASSERT(ret == OS_EOK);

    /* open serial device with int rx, int tx flag */
    ret = os_device_open(dev);
    OS_ASSERT(ret == OS_EOK);

    return dev;
}

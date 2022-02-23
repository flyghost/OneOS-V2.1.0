#include <drv_cfg.h>
#include <shell.h>

#include <oneos_config.h>
#include <os_errno.h>
#include <os_task.h>
#include <os_event.h>

#include "hal/hal_uart.h"

#define EVENT_FLAG_RX (1 << 0)
#define EVENT_FLAG_TX (1 << 1)

#define HAL_UART_TX_BUF_MAX 0x10
#define HAL_UART_RX_BUF_MAX 0x20

struct hal_uart_cfg
{
    hal_uart_tx_char tx_func;
    hal_uart_rx_char rx_func;
    uint8_t tx_buf[HAL_UART_TX_BUF_MAX];
    uint8_t rx_buf[HAL_UART_RX_BUF_MAX];
    uint8_t tx_cnt;
};

os_device_t *hal_uart;

struct hal_uart_cfg hal_uart_cfg;

static os_event_t hal_uart_event;

static os_err_t drv_uart_tx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    /* perhaps there is more data need to be send */
    if (HAL_UART_TX_BUF_MAX == hal_uart_cfg.tx_cnt)
    {
        os_event_send(&hal_uart_event, EVENT_FLAG_TX);
    }
    return 0;
}

static os_err_t drv_uart_rx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    os_event_send(&hal_uart_event, EVENT_FLAG_RX);
    return 0;
}

void hal_uart_task(void *parameter)
{
    int i;
    os_uint32_t recv_event;
    uint8_t ch;
    int data;
    int count;
    uint8_t *buf;              

    while (1)
    {
        if (os_event_recv(&hal_uart_event, (EVENT_FLAG_RX | EVENT_FLAG_TX),
                          (OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR),
                          OS_WAIT_FOREVER,
                          &recv_event) == OS_EOK)
        {
            if (recv_event & EVENT_FLAG_RX)
            {
                buf = hal_uart_cfg.rx_buf;

                while (1)
                {
                    count = os_device_read_nonblock(hal_uart, 0, buf, HAL_UART_RX_BUF_MAX);
                    if (0 < count)
                    {
                        for (i = 0; i < count; i++)
                        {
                            ch = buf[i];
                            hal_uart_cfg.rx_func(NULL, ch);
                        }
                        if (HAL_UART_RX_BUF_MAX > count)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (recv_event & EVENT_FLAG_TX)
            {
                buf = hal_uart_cfg.tx_buf;

                for (i = 0; i < HAL_UART_TX_BUF_MAX; i++)
                {
                    data = hal_uart_cfg.tx_func(NULL);
                    if (data < 0)
                    {
                        break;
                    }
                    buf[i] = (uint8_t)data;
                }
                count = i;

                if (0 < count)
                {
                    hal_uart_cfg.tx_cnt = count;
                    os_device_write_nonblock(hal_uart, 0, buf, count);
                }
            }
        }
    }
}

void hal_uart_start_tx(int port)
{
    os_event_send(&hal_uart_event, EVENT_FLAG_TX);
}

int hal_uart_close(int uart)
{
    os_device_close(hal_uart);
    return 0; // success
}

int hal_uart_init_cbs(int port, hal_uart_tx_char tx_func, hal_uart_tx_done tx_done,
                      hal_uart_rx_char rx_func, void *arg)
{
    hal_uart_cfg.rx_func = rx_func;
    hal_uart_cfg.tx_func = tx_func;
    return 0;
}

int hal_uart_config(int port, int32_t baudrate, uint8_t databits, uint8_t stopbits,
                    enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ctl)
{
    struct serial_configure cfg = OS_SERIAL_CONFIG_DEFAULT;
    struct os_device_cb_info os_device_rx_cb_info = {
        .type = OS_DEVICE_CB_TYPE_RX,
        .cb = drv_uart_rx_done};
    struct os_device_cb_info os_device_tx_cb_info = {
        .type = OS_DEVICE_CB_TYPE_TX,
        .cb = drv_uart_tx_done};

    hal_uart = os_device_find("uart1");
    OS_ASSERT(hal_uart);

    os_device_control(hal_uart, OS_DEVICE_CTRL_SET_CB, &os_device_rx_cb_info);
    os_device_control(hal_uart, OS_DEVICE_CTRL_SET_CB, &os_device_tx_cb_info);

    /* open serial device with int rx, int tx flag */
    os_device_open(hal_uart);

    cfg.baud_rate = baudrate;
    cfg.data_bits = databits;
    cfg.stop_bits = stopbits - 1;
    cfg.parity = parity;
    os_device_control(hal_uart, OS_DEVICE_CTRL_CONFIG, &cfg);
    return 0;
}

int hal_uart_init(int uart, void *cfg)
{
    os_task_t *task;

    os_event_init(&hal_uart_event, "hal_uart_event");

    task = os_task_create("hal_uart",
                          hal_uart_task,
                          OS_NULL,
                          1024,
                          6);
    OS_ASSERT(OS_NULL != task);
    os_task_startup(task);

    return 0;
}
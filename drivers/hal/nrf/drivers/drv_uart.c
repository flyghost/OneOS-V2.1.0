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
 * @file        drv_uart.c
 *
 * @brief       This file implements uart driver for nrf5
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <bus/bus.h>
#include <dma/dma.h>
#include <board.h>
#include <os_memory.h>
#include "drv_uart.h"
#include "drv_gpio.h"

#define DMA_RX_MAX_SIZE 0xFF

enum{
    UARTE_RECV_IDLE,
    UARTE_RECV_SART,
    UARTE_RECV_ING,
    UARTE_RECV_FINISH,
};

enum { FALSE = 0, TRUE = 1 };
typedef enum {RESET = 0, SET = !RESET} FlagStatus;

typedef enum{
    UART_RX_NO_NEED_RESTART = 0,
    UART_RX_NEED_RESTART = 1
}UART_RX_NEED_RESTART_T;

typedef struct nrf5_uart
{
    struct os_serial_device serial_dev;
    struct nrf5_uart_info *huart;

    soft_dma_t sdma;
    os_uint32_t sdma_hard_size;
    
    const char *device_name;

    os_uint8_t *rx_buffer;
    os_size_t rx_head_temp;
    os_size_t rx_head_stamp;
    os_size_t rx_head;
    os_size_t rx_head_next;
    os_size_t rx_size;
    
    os_size_t tx_size;
    bool poll_send;
    int rx_state;
    bool isInit;
    
    os_list_node_t list;
}nrf5_uart_t;

static os_list_node_t nrf5_uart_list = OS_LIST_INIT(nrf5_uart_list);

static void _nrfx_uarte_rx(nrf5_uart_t *uart, uint32_t rx_size)
{
    nrfx_uarte_rx(&uart->huart->uart, uart->rx_buffer + uart->rx_head_next, rx_size);
    uart->rx_head_next += rx_size;
    if (uart->rx_head_next >= uart->rx_size)
        uart->rx_head_next = 0;
}

static void nrf5_uarte_evt_handler(nrfx_uarte_event_t const * p_event, void *p_context)
{
    os_base_t level;
    struct nrf5_uart *uart = (struct nrf5_uart *)p_context;

    level = os_irq_lock();

    switch (p_event->type)
    {
        case NRFX_UARTE_EVT_RX_DONE:
        {
            uart->rx_head += p_event->data.rxtx.bytes;

            OS_ASSERT(uart->rx_head <= uart->rx_size);

            if (uart->rx_head == uart->rx_size)
                uart->rx_head = 0;

            uint32_t rx_space_remain = uart->rx_size - uart->rx_head;
                
            uint32_t rx_size = min(rx_space_remain, DMA_RX_MAX_SIZE);

            _nrfx_uarte_rx(uart, rx_size);

            uart->huart->uart.p_reg->EVENTS_RXDRDY = 0;

            uart->rx_head_temp  = uart->rx_head;
            uart->rx_head_stamp = uart->rx_head;

            soft_dma_half_irq(&uart->sdma);
            
            break;
        }

        case NRFX_UARTE_EVT_ERROR:
            break;
        
        case NRFX_UARTE_EVT_TX_DONE:
            if(uart->poll_send != true)
            {
                os_hw_serial_isr_txdone(&uart->serial_dev);
            }
            uart->tx_size = 0;
            break;

        default:
            break;
    }

    os_irq_unlock(level);
}


/* dma rx mode */
static os_uint32_t nrf5_sdma_dma_get_index(soft_dma_t *dma)
{
    os_base_t level;
    
    nrf5_uart_t *uart = os_container_of(dma, nrf5_uart_t, sdma);

    if (uart->huart->uart.p_reg->EVENTS_RXDRDY)
    {
        uart->huart->uart.p_reg->EVENTS_RXDRDY = 0;
        
        if (++uart->rx_head_temp == uart->rx_size)
            uart->rx_head_temp = 0;
    }
    else if (uart->rx_head_stamp != uart->rx_head_temp)
    {
        level = os_irq_lock();
        
        nrfx_uarte_rx_abort(&uart->huart->uart);
        
        while(!nrf_uarte_event_check(uart->huart->uart.p_reg, NRF_UARTE_EVENT_ENDRX));

        uart->rx_head      += nrf_uarte_rx_amount_get(uart->huart->uart.p_reg);
        uart->rx_head_temp  = uart->rx_head;
        uart->rx_head_stamp = uart->rx_head;

        uart->rx_head_next  = uart->rx_head;
        if (uart->rx_head_next >= uart->rx_size)
            uart->rx_head_next = 0;
        
        uint32_t rx_space_remain = uart->rx_size - uart->rx_head;
        uint32_t rx_size = min(rx_space_remain, DMA_RX_MAX_SIZE);

        _nrfx_uarte_rx(uart, rx_size);

        rx_space_remain -= rx_size;

        if (rx_space_remain > 0)
        {
            _nrfx_uarte_rx(uart, min(rx_space_remain, DMA_RX_MAX_SIZE));
        }
        else
        {
            _nrfx_uarte_rx(uart, min(uart->rx_size, DMA_RX_MAX_SIZE));
        }

        os_irq_unlock(level);
    }
    
    return uart->rx_head_temp;
}

static os_err_t nrf5_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t nrf5_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    nrf5_uart_t *uart = os_container_of(dma, nrf5_uart_t, sdma);

    OS_ASSERT(size <= (DMA_RX_MAX_SIZE * 2));

    if (uart->rx_state != UARTE_RECV_IDLE)
        return OS_EOK;

    uart->rx_state = UARTE_RECV_SART;
    
    uart->rx_buffer = buff;
    uart->rx_size = size;
    uart->rx_head = 0;
    uart->rx_head_temp = 0;
    uart->rx_head_next = 0;

    _nrfx_uarte_rx(uart, size / 2);
    _nrfx_uarte_rx(uart, size / 2);

    return OS_EOK;
}

static os_uint32_t nrf5_sdma_dma_stop(soft_dma_t *dma)
{

    nrf5_uart_t *uart = os_container_of(dma, nrf5_uart_t, sdma);

    nrfx_uarte_rx_abort(&uart->huart->uart);
    while(!nrf_uarte_event_check(uart->huart->uart.p_reg, NRF_UARTE_EVENT_ENDRX));

    uart->rx_head += nrf_uarte_rx_amount_get(uart->huart->uart.p_reg);
   
    uart->rx_state = UARTE_RECV_IDLE;

    return uart->rx_head;
}

/* sdma callback */
static void nrf5_uart_sdma_callback(soft_dma_t *dma)
{
    nrf5_uart_t *uart = os_container_of(dma, nrf5_uart_t, sdma);
    
    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void nrf5_uart_sdma_init(struct nrf5_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;
    soft_dma_stop(dma);
    
    memset(&dma->hard_info, 0, sizeof(dma->hard_info));
    
    dma->hard_info.mode         = HARD_DMA_MODE_CIRCULAR;
    dma->hard_info.max_size     = DMA_RX_MAX_SIZE * 2;
    dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial_dev.config.baud_rate);

    dma->ops.get_index          = nrf5_sdma_dma_get_index;
    dma->ops.dma_init           = nrf5_sdma_dma_init;
    dma->ops.dma_start          = nrf5_sdma_dma_start;
    dma->ops.dma_stop           = nrf5_sdma_dma_stop;

    dma->cbs.dma_half_callback      = nrf5_uart_sdma_callback;
    dma->cbs.dma_full_callback      = nrf5_uart_sdma_callback;
    dma->cbs.dma_timeout_callback   = nrf5_uart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);

}

void nrf5_uart_recfg(void)
{
    nrfx_uarte_t uart0 = NRFX_UARTE_INSTANCE(0);
    struct nrf5_uart *uart;

    os_list_for_each_entry(uart, &nrf5_uart_list, struct nrf5_uart, list){__NOP();}
    
    nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG;

    config.baudrate = NRF_UARTE_BAUDRATE_115200;

    config.parity = NRF_UARTE_PARITY_EXCLUDED;


    config.hwfc = NRF_UARTE_HWFC_DISABLED;
    config.interrupt_priority = 4;
    config.pselrxd = 8;
    config.pseltxd = 6;
    config.p_context = (void *)uart;
    
    nrfx_uarte_init(&uart0,(nrfx_uarte_config_t const *)&config,nrf5_uarte_evt_handler);
}

static os_err_t nrf5_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG;
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);
    if(uart->huart == OS_NULL)
        return OS_ERROR;

    if(uart->isInit)
    {
        nrfx_uarte_uninit(&uart->huart->uart);
        uart->isInit = FALSE;
    }

    switch (cfg->baud_rate)
    {
        case BAUD_RATE_2400:
            config.baudrate = NRF_UARTE_BAUDRATE_2400;
            break;
        case BAUD_RATE_4800:
            config.baudrate = NRF_UARTE_BAUDRATE_4800;
            break;
        case BAUD_RATE_9600:
            config.baudrate = NRF_UARTE_BAUDRATE_9600;
            break;
        case BAUD_RATE_19200:
            config.baudrate = NRF_UARTE_BAUDRATE_19200;
            break;
        case BAUD_RATE_38400:
            config.baudrate = NRF_UARTE_BAUDRATE_38400;
            break;
        case BAUD_RATE_57600:
            config.baudrate = NRF_UARTE_BAUDRATE_57600;
            break;
        case BAUD_RATE_115200:
            config.baudrate = NRF_UARTE_BAUDRATE_115200;
            break;
        case BAUD_RATE_230400:
            config.baudrate = NRF_UARTE_BAUDRATE_230400;
            break;
        case BAUD_RATE_460800:
            config.baudrate = NRF_UARTE_BAUDRATE_460800;
            break;
        case BAUD_RATE_921600:
            config.baudrate = NRF_UARTE_BAUDRATE_921600;
            break;
        case BAUD_RATE_2000000:
        case BAUD_RATE_3000000:
            return -OS_EINVAL;
        default:
            config.baudrate = NRF_UARTE_BAUDRATE_115200;
            break;
    }

    if (cfg->parity == PARITY_NONE)
    {
        config.parity = NRF_UARTE_PARITY_EXCLUDED;
    }
    else
    {
        config.parity = NRF_UARTE_PARITY_INCLUDED;
    }
    
    config.hwfc = NRF_UARTE_HWFC_DISABLED;
    nrf_gpio_cfg_default(uart->huart->rx_pin);
    nrf_gpio_cfg_default(uart->huart->tx_pin);
    config.pselrxd = uart->huart->rx_pin;
    config.pseltxd = uart->huart->tx_pin;
    config.p_context = (void *)uart;
    
    nrfx_uarte_init(&uart->huart->uart,
                                     (nrfx_uarte_config_t const *)&config,
                                     nrf5_uarte_evt_handler);
    uart->isInit = TRUE;

    nrf5_uart_sdma_init(uart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static int nrf5_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int ret;
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);

    OS_ASSERT(uart != OS_NULL);
    
    uart->tx_size = size;
    uart->poll_send = false;
    ret = nrfx_uarte_tx(&uart->huart->uart,buff, size);

    return (ret == NRFX_SUCCESS ? size : 0);

}

static int nrf5_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int ret;
    int i;
    struct nrf5_uart *uart;
    uint8_t data;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);

    OS_ASSERT(uart != OS_NULL);
    uart->tx_size = 0;
    uart->poll_send = true;

    for (i = 0; i < size; i++)
    {
        data = buff[i];

        os_base_t level = os_irq_lock();

        nrfx_uarte_tx(&uart->huart->uart, &data, 1);

        while (!nrf_uarte_event_check(uart->huart->uart.p_reg, NRF_UARTE_EVENT_ENDTX));

        nrfx_uarte_tx_abort(&uart->huart->uart);

        os_irq_unlock(level);
    }

    ret = size;
    
    uart->tx_size = 0;

    return ret;
}

static os_err_t nrf5_uart_deinit(struct os_serial_device *serial)
{
    struct nrf5_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct nrf5_uart, serial_dev);
    if(uart->huart == OS_NULL)
        return OS_ERROR;

    /* rx */
    if(uart->isInit)
    {
        nrfx_uarte_uninit(&uart->huart->uart);
        uart->isInit = FALSE;

        soft_dma_stop(&uart->sdma);
    }

    /* tx */
    nrfx_uarte_tx_abort(&uart->huart->uart);
    
    return OS_EOK;
}

static const struct os_uart_ops nrf5_uart_ops = {
    .init       = nrf5_uart_init,
    .deinit     = nrf5_uart_deinit,

    .start_send = nrf5_uart_start_send,
    .poll_send  = nrf5_uart_poll_send,   
};

int nrf5_uart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;

    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    struct nrf5_uart_info *uart_info = (struct nrf5_uart_info *)dev->info;
    struct nrf5_uart *uart = os_calloc(1, sizeof(struct nrf5_uart));

    OS_ASSERT(uart);
    uart->huart = uart_info;
    uart->isInit = FALSE;
    uart->rx_head = 0; 
    uart->rx_state = UARTE_RECV_IDLE;

    struct os_serial_device *serial = &uart->serial_dev;

    serial->ops    = &nrf5_uart_ops;
    serial->config = config;

    level = os_irq_lock();
    os_list_add_tail(&nrf5_uart_list, &uart->list);
    os_irq_unlock(level);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;

}
static struct nrf5_uart_info *console_uart = OS_NULL;

void __os_hw_console_output(char *str)
{
    if (console_uart == OS_NULL)
        return;

#if 0
    while (*str)
    {
        nrfx_uarte_tx(&console_uart->uart, (os_uint8_t *)str, 1);
        str++;
    }
#endif
}

OS_DRIVER_INFO nrf5_uart_driver = {
    .name   = "uart_Type",
    .probe  = nrf5_uart_probe,
};

OS_DRIVER_DEFINE(nrf5_uart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

static int nrf5_uart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if(!strcmp(dev->name,OS_CONSOLE_DEVICE_NAME))
        console_uart = (struct nrf5_uart_info *)dev->info;
    return OS_EOK;
}

OS_DRIVER_INFO nrf5_usart_early_driver = {
    .name   = "uart_Type",
    .probe  = nrf5_uart_early_probe,
};

OS_DRIVER_DEFINE(nrf5_usart_early_driver, CORE, OS_INIT_SUBLEVEL_LOW);

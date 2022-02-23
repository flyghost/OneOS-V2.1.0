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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <string.h>
#include <os_memory.h>
#include "drv_usart.h"
#include "drv_dma_irq.h"

static os_list_node_t hc32_uart_list = OS_LIST_INIT(hc32_uart_list);

static void uart_isr(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    hc_usart_t *uart = os_container_of(serial, hc_usart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    if(Uart_GetStatus(uart->info->idx, UartRC) != 0)
    {
        Uart_ClrStatus(uart->info->idx, UartRC);
        uart->rx_buff[uart->rx_index++] = Uart_ReceiveData(uart->info->idx);

        if (uart->rx_index == (uart->rx_size / 2))
        {
            soft_dma_half_irq(&uart->sdma);
        }

        if (uart->rx_index == uart->rx_size)
        {
            uart->rx_index = 0;
            soft_dma_full_irq(&uart->sdma);
        }
    }
}

void uart_irq(int device_num)
{
    struct hc_uart *uart;

    os_list_for_each_entry(uart, &hc32_uart_list, struct hc_uart, list)
    {
        if (uart->info->uart_device == device_num)
        {
            uart_isr(&uart->serial_dev);
            break;
        }
    }
}

void Uart0_IRQHandler(void)
{
    uart_irq(0);
}

void Uart1_IRQHandler(void)
{
    uart_irq(1);
}

void Uart2_IRQHandler(void)
{
    uart_irq(2);
}

void Uart3_IRQHandler(void)
{
    uart_irq(3);
}

int cnt_data = 0;

static void uart_dma_isr(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    hc_usart_t *uart = os_container_of(serial, hc_usart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    if (uart->info->dma_channel == DmaCh0)
    {
        cnt_data = uart->rx_size - M0P_DMAC->CONFA0_f.TC;
    }
    else
    {
        cnt_data = uart->rx_size - M0P_DMAC->CONFA1_f.TC;
    }
    Dma_ClrStat(uart->info->dma_channel);

    soft_dma_half_irq(&uart->sdma);

    Dma_EnableChannel(uart->info->dma_channel);
    Dma_EnableChannelIrq(uart->info->dma_channel);
}

void uart_dma_irq(en_dma_channel_t dma_channel)
{
    struct hc_uart *uart;

    os_list_for_each_entry(uart, &hc32_uart_list, struct hc_uart, list)
    {
        if (uart->info->dma_channel == dma_channel)
        {
            uart_dma_isr(&uart->serial_dev);
            break;
        }
    }
}

/* interrupt rx mode */
static os_uint32_t hc_usart_sdma_int_get_index(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    return uart->rx_index;
}

static os_err_t hc_usart_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    uart->rx_buff  = buff;
    uart->rx_index = 0;
    uart->rx_size  = size;

    Uart_ClrStatus(uart->info->idx, UartRC);
    Uart_EnableIrq(uart->info->idx, UartRxIrq);

    return OS_EOK;
}

static os_uint32_t hc_usart_sdma_int_stop(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    Uart_DisableIrq(uart->info->idx, UartRxIrq);

    return uart->rx_index;
}

/* dma rx mode */
static os_uint32_t hc_usart_sdma_dma_get_index(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    if (uart->info->dma_channel == DmaCh0)
    {
        return uart->sdma_hard_size - M0P_DMAC->CONFA0_f.TC - 1;
    }
    else if (uart->info->dma_channel == DmaCh1)
    {
        return uart->sdma_hard_size - M0P_DMAC->CONFA1_f.TC - 1;
    }

    return 0;
}

static os_err_t hc_usart_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t hc_usart_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    uart->sdma_hard_size = size;

    stc_dma_cfg_t stcDmaCfg;
    DDL_ZERO_STRUCT(stcDmaCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralDma, TRUE);

    stcDmaCfg.u32SrcAddress = (os_uint32_t)uart->info->idx;
    stcDmaCfg.enRequestNum = uart->info->req_num;
    stcDmaCfg.u32DstAddress = (uint32_t)buff;
    stcDmaCfg.enSrcAddrReloadCtl = DmaMskSrcAddrReloadEnable;
    stcDmaCfg.enSrcBcTcReloadCtl = DmaMskBcTcReloadEnable;
    stcDmaCfg.enDestAddrReloadCtl = DmaMskDstAddrReloadEnable;
    stcDmaCfg.enTransferMode = DmaMskContinuousTransfer;
    stcDmaCfg.enDstAddrMode = DmaMskDstAddrInc;
    stcDmaCfg.enSrcAddrMode = DmaMskSrcAddrFix;
    stcDmaCfg.u16BlockSize = 1;
    stcDmaCfg.u16TransferCnt = size;
    stcDmaCfg.enMode = DmaMskBlock;
    stcDmaCfg.enTransferWidth = DmaMsk8Bit;
    stcDmaCfg.enPriority = DmaMskPriorityFix;

    Dma_Enable();
    Dma_InitChannel(uart->info->dma_channel, &stcDmaCfg);

    EnableNvic(DMAC_IRQn, IrqLevel3, TRUE);

    Dma_ClrStat(uart->info->dma_channel);
    Dma_EnableChannel(uart->info->dma_channel);
    Dma_EnableChannelIrq(uart->info->dma_channel);

    return OS_EOK;
}

static os_uint32_t hc_usart_sdma_dma_stop(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    Dma_DisableChannel(uart->info->dma_channel);
    Dma_DisableChannelIrq(uart->info->dma_channel);

    return hc_usart_sdma_dma_get_index(dma);
}

/* sdma callback */
static void hc_usart_sdma_callback(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void hc_usart_sdma_init(struct hc_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = HARD_DMA_FLAG_FULL_IRQ;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial_dev.config.baud_rate);

    if (uart->info->dma_support == 0)
    {
        dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;

        dma->ops.get_index          = hc_usart_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = hc_usart_sdma_int_start;
        dma->ops.dma_stop           = hc_usart_sdma_int_stop;
    }
    else
    {
        dma->hard_info.mode         = HARD_DMA_MODE_CIRCULAR;

        dma->ops.get_index          = hc_usart_sdma_dma_get_index;
        dma->ops.dma_init           = hc_usart_sdma_dma_init;
        dma->ops.dma_start          = hc_usart_sdma_dma_start;
        dma->ops.dma_stop           = hc_usart_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = hc_usart_sdma_callback;
    dma->cbs.dma_full_callback      = hc_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = hc_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static void hc_uart_gpio_init(struct hc32_usart_info *info)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    stcGpioCfg.enDir =  GpioDirOut;
    Gpio_Init(info->tx_port, info->tx_pin, &stcGpioCfg);       /* TX */
    stcGpioCfg.enDir =  GpioDirIn;
    Gpio_Init(info->rx_port, info->rx_pin, &stcGpioCfg);       /* RX */
    Gpio_SetAfMode(info->tx_port, info->tx_pin, info->tx_af);
    Gpio_SetAfMode(info->rx_port, info->rx_pin, info->rx_af);
}

static os_err_t hc_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    hc_usart_t *uart = os_container_of(serial, hc_usart_t, serial_dev);

    stc_uart_cfg_t  stcCfg;
    DDL_ZERO_STRUCT(stcCfg);

    if (uart->info->uart_device == 0)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralUart0, TRUE);
    }

    if (uart->info->uart_device == 1)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralUart1, TRUE);
    }

    if (uart->info->uart_device == 2)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralUart2, TRUE);
    }

    if (uart->info->uart_device == 3)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralUart3, TRUE);
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        stcCfg.enStopBit = UartMsk1bit;
        break;
    case STOP_BITS_2:
        stcCfg.enStopBit = UartMsk2bit;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        stcCfg.enRunMode = UartMskMode1;
        break;
    case PARITY_ODD:
        stcCfg.enMmdorCk = UartMskOdd;
        stcCfg.enRunMode = UartMskMode3;
        break;
    case PARITY_EVEN:
        stcCfg.enMmdorCk = UartMskEven;
        stcCfg.enRunMode = UartMskMode3;
        break;
    default:
        return OS_EINVAL;
    }

    stcCfg.stcBaud.u32Pclk = Sysctrl_GetPClkFreq();
    stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;
    stcCfg.stcBaud.u32Baud = cfg->baud_rate;

    Uart_Init(uart->info->idx, &stcCfg);

    Uart_ClrStatus(uart->info->idx, UartRC);
    Uart_ClrStatus(uart->info->idx, UartTC);

    if (uart->info->dma_support == 1)
    {
        Uart_EnableFunc(uart->info->idx, UartDmaRxFunc);
    }
    else
    {
        EnableNvic(uart->info->irqn, IrqLevel3, TRUE);
        Uart_EnableIrq(uart->info->idx, UartRxIrq);
    }

    hc_usart_sdma_init(uart, &uart->serial_dev.rx_fifo->ring);

    return OS_EOK;
}

static os_err_t hc_uart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    hc_usart_t *uart = os_container_of(serial, hc_usart_t, serial_dev);

    if (uart->info->dma_support == 0)
    {
        Uart_DisableIrq(uart->info->idx, UartRxIrq);
    }
    else
    {
        soft_dma_stop(&uart->sdma);

        Dma_DisableChannel(uart->info->dma_channel);
        Dma_DisableChannelIrq(uart->info->dma_channel);
    }

    return OS_EOK;
}

static int hc_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);
    hc_usart_t *uart = os_container_of(serial, hc_usart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();

        Uart_SendDataPoll(uart->info->idx, buff[i]);

        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops hc_uart_ops =
{
    .init       = hc_uart_init,
    .deinit     = hc_uart_deinit,

    .poll_send  = hc_uart_poll_send,
};

static int hc32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct hc_uart *usart;

    usart = os_calloc(1, sizeof(struct hc_uart));
    OS_ASSERT(usart);

    usart->info = (struct hc32_usart_info *)dev->info;
    struct os_serial_device *serial = &usart->serial_dev;

    level = os_irq_lock();
    os_list_add_tail(&hc32_uart_list, &usart->list);
    os_irq_unlock(level);

    uart_dma_channel_get(usart);

    hc_uart_gpio_init(usart->info);

    if (SystemCoreClock >= 16000000)
    {
        config.baud_rate = BAUD_RATE_115200;
    }
    else
    {
        config.baud_rate = BAUD_RATE_9600;
    }

    serial->ops    = &hc_uart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, usart);

    return 0;
}

OS_DRIVER_INFO hc32_usart_driver = {
    .name   = "Usart_Type",
    .probe  = hc32_usart_probe,
};

OS_DRIVER_DEFINE(hc32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

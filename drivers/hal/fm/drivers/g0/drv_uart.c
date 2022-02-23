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
 * @brief       This file implements usart driver for fm33
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_uart.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.uart"
#include <drv_log.h>

#if defined (LPUART0_USE_DMA)
extern void lpuart_dma_recv_complete_handler(LPUART_Type *huart);
#endif

#ifdef OS_USING_SERIAL

static os_list_node_t fm33_uart_list = OS_LIST_INIT(fm33_uart_list);

struct fm33_uart
{
    struct os_serial_device serial;
    struct fm33_usart_info *info;

    soft_dma_t  sdma;

    os_uint8_t *rx_buff;
    os_uint32_t rx_size;
    os_uint32_t rx_index;

    const os_uint8_t *tx_buff;
    os_uint32_t tx_size;
    os_uint32_t tx_index;

    os_list_node_t list;
};

#if defined(BSP_USING_UART)
static void uart_isr(struct os_serial_device *serial)
{
    struct fm33_uart *uart = os_container_of(serial, struct fm33_uart, serial);

    if((ENABLE == UART_UARTIE_RxTxIE_GetableEx(uart->info->instance, RxInt))
            && (SET == UART_UARTIF_RxTxIF_ChkEx(uart->info->instance, RxInt)))
    {
        uart->rx_buff[uart->rx_index++] = UARTx_RXREG_Read(uart->info->instance);

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

    if((ENABLE == UART_UARTIE_RxTxIE_GetableEx(uart->info->instance, TxInt))
            && (SET == UART_UARTIF_RxTxIF_ChkEx(uart->info->instance, TxInt)))
    {
        UART_UARTIF_RxTxIF_ClrEx(uart->info->instance);

        if(uart->tx_size)
        {
            if(uart->tx_index < uart->tx_size)
            {
                UARTx_TXREG_Write(uart->info->instance, uart->tx_buff[uart->tx_index++]);
            }
            else
            {
                UART_UARTIE_RxTxIE_SetableEx(uart->info->instance, TxInt, DISABLE);
                os_hw_serial_isr_txdone((struct os_serial_device *)uart);
            }
        }
    }
}

static void usart_rx_irq_callback(os_uint32_t  usart_index)
{
    struct fm33_uart *uart;

    os_list_for_each_entry(uart, &fm33_uart_list, struct fm33_uart, list)
    {
        if (uart->info->index == usart_index)
        {
            uart_isr(&uart->serial);
            break;
        }
    }
}
#endif

#if defined(BSP_USING_UART0)
void UART0_IRQHandler(void)
{
    usart_rx_irq_callback(0);
}
#endif

#if defined(BSP_USING_UART1)
void UART1_IRQHandler(void)
{
    usart_rx_irq_callback(1);
}
#endif

#if defined(BSP_USING_UART2)
void UART2_IRQHandler(void)
{
    usart_rx_irq_callback(2);
}
#endif

#if defined(BSP_USING_UART3)
void UART3_IRQHandler(void)
{
    usart_rx_irq_callback(3);
}
#endif

#if defined(BSP_USING_UART4)
void UART4_IRQHandler(void)
{
    usart_rx_irq_callback(4);
}
#endif

#if defined(BSP_USING_UART5)
void UART5_IRQHandler(void)
{
    usart_rx_irq_callback(5);
}
#endif

static os_uint32_t fm33_uart_dma_ram_addr(struct fm33_uart *uart)
{
    DMA_CH_Type  CHx;
    os_uint32_t  addr;

    CHx = uart->info->dma_param.CHx;
    addr = *(uint32_t *)(0x40000404 + 4 + CHx * 8);

    return addr;
}

#if defined(BSP_USING_UART)
static void uart_dma_recv_complete_handler(UARTx_Type *huart)
{
    struct fm33_uart     *uart;

    os_list_for_each_entry(uart, &fm33_uart_list, struct fm33_uart, list)
    {
        if (uart->info->instance == huart)
        {
            soft_dma_half_irq(&uart->sdma);

            break;
        }
    }
}
#endif

void DMA_IRQHandler(void)
{
    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH0))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH0);
    }

    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH1))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH1);

#if defined(UART0_USE_DMA)
        uart_dma_recv_complete_handler(UART0);
#endif

#if defined(UART3_USE_DMA)
        uart_dma_recv_complete_handler(UART3);
#endif
    }

    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH2))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH2);
    }

    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH3))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH3);

#if defined(UART1_USE_DMA)
        uart_dma_recv_complete_handler(UART1);
#endif

#if defined(UART4_USE_DMA)
        uart_dma_recv_complete_handler(UART4);
#endif

    }

    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH4))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH4);
    }

    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH5))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH5);

#if defined(UART2_USE_DMA)
        uart_dma_recv_complete_handler(UART2);
#endif

#if defined(UART5_USE_DMA)
        uart_dma_recv_complete_handler(UART5);
#endif

#if defined(LPUART0_USE_DMA)
        lpuart_dma_recv_complete_handler(LPUART);
#endif
    }

    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH6))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH6);
    }

    if(SET == DMA_CHSTATUS_DMACHxFT_Chk(DMA_CH7))
    {
        DMA_CHSTATUS_DMACHxFT_Clr(DMA_CH7);
    }
}

static os_uint32_t fm33_sdma_int_get_index(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    return uart->rx_index;
}

static os_err_t fm33_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_uart *uart = os_container_of(dma, struct fm33_uart, sdma);

    uart->rx_buff  = buff;
    uart->rx_size  = size;
    uart->rx_index = 0;

    UART_UARTIE_RxTxIE_SetableEx(uart->info->instance, RxInt, ENABLE);
    NVIC_EnableIRQ(uart->info->irqn);

    return OS_EOK;
}

static os_uint32_t fm33_sdma_int_stop(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    UART_UARTIE_RxTxIE_SetableEx(uart->info->instance, RxInt, DISABLE);

    return uart->rx_index;
}

static os_uint32_t fm33_sdma_dma_get_index(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    return fm33_uart_dma_ram_addr(uart) - uart->info->dma_addr_start;
}

static os_err_t fm33_sdma_dma_init(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    RCC_PERCLK_SetableEx(DMACLK, ENABLE);

    uart->info->dma_param.CHxPRI  = DMA_CHxCTRL_PRI_MEDIUM;
    uart->info->dma_param.CHxINC  = DMA_CHxCTRL_INC_INC;
    uart->info->dma_param.CHxFTIE = ENABLE;
    uart->info->dma_param.CHxHTIE = DISABLE;
    uart->info->dma_param.CHxEN   = ENABLE;

    DMA_GLOBALCTRL_DMAEN_Setable(ENABLE);

    return OS_EOK;
}

static os_err_t fm33_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_uart *uart = os_container_of(dma, struct fm33_uart, sdma);

    uart->rx_buff  = buff;
    uart->rx_size  = size;

    uart->info->dma_param.CHxTSIZE = uart->rx_size - 1;
    uart->info->dma_param.CHxRAMAD = (uint32_t)(uart->rx_buff);

    DMA_Init(&uart->info->dma_param);
    uart->info->dma_addr_start = fm33_uart_dma_ram_addr(uart);

    UART_UARTIE_RxTxIE_SetableEx(uart->info->instance, RxInt, DISABLE);

    DMA_CHSTATUS_DMACHxFT_Clr(uart->info->dma_param.CHx);

    NVIC_EnableIRQ(DMA_IRQn);

    return OS_EOK;
}
static os_uint32_t fm33_sdma_dma_stop(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    OS_ASSERT(uart->info->dma_enable != OS_FALSE);

    DMA_CHxCTRL_EN_Setable(uart->info->dma_param.CHx, DISABLE);

    return fm33_uart_dma_ram_addr(uart) - uart->info->dma_addr_start;
}

static void fm33_usart_sdma_callback(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void fm33_usart_sdma_init(struct fm33_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);

    if (uart->info->dma_enable == OS_FALSE)
    {
        dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;

        dma->ops.get_index          = fm33_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = fm33_sdma_int_start;
        dma->ops.dma_stop           = fm33_sdma_int_stop;
    }
    else
    {
        dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;

        dma->ops.get_index          = fm33_sdma_dma_get_index;
        dma->ops.dma_init           = fm33_sdma_dma_init;
        dma->ops.dma_start          = fm33_sdma_dma_start;
        dma->ops.dma_stop           = fm33_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = fm33_usart_sdma_callback;
    dma->cbs.dma_full_callback      = fm33_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = fm33_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static os_err_t fm33_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct fm33_uart      *uart;
    RCC_ClocksType         RCC_Clocks;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct fm33_uart, serial);

    RCC_PERCLK_SetableEx(UARTCOMCLK, ENABLE);
    RCC_PERCLK_SetableEx(uart->info->clk_src, ENABLE);

    AltFunIO(uart->info->port, uart->info->tx_pin, 0);
    AltFunIO(uart->info->port, uart->info->rx_pin, 0);

    uart->info->uart_param.BaudRate = cfg->baud_rate;

    switch(cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart->info->uart_param.StopBit = OneBit;
        break;
    case STOP_BITS_2:
        uart->info->uart_param.StopBit = TwoBit;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        uart->info->uart_param.ParityBit = NONE;
        break;
    case PARITY_ODD:
        uart->info->uart_param.ParityBit = ODD;
        break;
    case PARITY_EVEN:
        uart->info->uart_param.ParityBit = EVEN;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->data_bits)
    {
    case DATA_BITS_7:
        uart->info->uart_param.DataBit = Seven7Bit;
        break;
    case DATA_BITS_8:
        uart->info->uart_param.DataBit = Eight8Bit;
        break;
    case DATA_BITS_9:
        uart->info->uart_param.DataBit = Nine9Bit;
        break;
    default:
        return OS_EINVAL;
    }

    RCC_GetClocksFreq(&RCC_Clocks);
    UART_SInit(uart->info->instance, &(uart->info->uart_param), RCC_Clocks.APBCLK_Frequency);

    UARTx_RXSTA_RXEN_Setable(uart->info->instance, ENABLE);
    UARTx_TXSTA_TXEN_Setable(uart->info->instance, ENABLE);

    fm33_usart_sdma_init(uart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t fm33_uart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    struct fm33_uart *uart = os_container_of(serial, struct fm33_uart, serial);

    if (uart->info->dma_enable != OS_FALSE)
    {
        soft_dma_stop(&uart->sdma);
    }

    UART_UARTIE_RxTxIE_SetableEx(uart->info->instance, RxInt, DISABLE);

    return OS_EOK;
}

static int fm33_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct fm33_uart     *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct fm33_uart, serial);

    uart->tx_index   = 0;
    uart->tx_size  = size;
    uart->tx_buff  = buff;

    UART_UARTIF_RxTxIF_ClrEx(uart->info->instance);
    UART_UARTIE_RxTxIE_SetableEx(uart->info->instance, TxInt, ENABLE);
    NVIC_EnableIRQ(uart->info->irqn);
    UARTx_TXREG_Write(uart->info->instance, uart->tx_buff[uart->tx_index++]);

    return size;
}

static int fm33_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;
    struct fm33_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct fm33_uart, serial);

    UART_UARTIE_RxTxIE_SetableEx(uart->info->instance, TxInt, DISABLE);
    for(i = 0; i < size; i++)
    {
        level = os_irq_lock();
        UARTx_TXREG_Write(uart->info->instance, buff[i]);
        while(SET == UARTx_TXBUFSTA_TXFF_Chk(uart->info->instance));
        os_irq_unlock(level);
    }
    return size;
}

static const struct os_uart_ops fm33_uart_ops = {
    .init         = fm33_uart_init,
    .deinit       = fm33_uart_deinit,

    .start_send   = fm33_uart_start_send,

    .poll_send    = fm33_uart_poll_send,
};

/**
 ***********************************************************************************************************************
 * @brief           fm33_usart_probe:register uart.
 *
 * @param[in]       none
 *
 * @return          Return init result.
 * @retval          OS_EOK       init success.
 * @retval          Others       init failed.
 ***********************************************************************************************************************
 */

static int fm33_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct fm33_uart *usart;

    usart = os_calloc(1, sizeof(struct fm33_uart));
    OS_ASSERT(usart);

    usart->info = (struct fm33_usart_info *)dev->info;
    struct os_serial_device *serial = &usart->serial;

    level = os_irq_lock();
    os_list_add_tail(&fm33_uart_list, &usart->list);
    os_irq_unlock(level);

    serial->ops    = &fm33_uart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, usart);

    return 0;
}

#else   /* OS_USING_SERIAL */

static UART_HandleTypeDef *console_uart;

void os_hw_console_output(const char *str)
{
    if (console_uart == OS_NULL)
        return;

    while (*str)
    {
        if (*str == '\n')
        {
            UARTx_TXREG_Write(console_uart, '\r');
        }

        UARTx_TXREG_Write(console_uart, *str);
        str++;
    }
}

static int fm33_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    console_uart = (UART_HandleTypeDef *)dev->info;
    return 0;
}

#endif

OS_DRIVER_INFO fm33_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = fm33_usart_probe,
};

OS_DRIVER_DEFINE(fm33_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

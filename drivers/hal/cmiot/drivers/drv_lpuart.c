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
 * @file        drv_lpuart.c
 *
 * @brief       This file implements lpuart driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <string.h>
#include "drv_lpuart.h"
#include "board.h"

static os_list_node_t cm32_lpuart_list = OS_LIST_INIT(cm32_lpuart_list);

/* lpuart driver */
typedef struct cm_lpuart
{
    struct os_serial_device serial_dev;
    struct cm32_lpuart_info *info;
    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;

    uint8_t  *rx_buff;
    uint32_t  rx_index;
    uint32_t  rx_size;
    os_list_node_t list;
} cm_lpuart_t;

static void lpuart_isr(struct os_serial_device *serial)
{
    struct cm_lpuart *lpuart;

    OS_ASSERT(serial != OS_NULL);
    lpuart = (struct cm_lpuart *)serial->parent.user_data;

    OS_ASSERT(lpuart != OS_NULL);

    if(LPUART_GetFlagStatus(LPUART_FLAG_FIFO_NE) != RESET)
    {
        lpuart->rx_buff[lpuart->rx_index++] = LPUART_ReceiveData();

        if (lpuart->rx_index == (lpuart->rx_size / 2))
        {
            soft_dma_half_irq(&lpuart->sdma);
        }

        if (lpuart->rx_index == lpuart->rx_size)
        {
            lpuart->rx_index = 0;
            soft_dma_full_irq(&lpuart->sdma);
        }
    }
}

void LPUART_IRQHandler(void)
{
    struct cm_lpuart *lpuart;

    os_list_for_each_entry(lpuart, &cm32_lpuart_list, struct cm_lpuart, list)
    {
        lpuart_isr(&lpuart->serial_dev);
        break;
    }
}

/* interrupt rx mode */
static os_uint32_t cm_lpuart_sdma_int_get_index(soft_dma_t *dma)
{
    cm_lpuart_t *lpuart = os_container_of(dma, cm_lpuart_t, sdma);

    return lpuart->rx_index;
}

static os_err_t cm_lpuart_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    cm_lpuart_t *lpuart = os_container_of(dma, cm_lpuart_t, sdma);

    lpuart->rx_buff  = buff;
    lpuart->rx_index = 0;
    lpuart->rx_size  = size;

    LPUART_ConfigInt(LPUART_INT_FIFO_NE, ENABLE);

    return OS_EOK;
}

static os_uint32_t cm_lpuart_sdma_int_stop(soft_dma_t *dma)
{
    cm_lpuart_t *lpuart = os_container_of(dma, cm_lpuart_t, sdma);

    LPUART_ConfigInt(LPUART_INT_FIFO_NE, DISABLE);

    return lpuart->rx_index;
}

#ifdef LPUART0_USING_DMA
/* dma rx mode */
static os_uint32_t cm_lpuart_sdma_dma_get_index(soft_dma_t *dma)
{
    cm_lpuart_t *lpuart = os_container_of(dma, cm_lpuart_t, sdma);

    if (lpuart->info->dma_support == 1)
    {
        cm_lpuart_t *lpuart = os_container_of(dma, cm_lpuart_t, sdma);

        return lpuart->sdma_hard_size - DMA_GetCurrDataCounter(DMA_CH1);
    }

    return 0;
}

static os_err_t cm_lpuart_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t cm_lpuart_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    cm_lpuart_t *lpuart = os_container_of(dma, cm_lpuart_t, sdma);

    lpuart->sdma_hard_size = size;

    DMA_InitType DMA_InitStructure;

    /* DMA clock enable */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);

    DMA_DeInit(DMA_CH1);
    DMA_InitStructure.PeriphAddr     = LPUART_BASE + 0x10;
    DMA_InitStructure.MemAddr        = (uint32_t)buff;
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_SRC;
    DMA_InitStructure.BufSize        = size;
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode   = DMA_MODE_CIRCULAR;
    DMA_InitStructure.Priority       = DMA_PRIORITY_VERY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
    DMA_Init(DMA_CH1, &DMA_InitStructure);
    DMA_RequestRemap(DMA_REMAP_LPUART_RX, DMA, DMA_CH1, ENABLE);

    LPUART_EnableDMA(LPUART_DMAREQ_RX, ENABLE);
    DMA_EnableChannel(DMA_CH1, ENABLE);

    return OS_EOK;
}

static os_uint32_t cm_lpuart_sdma_dma_stop(soft_dma_t *dma)
{
    LPUART_EnableDMA(LPUART_DMAREQ_RX, DISABLE);
    DMA_EnableChannel(DMA_CH1, DISABLE);

    return cm_lpuart_sdma_dma_get_index(dma);
}
#endif

/* sdma callback */
static void cm_lpuart_sdma_callback(soft_dma_t *dma)
{
    cm_lpuart_t *lpuart = os_container_of(dma, cm_lpuart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)lpuart);
}

static void cm_lpuart_sdma_init(struct cm_lpuart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial_dev.config.baud_rate);

    if (uart->info->dma_support == 0)
    {
        //dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;

        dma->ops.get_index          = cm_lpuart_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = cm_lpuart_sdma_int_start;
        dma->ops.dma_stop           = cm_lpuart_sdma_int_stop;
    }
#ifdef LPUART0_USING_DMA
    else
    {
        dma->hard_info.flag        |= HARD_DMA_FLAG_FULL_IRQ;
        dma->ops.get_index          = cm_lpuart_sdma_dma_get_index;
        dma->ops.dma_init           = cm_lpuart_sdma_dma_init;
        dma->ops.dma_start          = cm_lpuart_sdma_dma_start;
        dma->ops.dma_stop           = cm_lpuart_sdma_dma_stop;
    }
#endif

    dma->cbs.dma_half_callback      = cm_lpuart_sdma_callback;
    dma->cbs.dma_full_callback      = cm_lpuart_sdma_callback;
    dma->cbs.dma_timeout_callback   = cm_lpuart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

void GPIO_Configuration_lpuart(struct os_serial_device *serial)
{
    struct cm_lpuart *lpuart;

    OS_ASSERT(serial != OS_NULL);
    lpuart = (struct cm_lpuart *)serial->parent.user_data;

    OS_ASSERT(lpuart != OS_NULL);

    GPIO_InitType GPIO_InitStructure;

    /* Initialize GPIO_InitStructure */
    GPIO_InitStruct(&GPIO_InitStructure);

    /* Configure LPUARTx Tx as alternate function push-pull */
    GPIO_InitStructure.Pin            = lpuart->info->tx_pin;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = lpuart->info->tx_af;
    GPIO_InitPeripheral(lpuart->info->tx_port, &GPIO_InitStructure);

    /* Configure LPUARTx Rx as alternate function push-pull and pull-up */
    GPIO_InitStructure.Pin            = lpuart->info->rx_pin;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = lpuart->info->rx_af;
    GPIO_InitPeripheral(lpuart->info->rx_port, &GPIO_InitStructure);
}

void RCC_Configuration_CLK(uint32_t LPUART_CLK_SRC)
{
    switch(LPUART_CLK_SRC)
    {
    case RCC_LPUARTCLK_SRC_LSE:
    {
        RCC_EnableAPB1PeriphClk (RCC_APB1_PERIPH_PWR, ENABLE);
        PWR->CTRL1 |= PWR_CTRL1_DRBP;
        /* Configures the External Low Speed oscillator (LSE) */
        RCC_ConfigLse(RCC_LSE_ENABLE);
        while (RCC_GetFlagStatus(RCC_LDCTRL_FLAG_LSERD) == RESET)
        {
        }
        /* Specifies the LPUART clock source, LSE selected as LPUART clock */
        RCC_ConfigLPUARTClk(RCC_LPUARTCLK_SRC_LSE);
    }
    break;
    case RCC_LPUARTCLK_SRC_HSI:
    {
        /* Configures the High Speed Internal RC clock (HSI) */
        RCC_ConfigHsi(RCC_HSI_ENABLE);
        while (RCC_GetFlagStatus(RCC_CTRL_FLAG_HSIRDF) == RESET)
        {
        }
        /* Specifies the LPUART clock source, HSI selected as LPUART clock */
        RCC_ConfigLPUARTClk(RCC_LPUARTCLK_SRC_HSI);
    }
    break;
    case RCC_LPUARTCLK_SRC_SYSCLK:
    {
        /* Specifies the LPUART clock source, SYSCLK selected as LPUART clock */
        RCC_ConfigLPUARTClk(RCC_LPUARTCLK_SRC_SYSCLK);
    }
    break;
    default:
    {
        /* Specifies the LPUART clock source, APB1 selected as LPUART clock */
        RCC_ConfigLPUARTClk(RCC_LPUARTCLK_SRC_APB1);
    }
    break;
    }

    /* Enable LPUART and USARTz Clock */
    RCC_EnableRETPeriphClk(RCC_RET_PERIPH_LPUART, ENABLE);
}

void RCC_Configuration_lpuart(struct os_serial_device *serial)
{
    struct cm_lpuart *lpuart;

    OS_ASSERT(serial != OS_NULL);
    lpuart = (struct cm_lpuart *)serial->parent.user_data;

    OS_ASSERT(lpuart != OS_NULL);

    /* Enable GPIO clock */
    LPUART0_GPIO_APBxClkCmd(lpuart->info->lpuart_gpio_clk, ENABLE);
    /* Enable LPUARTx Clock */
    RCC_Configuration_CLK(RCC_LPUARTCLK_SRC_HSI);
}

void NVIC_Configuration_lpuart(void)
{
    NVIC_InitType NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    /* Enable the LPUARTx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel            = LPUART_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static os_err_t cm_lpuart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    LPUART_InitType LPUART_InitStructure;

    /* System Clocks Configuration */
    RCC_Configuration_lpuart(serial);

    /* NVIC configuration */
    NVIC_Configuration_lpuart();

    /* Configure the GPIO ports */
    GPIO_Configuration_lpuart(serial);

    struct cm_lpuart *lpuart;

    OS_ASSERT(serial != OS_NULL);
    lpuart = (struct cm_lpuart *)serial->parent.user_data;

    OS_ASSERT(lpuart != OS_NULL);

    /* LPUARTx configuration */
    LPUART_InitStructure.BaudRate            = cfg->baud_rate;

    switch (cfg->parity)
    {
    case PARITY_NONE:
        LPUART_InitStructure.Parity          = LPUART_PE_NO;
        break;
    case PARITY_ODD:
        LPUART_InitStructure.Parity          = LPUART_PE_ODD;
        break;
    case PARITY_EVEN:
        LPUART_InitStructure.Parity          = LPUART_PE_EVEN;
        break;
    default:
        return OS_EINVAL;
    }
    LPUART_InitStructure.RtsThreshold        = LPUART_RTSTH_FIFOFU;
    LPUART_InitStructure.HardwareFlowControl = LPUART_HFCTRL_NONE;
    LPUART_InitStructure.Mode                = LPUART_MODE_RX | LPUART_MODE_TX;

    /* Configure LPUARTx */
    LPUART_Init(&LPUART_InitStructure);

    cm_lpuart_sdma_init(lpuart, &lpuart->serial_dev.rx_fifo->ring);

    return OS_EOK;
}

static os_err_t cm_lpuart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    cm_lpuart_t *lpuart = os_container_of(serial, cm_lpuart_t, serial_dev);

    if (lpuart->info->dma_support == 0)
    {
        LPUART_ConfigInt(LPUART_INT_FIFO_NE, DISABLE);
    }
#ifdef LPUART0_USING_DMA
    else
    {
        LPUART_EnableDMA(LPUART_DMAREQ_RX, DISABLE);
        DMA_EnableChannel(DMA_CH1, DISABLE);
    }
#endif

    return OS_EOK;
}

/*
static int cm_lpuart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    return 0;
}
*/

static int cm_lpuart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;
    struct cm_lpuart *lpuart;

    OS_ASSERT(serial != OS_NULL);
    lpuart = (struct cm_lpuart *)serial->parent.user_data;

    OS_ASSERT(lpuart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();

        LPUART_SendData(buff[i]);
        /* Loop until LPUARTx DAT register is empty */
        while (LPUART_GetFlagStatus(LPUART_FLAG_TXC) == RESET)
        {
        }
        LPUART_ClrFlag(LPUART_FLAG_TXC);

        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops cm_lpuart_ops =
{
    .init         = cm_lpuart_init,
    .deinit       = cm_lpuart_deinit,

    .poll_send    = cm_lpuart_poll_send,
};

static int cm32_lpuart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct cm_lpuart *lpuart;

    lpuart = os_calloc(1, sizeof(struct cm_lpuart));
    OS_ASSERT(lpuart);

    lpuart->info = (struct cm32_lpuart_info *)dev->info;
    struct os_serial_device *serial = &lpuart->serial_dev;

    level = os_irq_lock();
    os_list_add_tail(&cm32_lpuart_list, &lpuart->list);
    os_irq_unlock(level);

    serial->ops    = &cm_lpuart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, lpuart);

    return 0;
}

OS_DRIVER_INFO cm32_lpuart_driver = {
    .name   = "LPuart_Type",
    .probe  = cm32_lpuart_probe,
};

OS_DRIVER_DEFINE(cm32_lpuart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

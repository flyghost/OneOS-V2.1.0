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
#include <drv_lpuart.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.lpuart"
#include <drv_log.h>

#ifdef OS_USING_SERIAL

static os_list_node_t fm33_lpuart_list = OS_LIST_INIT(fm33_lpuart_list);

struct fm33_lpuart
{
    struct os_serial_device serial;
    struct fm33_lpuart_info *info;

    soft_dma_t  sdma;

    os_uint8_t *rx_buff;
    os_uint32_t rx_size;
    os_uint32_t rx_index;

    const os_uint8_t *tx_buff;
    os_uint32_t tx_size;
    os_uint32_t tx_index;

    os_list_node_t list;
};

static void lpuart_isr(struct os_serial_device *serial)
{
    struct fm33_lpuart *lpuart = os_container_of(serial, struct fm33_lpuart, serial);

    if((ENABLE == LPUART_LPUCON_RXIE_Getable())
            && (SET == LPUART_LPUSTA_MATCH_Chk()))
    {
        lpuart->rx_buff[lpuart->rx_index++] = LPUART_LPURXD_Read();

        if (lpuart->rx_index == (lpuart->rx_size / 2))
        {
            soft_dma_half_irq(&lpuart->sdma);
        }

        if (lpuart->rx_index == lpuart->rx_size)
        {
            lpuart->rx_index = 0;
            soft_dma_full_irq(&lpuart->sdma);
        }

        LPUART_LPUSTA_MATCH_Clr();
    }
    if((ENABLE == LPUART_LPUCON_TCIE_Getable())
            && (SET == LPUART_LPUIF_TC_IF_Chk()))
    {
        LPUART_LPUIF_TC_IF_Clr();

        if(lpuart->tx_size)
        {
            if(lpuart->tx_index < lpuart->tx_size)
            {
                LPUART_LPUTXD_Write(lpuart->tx_buff[lpuart->tx_index++]);
            }
            else
            {
                LPUART_LPUCON_TCIE_Setable(DISABLE);
                os_hw_serial_isr_txdone((struct os_serial_device *)lpuart);
            }
        }
    }
}

void LPUART_IRQHandler(void)
{
    struct fm33_lpuart *lpuart;

    os_list_for_each_entry(lpuart, &fm33_lpuart_list, struct fm33_lpuart, list)
    {
        if (lpuart->info->instance == LPUART)
        {
            lpuart_isr(&lpuart->serial);
            break;
        }
    }
}

static os_uint32_t fm33_lpuart_dma_ram_addr(struct fm33_lpuart *lpuart)
{
    DMA_CH_Type  CHx;
    os_uint32_t  addr;

    CHx = lpuart->info->dma_param.CHx;
    addr = *(uint32_t *)(0x40000404 + 4 + CHx * 8);

    return addr;
}

/*this handler will be called in DMA_IRQHandler,in drv_uart.c, if lpuart dma is enabled*/
void lpuart_dma_recv_complete_handler(LPUART_Type *huart)
{
    struct fm33_lpuart     *lpuart;

    os_list_for_each_entry(lpuart, &fm33_lpuart_list, struct fm33_lpuart, list)
    {
        if (lpuart->info->instance == huart)
        {
            soft_dma_half_irq(&lpuart->sdma);

            break;
        }
    }
}

static os_uint32_t fm33_sdma_int_get_index(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    return lpuart->rx_index;
}

static os_err_t fm33_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_lpuart *lpuart = os_container_of(dma, struct fm33_lpuart, sdma);

    lpuart->rx_buff  = buff;
    lpuart->rx_size  = size;
    lpuart->rx_index = 0;

    NVIC_SetPriority(lpuart->info->irqn, lpuart->info->irq_priority);
    NVIC_EnableIRQ(lpuart->info->irqn);
	  LPUART_COMPARE_Write(0x33);
    LPUART_LPUCON_RXIE_Setable(ENABLE);

    return OS_EOK;
}

static os_uint32_t fm33_sdma_int_stop(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    LPUART_LPUCON_RXIE_Setable(DISABLE);

    return lpuart->rx_index;
}

static os_uint32_t fm33_sdma_dma_get_index(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    return fm33_lpuart_dma_ram_addr(lpuart) - lpuart->info->dma_addr_start;
}

static os_err_t fm33_sdma_dma_init(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    RCC_PERCLK_SetableEx(DMACLK, ENABLE);

    lpuart->info->dma_param.CHxPRI  = DMA_CHxCTRL_PRI_MEDIUM;
    lpuart->info->dma_param.CHxINC  = DMA_CHxCTRL_INC_INC;
    lpuart->info->dma_param.CHxFTIE = ENABLE;
    lpuart->info->dma_param.CHxHTIE = DISABLE;
    lpuart->info->dma_param.CHxEN   = ENABLE;

    DMA_GLOBALCTRL_DMAEN_Setable(ENABLE);

    return OS_EOK;
}

static os_err_t fm33_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_lpuart *lpuart = os_container_of(dma, struct fm33_lpuart, sdma);

    lpuart->rx_buff  = buff;
    lpuart->rx_size  = size;

    lpuart->info->dma_param.CHxTSIZE = lpuart->rx_size - 1;
    lpuart->info->dma_param.CHxRAMAD = (uint32_t)(lpuart->rx_buff);

    DMA_Init(&lpuart->info->dma_param);
    lpuart->info->dma_addr_start = fm33_lpuart_dma_ram_addr(lpuart);

    LPUART_LPUCON_RXIE_Setable(DISABLE);

    DMA_CHSTATUS_DMACHxFT_Clr(lpuart->info->dma_param.CHx);

    NVIC_EnableIRQ(DMA_IRQn);

    return OS_EOK;
}
static os_uint32_t fm33_sdma_dma_stop(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    OS_ASSERT(lpuart->info->dma_enable != OS_FALSE);

    DMA_CHxCTRL_EN_Setable(lpuart->info->dma_param.CHx, DISABLE);

    return fm33_lpuart_dma_ram_addr(lpuart) - lpuart->info->dma_addr_start;
}

static void fm33_lpuart_sdma_callback(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)lpuart);
}

static void fm33_lpuart_sdma_init(struct fm33_lpuart *lpuart, dma_ring_t *ring)
{
    soft_dma_t *dma = &lpuart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(lpuart->serial.config.baud_rate);

    if (lpuart->info->dma_enable == OS_FALSE)
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

    dma->cbs.dma_half_callback      = fm33_lpuart_sdma_callback;
    dma->cbs.dma_full_callback      = fm33_lpuart_sdma_callback;
    dma->cbs.dma_timeout_callback   = fm33_lpuart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&lpuart->sdma, OS_TRUE);
}

static os_err_t fm33_lpuart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct fm33_lpuart    *lpuart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    lpuart = os_container_of(serial, struct fm33_lpuart, serial);

    RCC_PERCLK_SetableEx(LPUFCKEN, ENABLE);
    RCC_PERCLK_SetableEx(LPUARTCKEN, ENABLE);

    GPIO_PF4AFSEL_PF4AFS_Set(GPIO_PF4AFSEL_PF4AFS_LPUART_TX);

    AltFunIO(lpuart->info->port, lpuart->info->tx_pin, 0);
    AltFunIO(lpuart->info->port, lpuart->info->rx_pin, 2);

    lpuart->info->uart_param.BaudRate = cfg->baud_rate;

    switch(cfg->stop_bits)
    {
    case STOP_BITS_1:
        lpuart->info->uart_param.StopBit = OneBit;
        break;
    case STOP_BITS_2:
        lpuart->info->uart_param.StopBit = TwoBit;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        lpuart->info->uart_param.ParityBit = NONE;
        break;
    case PARITY_ODD:
        lpuart->info->uart_param.ParityBit = ODD;
        break;
    case PARITY_EVEN:
        lpuart->info->uart_param.ParityBit = EVEN;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->data_bits)
    {
    case DATA_BITS_7:
        lpuart->info->uart_param.DataBit = Seven7Bit;
        break;
    case DATA_BITS_8:
        lpuart->info->uart_param.DataBit = Eight8Bit;
        break;
    case DATA_BITS_9:
        lpuart->info->uart_param.DataBit = Nine9Bit;
        break;
    default:
        return OS_EINVAL;
    }

    LPUART_SInit(&(lpuart->info->uart_param));

    LPUART_LPUEN_RXEN_Setable(ENABLE);
    LPUART_LPUEN_TXEN_Setable(ENABLE);

    fm33_lpuart_sdma_init(lpuart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t fm33_lpuart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    struct fm33_lpuart *lpuart = os_container_of(serial, struct fm33_lpuart, serial);

    if (lpuart->info->dma_enable != OS_FALSE)
    {
        soft_dma_stop(&lpuart->sdma);
    }

    LPUART_LPUCON_RXIE_Setable(DISABLE);

    return OS_EOK;
}

static int fm33_lpuart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct fm33_lpuart     *lpuart;

    OS_ASSERT(serial != OS_NULL);
    lpuart = os_container_of(serial, struct fm33_lpuart, serial);

    lpuart->tx_index   = 0;
    lpuart->tx_size  = size;
    lpuart->tx_buff  = buff;

    LPUART_LPUIF_TXIF_Clr();
    LPUART_LPUIF_TC_IF_Clr();
    LPUART_LPUCON_TCIE_Setable(ENABLE);
    NVIC_EnableIRQ(lpuart->info->irqn);
    LPUART_LPUTXD_Write(lpuart->tx_buff[lpuart->tx_index++]);

    return size;
}

static int fm33_lpuart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    LPUART_LPUIF_TXIF_Clr();
    LPUART_LPUIF_TC_IF_Clr();
    LPUART_LPUCON_TCIE_Setable(DISABLE);

    for(i = 0; i < size; i++)
    {
        level = os_irq_lock();
        LPUART_LPUTXD_Write(buff[i]);
        while(RESET == LPUART_LPUIF_TC_IF_Chk());
        LPUART_LPUIF_TC_IF_Clr();
        os_irq_unlock(level);
    }
    return size;
}

static const struct os_uart_ops fm33_lpuart_ops = {
    .init         = fm33_lpuart_init,
    .deinit       = fm33_lpuart_deinit,

    .start_send   = fm33_lpuart_start_send,

    .poll_send    = fm33_lpuart_poll_send,
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

static int fm33_lpuart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct fm33_lpuart *lpuart;

    lpuart = os_calloc(1, sizeof(struct fm33_lpuart));
    OS_ASSERT(lpuart);

    lpuart->info = (struct fm33_lpuart_info *)dev->info;
    config.baud_rate = lpuart->info->baud_rate;
    struct os_serial_device *serial = &lpuart->serial;

    level = os_irq_lock();
    os_list_add_tail(&fm33_lpuart_list, &lpuart->list);
    os_irq_unlock(level);

    serial->ops    = &fm33_lpuart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, lpuart);

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
            LPUART_LPUTXD_Write('\r');
        }

        LPUART_LPUTXD_Write(*str);
        str++;
    }
}

static int fm33_lpuart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    console_uart = (LPUART_HandleTypeDef *)dev->info;
    return 0;
}

#endif

OS_DRIVER_INFO fm33_lpuart_driver = {
    .name   = "LPUART_HandleTypeDef",
    .probe  = fm33_lpuart_probe,
};

OS_DRIVER_DEFINE(fm33_lpuart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

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
 * @brief       This file implements lpuart driver for hc32
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
#include "drv_lpuart.h"
#include "drv_dma_irq.h"

static os_list_node_t hc32_lpuart_list = OS_LIST_INIT(hc32_lpuart_list);

static void lpuart_isr(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    hc_lpuart_t *lpuart = os_container_of(serial, hc_lpuart_t, serial_dev);

    OS_ASSERT(lpuart != OS_NULL);

    if(LPUart_GetStatus(lpuart->info->lp_idx, LPUartRC) != 0)
    {
        LPUart_ClrStatus(lpuart->info->lp_idx, LPUartRC);
        lpuart->rx_buff[lpuart->rx_index++] = LPUart_ReceiveData(lpuart->info->lp_idx);

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

void lpuart_irq(int device_num)
{
    struct hc_lpuart *lpuart;

    os_list_for_each_entry(lpuart, &hc32_lpuart_list, struct hc_lpuart, list)
    {
        if (lpuart->info->uart_device == device_num)
        {
            lpuart_isr(&lpuart->serial_dev);
            break;
        }
    }
}

void LpUart0_IRQHandler(void)
{
    lpuart_irq(0);
}

void LpUart1_IRQHandler(void)
{
    lpuart_irq(1);
}

int lp_cnt_data = 0;

static void lpuart_dma_isr(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    hc_lpuart_t *lpuart = os_container_of(serial, hc_lpuart_t, serial_dev);

    OS_ASSERT(lpuart != OS_NULL);

    if (lpuart->info->dma_channel == DmaCh0)
    {
        lp_cnt_data = lpuart->rx_size - M0P_DMAC->CONFA0_f.TC;
    }
    else
    {
        lp_cnt_data = lpuart->rx_size - M0P_DMAC->CONFA1_f.TC;
    }
    Dma_ClrStat(lpuart->info->dma_channel);

    soft_dma_half_irq(&lpuart->sdma);

    Dma_EnableChannel(lpuart->info->dma_channel);
    Dma_EnableChannelIrq(lpuart->info->dma_channel);
}

void lpuart_dma_irq(en_dma_channel_t dma_channel)
{
    struct hc_lpuart *lpuart;

    os_list_for_each_entry(lpuart, &hc32_lpuart_list, struct hc_lpuart, list)
    {
        if (lpuart->info->dma_channel == dma_channel)
        {
            lpuart_dma_isr(&lpuart->serial_dev);
            break;
        }
    }
}

/* interrupt rx mode */
static os_uint32_t hc_lpuart_sdma_int_get_index(soft_dma_t *dma)
{
    hc_lpuart_t *lpuart = os_container_of(dma, hc_lpuart_t, sdma);

    return lpuart->rx_index;
}

static os_err_t hc_lpuart_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    hc_lpuart_t *lpuart = os_container_of(dma, hc_lpuart_t, sdma);

    lpuart->rx_buff  = buff;
    lpuart->rx_index = 0;
    lpuart->rx_size  = size;

    LPUart_ClrStatus(lpuart->info->lp_idx, LPUartRC);
    LPUart_EnableIrq(lpuart->info->lp_idx, LPUartRxIrq);

    return OS_EOK;
}

static os_uint32_t hc_lpuart_sdma_int_stop(soft_dma_t *dma)
{
    hc_lpuart_t *lpuart = os_container_of(dma, hc_lpuart_t, sdma);

    LPUart_DisableIrq(lpuart->info->lp_idx, LPUartRxIrq);

    return lpuart->rx_index;
}

/* dma rx mode */
static os_uint32_t hc_lpuart_sdma_dma_get_index(soft_dma_t *dma)
{
    hc_lpuart_t *lpuart = os_container_of(dma, hc_lpuart_t, sdma);

    if (lpuart->info->dma_channel == DmaCh0)
    {
        return lpuart->sdma_hard_size - M0P_DMAC->CONFA0_f.TC - 1;
    }
    else if (lpuart->info->dma_channel == DmaCh1)
    {
        return lpuart->sdma_hard_size - M0P_DMAC->CONFA1_f.TC - 1;
    }

    return 0;
}

static os_err_t hc_lpuart_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t hc_lpuart_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    hc_lpuart_t *lpuart = os_container_of(dma, hc_lpuart_t, sdma);

    lpuart->sdma_hard_size = size;

    stc_dma_cfg_t stcDmaCfg;
    DDL_ZERO_STRUCT(stcDmaCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralDma, TRUE);

    stcDmaCfg.u32SrcAddress = (os_uint32_t)lpuart->info->lp_idx;
    stcDmaCfg.enRequestNum = lpuart->info->req_num;
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
    Dma_InitChannel(lpuart->info->dma_channel, &stcDmaCfg);

    EnableNvic(DMAC_IRQn, IrqLevel3, TRUE);

    Dma_ClrStat(lpuart->info->dma_channel);
    Dma_EnableChannel(lpuart->info->dma_channel);
    Dma_EnableChannelIrq(lpuart->info->dma_channel);

    return OS_EOK;
}

static os_uint32_t hc_lpuart_sdma_dma_stop(soft_dma_t *dma)
{
    hc_lpuart_t *lpuart = os_container_of(dma, hc_lpuart_t, sdma);

    Dma_DisableChannel(lpuart->info->dma_channel);
    Dma_DisableChannelIrq(lpuart->info->dma_channel);

    return hc_lpuart_sdma_dma_get_index(dma);
}

/* sdma callback */
static void hc_lpuart_sdma_callback(soft_dma_t *dma)
{
    hc_lpuart_t *lpuart = os_container_of(dma, hc_lpuart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)lpuart);
}

static void hc_lpuart_sdma_init(struct hc_lpuart *lpuart, dma_ring_t *ring)
{
    soft_dma_t *dma = &lpuart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = HARD_DMA_FLAG_FULL_IRQ;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(lpuart->serial_dev.config.baud_rate);

    if (lpuart->info->dma_support == 0)
    {
        dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;

        dma->ops.get_index          = hc_lpuart_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = hc_lpuart_sdma_int_start;
        dma->ops.dma_stop           = hc_lpuart_sdma_int_stop;
    }
    else
    {
        dma->hard_info.mode         = HARD_DMA_MODE_CIRCULAR;

        dma->ops.get_index          = hc_lpuart_sdma_dma_get_index;
        dma->ops.dma_init           = hc_lpuart_sdma_dma_init;
        dma->ops.dma_start          = hc_lpuart_sdma_dma_start;
        dma->ops.dma_stop           = hc_lpuart_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = hc_lpuart_sdma_callback;
    dma->cbs.dma_full_callback      = hc_lpuart_sdma_callback;
    dma->cbs.dma_timeout_callback   = hc_lpuart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&lpuart->sdma, OS_TRUE);
}

void App_LpUartClkCfg(void)
{
    Sysctrl_SetRCLTrim(SysctrlRclFreq38400);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
}

static void hc_lpuart_gpio_init(struct hc32_lpuart_info *info)
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

static os_err_t hc_lpuart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    hc_lpuart_t *lpuart = os_container_of(serial, hc_lpuart_t, serial_dev);

    stc_lpuart_cfg_t  stcCfg;
    DDL_ZERO_STRUCT(stcCfg);

    /* LPUART clock configuration, transfer clock select RCL */
    /*App_LpUartClkCfg();*/

    if (lpuart->info->uart_device == 0)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart0, TRUE);
    }
    if (lpuart->info->uart_device == 1)
    {
        Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart1, TRUE);
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        stcCfg.enStopBit = LPUart1bit;
        break;
    case STOP_BITS_2:
        stcCfg.enStopBit = LPUart2bit;
        break;
    default:
        return OS_EINVAL;
    }

    switch (cfg->parity)
    {
    case PARITY_NONE:
        stcCfg.enRunMode = LPUartMskMode1;
        break;
    case PARITY_ODD:
        stcCfg.enMmdorCk = LPUartOdd;
        stcCfg.enRunMode = LPUartMskMode3;
        break;
    case PARITY_EVEN:
        stcCfg.enMmdorCk = LPUartEven;
        stcCfg.enRunMode = LPUartMskMode3;
        break;
    default:
        return OS_EINVAL;
    }

    stcCfg.stcBaud.enSclkSel = LPUartMskPclk;        /* LPUartMskRcl; Transfer clock source RCL */
    stcCfg.stcBaud.u32Sclk = Sysctrl_GetPClkFreq();  /* 38400; RCL clock frequency 38400Hz */
    stcCfg.stcBaud.enSclkDiv = LPUartMsk4Or8Div;
    stcCfg.stcBaud.u32Baud = cfg->baud_rate;

    LPUart_Init(lpuart->info->lp_idx, &stcCfg);

    LPUart_ClrStatus(lpuart->info->lp_idx, LPUartRC);
    LPUart_ClrStatus(lpuart->info->lp_idx, LPUartTC);

    if (lpuart->info->dma_support == 1)
    {
        LPUart_EnableFunc(lpuart->info->lp_idx, LPUartDmaRxFunc);
    }
    else
    {
        EnableNvic(lpuart->info->irqn, IrqLevel3, TRUE);
        LPUart_EnableIrq(lpuart->info->lp_idx, LPUartRxIrq);
    }

    hc_lpuart_sdma_init(lpuart, &lpuart->serial_dev.rx_fifo->ring);

    return OS_EOK;
}

static os_err_t hc_lpuart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    hc_lpuart_t *lpuart = os_container_of(serial, hc_lpuart_t, serial_dev);

    if (lpuart->info->dma_support == 0)
    {
        LPUart_DisableIrq(lpuart->info->lp_idx, LPUartRxIrq);
    }
    else
    {
        soft_dma_stop(&lpuart->sdma);

        Dma_DisableChannel(lpuart->info->dma_channel);
        Dma_DisableChannelIrq(lpuart->info->dma_channel);
    }

    return OS_EOK;
}

static int hc_lpuart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);
    hc_lpuart_t *lpuart = os_container_of(serial, hc_lpuart_t, serial_dev);

    OS_ASSERT(lpuart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();

        LPUart_SendData(lpuart->info->lp_idx, buff[i]);

        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops hc_lpuart_ops =
{
    .init       = hc_lpuart_init,
    .deinit     = hc_lpuart_deinit,

    .poll_send  = hc_lpuart_poll_send,
};

static int hc32_lpuart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct hc_lpuart *lpuart;

    lpuart = os_calloc(1, sizeof(struct hc_lpuart));
    OS_ASSERT(lpuart);

    lpuart->info = (struct hc32_lpuart_info *)dev->info;
    struct os_serial_device *serial = &lpuart->serial_dev;

    level = os_irq_lock();
    os_list_add_tail(&hc32_lpuart_list, &lpuart->list);
    os_irq_unlock(level);

    lpuart_dma_channel_get(lpuart);

    hc_lpuart_gpio_init(lpuart->info);

    config.baud_rate = BAUD_RATE_57600;      /* lpuart support baud rate 57600 */

    serial->ops    = &hc_lpuart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, lpuart);

    return 0;
}

OS_DRIVER_INFO hc32_lpuart_driver = {
    .name   = "LPuart_Type",
    .probe  = hc32_lpuart_probe,
};

OS_DRIVER_DEFINE(hc32_lpuart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

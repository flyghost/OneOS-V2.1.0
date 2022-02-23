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
 * @file        drv_eth.c
 *
 * @brief       This file implements eth driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <netif/ethernetif.h>
#include "lwipopts.h"
#include "drv_eth.h"
#include <os_memory.h>
#include <dma.h>
#include <string.h>
#include <bus/bus.h>

/*
 * Emac driver uses CubeMX tool to generate emac and phy's configuration,
 * the configuration files can be found in CubeMX_Config folder.
 */

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.eth"

#define DBG_TAG "drv.eth"
#include <dlog.h>

#define MAX_ADDR_LEN 6

struct os_stm32_eth
{
    struct eth_device parent;          /* inherit from ethernet device */
#ifndef PHY_USING_INTERRUPT_MODE
    os_timer_t *poll_link_timer;
#endif
    os_uint8_t dev_addr[MAX_ADDR_LEN]; /* interface address info, hw address */
    uint32_t   ETH_Speed;              /* ETH_Speed */
    uint32_t   ETH_Mode;               /* ETH_Duplex_Mode */
};

static ETH_DMADescTypeDef *DMARxDscrTab, *DMATxDscrTab;
static os_uint8_t         *Rx_Buff, *Tx_Buff;
static ETH_HandleTypeDef  *EthHandle;
static struct os_stm32_eth stm32_eth_device;

#if defined(ETH_RX_DUMP) || defined(ETH_TX_DUMP)
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const os_uint8_t *ptr, os_size_t buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int            i, j;

    for (i = 0; i < buflen; i += 16)
    {
        os_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                os_kprintf("%02X ", buf[i + j]);
            else
                os_kprintf("   ");
        os_kprintf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                os_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        os_kprintf("\r\n");
    }
}
#endif

static int phy_reset(void)
{
    if (ETH_RESET_PIN < 0)
        return OS_EINVAL;
    
#ifdef ETH_RESET_PIN_ACTIVE_HIGH
    os_pin_write(ETH_RESET_PIN, PIN_HIGH);
    os_task_msleep(100);
    os_pin_write(ETH_RESET_PIN, PIN_LOW);
    os_task_msleep(100);
#else
    os_pin_write(ETH_RESET_PIN, PIN_LOW);
    os_task_msleep(100);
    os_pin_write(ETH_RESET_PIN, PIN_HIGH);
    os_task_msleep(100);
#endif

    return OS_EOK;
}

static int phy_init(void)
{
    if (ETH_RESET_PIN < 0)
        return OS_EINVAL;
    
    os_pin_mode(ETH_RESET_PIN, PIN_MODE_OUTPUT);
    
#ifdef ETH_RESET_PIN_ACTIVE_HIGH
    os_pin_write(ETH_RESET_PIN, PIN_LOW);
#else
    os_pin_write(ETH_RESET_PIN, PIN_HIGH);
#endif

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           EMAC initialization function
 *
 * @param[in]       dev             pointer to os_device_t.
 *
 * @return          Return eth init status.
 * @retval          OS_EOK       init success.
 * @retval          Others       init failed.
 ***********************************************************************************************************************
 */
static os_err_t os_stm32_eth_init(os_device_t *dev)
{
    phy_init();
    phy_reset();

    /* ETHERNET ReConfiguration MAC addr */
    EthHandle->Init.MACAddr = (os_uint8_t *)&stm32_eth_device.dev_addr[0];
    EthHandle->Init.RxMode  = ETH_RXINTERRUPT_MODE;

    HAL_ETH_DeInit(EthHandle);

    /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
    if (HAL_ETH_Init(EthHandle) != HAL_OK)
    {
        LOG_E(DBG_TAG,"eth hardware init failed");
    }
    else
    {
        LOG_D(DBG_TAG, "eth hardware init success");
    }
    
    /* Initialize Tx Descriptors list: Chain Mode */
    HAL_ETH_DMATxDescListInit(EthHandle, DMATxDscrTab, Tx_Buff, ETH_TXBUFNB);

    /* Initialize Rx Descriptors list: Chain Mode  */
    HAL_ETH_DMARxDescListInit(EthHandle, DMARxDscrTab, Rx_Buff, ETH_RXBUFNB);

    /* ETH interrupt Init */
    HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
    
    /* Enable MAC and DMA transmission and reception */
    if (HAL_ETH_Start(EthHandle) == HAL_OK)
    {
        LOG_D(DBG_TAG, "emac hardware start");
    }
    else
    {
        LOG_E(DBG_TAG,"emac hardware start faild");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t os_stm32_eth_deinit(os_device_t *dev)
{
    LOG_D(DBG_TAG, "emac deinit");
    return OS_EOK;
}

static os_size_t os_stm32_eth_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    LOG_D(DBG_TAG, "emac read");
    os_set_errno(OS_ENOSYS);
    return 0;
}

static os_size_t os_stm32_eth_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    LOG_D(DBG_TAG, "emac write");
    os_set_errno(OS_ENOSYS);
    return 0;
}

static os_err_t os_stm32_eth_control(os_device_t *dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args)
            memcpy(args, stm32_eth_device.dev_addr, 6);
        else
            return OS_ERROR;
        break;

    default:
        break;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           ethernet device interface,transmit data.
 *
 * @param[in]       dev             pointer to os_device_t.
 * @param[in]       p               pointer to struct pbuf.
 *
 * @return          Return eth tx result.
 * @retval          ERR_OK            tx success.
 * @retval          Others            tx failed.
 ***********************************************************************************************************************
 */
os_err_t os_stm32_eth_tx(os_device_t *dev, struct pbuf *p)
{
    os_base_t level;
    os_err_t          ret = OS_ERROR;
    HAL_StatusTypeDef state;
    struct pbuf      *q;
    uint8_t          *buffer = (uint8_t *)(EthHandle->TxDesc->Buffer1Addr);
    __IO ETH_DMADescTypeDef *DmaTxDesc;
    uint32_t                 framelength     = 0;
    uint32_t                 bufferoffset    = 0;

    DmaTxDesc    = EthHandle->TxDesc;
    bufferoffset = 0;

    if (p->tot_len > (ETH_MAX_ETH_PAYLOAD + ETH_HEADER))
        OS_ASSERT_EX(0, "eth support mtu is %d!", (ETH_MAX_ETH_PAYLOAD + ETH_HEADER));
    
    while ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
    {
        if ((EthHandle->Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
        {
            /* Clear TUS ETHERNET DMA flag */
            EthHandle->Instance->DMASR = ETH_DMASR_TUS;

            /* Resume DMA transmission*/
            EthHandle->Instance->DMATPDR = 0;
        }
    }
    /* copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next)
    {
        memcpy((uint8_t *)((uint8_t *)buffer + bufferoffset),
               (uint8_t *)((uint8_t *)q->payload),
               q->len);
        bufferoffset = bufferoffset + q->len;
        framelength  = framelength + q->len;
    }

#ifdef ETH_TX_DUMP
    dump_hex(buffer, p->tot_len);
#endif

    /* Prepare transmit descriptors to give to DMA */
    /* TODO Optimize data send speed*/
    LOG_D(DBG_TAG, "transmit frame length :%d", framelength);

    level = os_irq_lock();
    OS_ASSERT(EthHandle->Lock != HAL_LOCKED);
    state = HAL_ETH_TransmitFrame(EthHandle, framelength);
    os_irq_unlock(level);
    
    if (state != HAL_OK)
    {
        LOG_E(DBG_TAG,"eth transmit frame faild: %d", state);
        ret = OS_ERROR;
    }

    ret = OS_EOK;

    return ret;
}

/* receive data */
struct pbuf *os_stm32_eth_rx(os_device_t *dev)
{
    os_base_t level;
    struct pbuf      *p = NULL;
    struct pbuf      *q = NULL;
    HAL_StatusTypeDef state;
    uint16_t          len = 0;
    uint8_t          *buffer;
    __IO ETH_DMADescTypeDef *dmarxdesc;
    uint32_t                 bufferoffset    = 0;
    uint32_t                 payloadoffset   = 0;
    uint32_t                 byteslefttocopy = 0;
    uint32_t                 i               = 0;

    /* Get received frame */
    level = os_irq_lock();
    OS_ASSERT(EthHandle->Lock != HAL_LOCKED);
    state = HAL_ETH_GetReceivedFrame_IT(EthHandle);
    os_irq_unlock(level);
    
    if (state != HAL_OK)
    {
        LOG_D(DBG_TAG, "receive frame faild");
        return NULL;
    }

    /* Obtain the size of the packet and put it into the "len" variable. */
    len    = EthHandle->RxFrameInfos.length;
    buffer = (uint8_t *)EthHandle->RxFrameInfos.buffer;

    LOG_D(DBG_TAG, "receive frame len : %d", len);

    if (len > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }

#ifdef ETH_RX_DUMP
    dump_hex(buffer, p->tot_len);
#endif

    if (p != NULL)
    {
        dmarxdesc    = EthHandle->RxFrameInfos.FSRxDesc;
        bufferoffset = 0;
        for (q = p; q != NULL; q = q->next)
        {
            byteslefttocopy = q->len;
            payloadoffset   = 0;

            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
            while ((byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE)
            {
                /* Copy data to pbuf */
                memcpy((uint8_t *)((uint8_t *)q->payload + payloadoffset),
                       (uint8_t *)((uint8_t *)buffer + bufferoffset),
                       (ETH_RX_BUF_SIZE - bufferoffset));

                /* Point to next descriptor */
                dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
                buffer    = (uint8_t *)(dmarxdesc->Buffer1Addr);

                byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
                payloadoffset   = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
                bufferoffset    = 0;
            }
            /* Copy remaining data in pbuf */
            memcpy((uint8_t *)((uint8_t *)q->payload + payloadoffset),
                   (uint8_t *)((uint8_t *)buffer + bufferoffset),
                   byteslefttocopy);
            bufferoffset = bufferoffset + byteslefttocopy;
        }
    }

    /* Release descriptors to DMA */
    /* Point to first descriptor */
    dmarxdesc = EthHandle->RxFrameInfos.FSRxDesc;
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i = 0; i < EthHandle->RxFrameInfos.SegCount; i++)
    {
        dmarxdesc->Status |= ETH_DMARXDESC_OWN;
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
    }

    /* Clear Segment_Count */
    EthHandle->RxFrameInfos.SegCount = 0;

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((EthHandle->Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
    {
        /* Clear RBUS ETHERNET DMA flag */
        EthHandle->Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        EthHandle->Instance->DMARPDR = 0;
    }

    return p;
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    
    os_err_t result;
    result = eth_device_ready(&(stm32_eth_device.parent));
    if (result != OS_EOK)
        LOG_I(DBG_TAG,"RxCpltCallback err = %d", result);

}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    LOG_E(DBG_TAG,"eth err");
}

enum
{
    PHY_LINK        = (1 << 0),
    PHY_100M        = (1 << 1),
    PHY_FULL_DUPLEX = (1 << 2),
};

static void phy_linkchange()
{
    static os_uint8_t phy_speed     = 0;
    os_uint8_t        phy_speed_new = 0;
    os_uint32_t       status;

    HAL_ETH_ReadPHYRegister(EthHandle, PHY_BASIC_STATUS_REG, (uint32_t *)&status);
    //LOG_D(DBG_TAG, "phy basic status reg is 0x%X", status);

    if (status & (PHY_AUTONEGO_COMPLETE_MASK | PHY_LINKED_STATUS_MASK))
    {
        os_uint32_t SR = 0;

        phy_speed_new |= PHY_LINK;

        HAL_ETH_ReadPHYRegister(EthHandle, PHY_Status_REG, (uint32_t *)&SR);
        LOG_D(DBG_TAG, "phy control status reg is 0x%X", SR);

        if (PHY_Status_SPEED_100M(SR))
        {
            phy_speed_new |= PHY_100M;
        }

        if (PHY_Status_FULL_DUPLEX(SR))
        {
            phy_speed_new |= PHY_FULL_DUPLEX;
        }
    }

    if (phy_speed != phy_speed_new)
    {
        phy_speed = phy_speed_new;
        if (phy_speed & PHY_LINK)
        {
            LOG_D(DBG_TAG, "link up");
            if (phy_speed & PHY_100M)
            {
                LOG_D(DBG_TAG, "100Mbps");
                stm32_eth_device.ETH_Speed = ETH_SPEED_100M;
            }
            else
            {
                stm32_eth_device.ETH_Speed = ETH_SPEED_10M;
                LOG_D(DBG_TAG, "10Mbps");
            }

            if (phy_speed & PHY_FULL_DUPLEX)
            {
                LOG_D(DBG_TAG, "full-duplex");
                stm32_eth_device.ETH_Mode = ETH_MODE_FULLDUPLEX;
            }
            else
            {
                LOG_D(DBG_TAG, "half-duplex");
                stm32_eth_device.ETH_Mode = ETH_MODE_HALFDUPLEX;
            }

            /* send link up. */
            eth_device_linkchange(&stm32_eth_device.parent, OS_TRUE);
        }
        else
        {
            LOG_I(DBG_TAG,"link down");
            eth_device_linkchange(&stm32_eth_device.parent, OS_FALSE);
        }
    }
}

#ifdef PHY_USING_INTERRUPT_MODE
static void eth_phy_isr(void *args)
{
    os_uint32_t status = 0;

    HAL_ETH_ReadPHYRegister(EthHandle, PHY_INTERRUPT_FLAG_REG, (uint32_t *)&status);
    LOG_D(DBG_TAG, "phy interrupt status reg is 0x%X", status);

    phy_linkchange();
}
#endif /* PHY_USING_INTERRUPT_MODE */

static void phy_monitor_task_entry(void *parameter)
{
    uint8_t phy_addr       = 0xFF;
    uint8_t detected_count = 0;

    while (phy_addr == 0xFF)
    {
        /* phy search */
        os_uint32_t i, temp;
        for (i = 0; i <= 0x1F; i++)
        {
            EthHandle->Init.PhyAddress = i;
            HAL_ETH_ReadPHYRegister(EthHandle, PHY_ID1_REG, (uint32_t *)&temp);

            if (temp != 0xFFFF && temp != 0x00)
            {
                phy_addr = i;
                break;
            }
        }

        detected_count++;
        os_task_msleep(1000);

        if (detected_count > 10)
        {
            LOG_E(DBG_TAG,"No PHY device was detected, please check hardware!");
        }
    }

    LOG_D(DBG_TAG, "Found a phy, address:0x%02X", phy_addr);

    /* RESET PHY */
    LOG_D(DBG_TAG, "RESET PHY!");
    HAL_ETH_WritePHYRegister(EthHandle, PHY_BASIC_CONTROL_REG, PHY_RESET_MASK);
    os_task_msleep(2000);
    HAL_ETH_WritePHYRegister(EthHandle, PHY_BASIC_CONTROL_REG, PHY_AUTO_NEGOTIATION_MASK);

    phy_linkchange();
#ifdef PHY_USING_INTERRUPT_MODE
    /* configuration intterrupt pin */
    os_pin_mode(PHY_INT_PIN, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(PHY_INT_PIN, PIN_IRQ_MODE_FALLING, eth_phy_isr, (void *)"callbackargs");
    os_pin_irq_enable(PHY_INT_PIN, PIN_IRQ_ENABLE);

    /* enable phy interrupt */
    HAL_ETH_WritePHYRegister(EthHandle, PHY_INTERRUPT_MASK_REG, PHY_INT_MASK);
#if defined(PHY_INTERRUPT_CTRL_REG)
    HAL_ETH_WritePHYRegister(EthHandle, PHY_INTERRUPT_CTRL_REG, PHY_INTERRUPT_EN);
#endif
#else  /* PHY_USING_INTERRUPT_MODE */
    stm32_eth_device.poll_link_timer =
        os_timer_create("phylnk", (void (*)(void *))phy_linkchange, NULL, OS_TICK_PER_SECOND, OS_TIMER_FLAG_PERIODIC);
    if (!stm32_eth_device.poll_link_timer || os_timer_start(stm32_eth_device.poll_link_timer) != OS_EOK)
    {
        LOG_E(DBG_TAG,"Start link change detection timer failed");
    }
#endif /* PHY_USING_INTERRUPT_MODE */
}

const static struct os_device_ops eth_ops = {
    .init    = os_stm32_eth_init,
    .deinit  = os_stm32_eth_deinit,
    .read    = os_stm32_eth_read,
    .write   = os_stm32_eth_write,
    .control = os_stm32_eth_control,
};

/**
 ***********************************************************************************************************************
 * @brief           os_hw_stm32_eth_init:init and register the EMAC device.
 *
 * @param[in]       none
 *
 * @return          Return eth init status.
 * @retval          ERR_OK            init success.
 * @retval          Others            init failed.
 ***********************************************************************************************************************
 */
static int stm32_eth_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t state = OS_EOK;
    int align = 64; /* address align */

    EthHandle = (ETH_HandleTypeDef *)dev->info;

    /* Prepare receive and send buffers */
    Rx_Buff = (os_uint8_t *)os_dma_malloc_align(ETH_RXBUFNB* ETH_MAX_PACKET_SIZE, align);
    if (Rx_Buff == OS_NULL)
    {
        LOG_E(DBG_TAG,"No memory");
        state = OS_ENOMEM;
        goto __exit;
    }

    Tx_Buff = (os_uint8_t *)os_dma_malloc_align(ETH_TXBUFNB*ETH_MAX_PACKET_SIZE, align);
    if (Tx_Buff == OS_NULL)
    {
        LOG_E(DBG_TAG,"No memory");
        state = OS_ENOMEM;
        goto __exit;
    }

    DMARxDscrTab = (ETH_DMADescTypeDef *)os_dma_malloc_align(ETH_RXBUFNB*sizeof(ETH_DMADescTypeDef), align);
    if (DMARxDscrTab == OS_NULL)
    {
        LOG_E(DBG_TAG,"No memory");
        state = OS_ENOMEM;
        goto __exit;
    }

    DMATxDscrTab = (ETH_DMADescTypeDef *)os_dma_malloc_align(ETH_TXBUFNB*sizeof(ETH_DMADescTypeDef), align);
    if (DMATxDscrTab == OS_NULL)
    {
        LOG_E(DBG_TAG,"No memory");
        state = OS_ENOMEM;
        goto __exit;
    }

    stm32_eth_device.ETH_Speed = ETH_SPEED_100M;
    stm32_eth_device.ETH_Mode  = ETH_MODE_FULLDUPLEX;

    /* OUI 00-80-E1 STMICROELECTRONICS. */
    stm32_eth_device.dev_addr[0] = 0x00;
    stm32_eth_device.dev_addr[1] = 0x80;
    stm32_eth_device.dev_addr[2] = 0xE1;
    /* generate MAC addr from 96bit unique ID (only for test). */
    stm32_eth_device.dev_addr[3] = *(os_uint8_t *)(UID_BASE + 4);
    stm32_eth_device.dev_addr[4] = *(os_uint8_t *)(UID_BASE + 2);
    stm32_eth_device.dev_addr[5] = *(os_uint8_t *)(UID_BASE + 0);

    stm32_eth_device.parent.parent.ops = &eth_ops;
    stm32_eth_device.parent.parent.user_data = OS_NULL;

    stm32_eth_device.parent.eth_rx = os_stm32_eth_rx;
    stm32_eth_device.parent.eth_tx = os_stm32_eth_tx;

    /* register eth device */
    state = eth_device_init(&(stm32_eth_device.parent), "e0");
    if (OS_EOK == state)
    {
        LOG_D(DBG_TAG, "emac device init success");
    }
    else
    {
        LOG_E(DBG_TAG,"emac device init faild: %d", state);
        state = OS_ERROR;
        goto __exit;
    }

    /* start phy monitor */
    os_task_t *tid;
    tid = os_task_create("phy", phy_monitor_task_entry, OS_NULL, 1024, OS_TASK_PRIORITY_MAX - 2);
    if (tid != OS_NULL)
    {
        os_task_startup(tid);
    }
    else
    {
        state = OS_ERROR;
    }
__exit:
    if (state != OS_EOK)
    {
        if (Rx_Buff)
        {
            os_dma_free_align(Rx_Buff);
        }

        if (Tx_Buff)
        {
            os_dma_free_align(Tx_Buff);
        }

        if (DMARxDscrTab)
        {
            os_dma_free_align(DMARxDscrTab);
        }

        if (DMATxDscrTab)
        {
            os_dma_free_align(DMATxDscrTab);
        }
    }

    return state;
}

OS_DRIVER_INFO stm32_eth_driver = {
    .name   = "ETH_HandleTypeDef",
    .probe  = stm32_eth_probe,
};

OS_DRIVER_DEFINE(stm32_eth_driver,DEVICE,OS_INIT_SUBLEVEL_LOW);


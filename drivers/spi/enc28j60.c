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
 * @file        enc28j60.c
 *
 * @brief       This file provides functions for enc28j60.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_event.h>
#include "enc28j60.h"
#include <pin.h>
#include <enc28j60.h>
#include "drv_spi.h"
#include <drv_gpio.h>
#include "board.h"

#ifdef NET_TRACE
#define NET_DEBUG os_kprintf
#else
#define NET_DEBUG(...)
#endif /* #ifdef NET_TRACE */

struct enc28j60_tx_list_typedef
{
    struct enc28j60_tx_list_typedef *prev;
    struct enc28j60_tx_list_typedef *next;
    os_uint32_t                      addr; /* Pkt addr in buffer */
    os_uint32_t                      len;  /* Pkt len */
    volatile os_bool_t               free; /* 0:busy, 1:free */
};
static struct enc28j60_tx_list_typedef           enc28j60_tx_list[2];
static volatile struct enc28j60_tx_list_typedef *tx_current;
static volatile struct enc28j60_tx_list_typedef *tx_ack;

static os_sem_t tx_sem;

static uint8_t spi_read_op(struct os_spi_device *spi_device, uint8_t op, uint8_t address);
static void    spi_write_op(struct os_spi_device *spi_device, uint8_t op, uint8_t address, uint8_t data);

static uint8_t spi_read(struct os_spi_device *spi_device, uint8_t address);
static void    spi_write(struct os_spi_device *spi_device, os_uint8_t address, os_uint8_t data);

static void     enc28j60_clkout(struct os_spi_device *spi_device, os_uint8_t clk);
static void     enc28j60_set_bank(struct os_spi_device *spi_device, uint8_t address);
static uint32_t enc28j60_interrupt_disable(struct os_spi_device *spi_device);
static void     enc28j60_interrupt_enable(struct os_spi_device *spi_device, uint32_t level);

static uint16_t  enc28j60_phy_read(struct os_spi_device *spi_device, os_uint8_t address);
static void      enc28j60_phy_write(struct os_spi_device *spi_device, os_uint8_t address, uint16_t data);
static os_bool_t enc28j60_check_link_status(struct os_spi_device *spi_device);

#define enc28j60_lock(dev)   os_mutex_lock(&((struct net_device *)dev)->lock, OS_WAIT_FOREVER);
#define enc28j60_unlock(dev) os_mutex_unlock(&((struct net_device *)dev)->lock);

static struct net_device enc28j60_dev;
static uint8_t           Enc28j60Bank;
static uint16_t          NextPacketPtr;

static void _delay_us(uint32_t us)
{
    volatile uint32_t len;
    for (; us > 0; us--)
        for (len = 0; len < 20; len++);
}

static uint8_t spi_read_op(struct os_spi_device *spi_device, uint8_t op, uint8_t address)
{
    uint8_t  send_buffer[2];
    uint8_t  recv_buffer[1];
    uint32_t send_size = 1;

    send_buffer[0] = op | (address & ADDR_MASK);
    send_buffer[1] = 0xFF;

    /* Do dummy read if needed (for mac and mii, see datasheet page 29). */
    if (address & 0x80)
    {
        send_size = 2;
    }

    os_spi_send_then_recv(spi_device, send_buffer, send_size, recv_buffer, 1);
    return (recv_buffer[0]);
}

static void spi_write_op(struct os_spi_device *spi_device, uint8_t op, uint8_t address, uint8_t data)
{
    uint8_t  buffer[2];

    buffer[0] = op | (address & ADDR_MASK);
    buffer[1] = data;
    os_spi_send(spi_device, buffer, 2);
}

static void enc28j60_clkout(struct os_spi_device *spi_device, os_uint8_t clk)
{
    /* Setup clkout: 2 is 12.5MHz: */
    spi_write(spi_device, ECOCON, clk & 0x7);
}

static void enc28j60_set_bank(struct os_spi_device *spi_device, uint8_t address)
{
    /* Set the bank (if needed) .*/
    if ((address & BANK_MASK) != Enc28j60Bank)
    {
        /* Set the bank. */
        spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
        spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK) >> 5);
        Enc28j60Bank = (address & BANK_MASK);
    }
}

static uint8_t spi_read(struct os_spi_device *spi_device, uint8_t address)
{
    /* Set the bank. */
    enc28j60_set_bank(spi_device, address);
    /* Do the read. */
    return spi_read_op(spi_device, ENC28J60_READ_CTRL_REG, address);
}

static void spi_write(struct os_spi_device *spi_device, os_uint8_t address, os_uint8_t data)
{
    /* Set the bank. */
    enc28j60_set_bank(spi_device, address);
    /* Do the write. */
    spi_write_op(spi_device, ENC28J60_WRITE_CTRL_REG, address, data);
}

static os_err_t enc28j60_wait_busy(struct os_spi_device *spi_device)
{
    int retry = 10000;

    while (--retry > 0)
    {
        if (!(spi_read(spi_device, MISTAT) & MISTAT_BUSY))
        {
            return OS_EOK;
        }

        _delay_us(10);
    }

    NET_DEBUG("enc28j60 wait timeout.\r\n");
    return OS_EBUSY;
}

static uint16_t enc28j60_phy_read(struct os_spi_device *spi_device, os_uint8_t address)
{
    uint16_t value;

    /* Set the right address and start the register read operation. */
    spi_write(spi_device, MIREGADR, address);
    spi_write(spi_device, MICMD, MICMD_MIIRD);

    _delay_us(15);

    /* Wait until the PHY read completes. */
    if (enc28j60_wait_busy(spi_device) == OS_EBUSY)
    {
        return 0xffff;
    }

    /* Reset reading bit */
    spi_write(spi_device, MICMD, 0x00);

    value = spi_read(spi_device, MIRDL) | spi_read(spi_device, MIRDH) << 8;

    return (value);
}

static void enc28j60_phy_write(struct os_spi_device *spi_device, os_uint8_t address, uint16_t data)
{
    /* Set the PHY register address. */
    spi_write(spi_device, MIREGADR, address);

    /* Write the PHY data. */
    spi_write(spi_device, MIWRL, data);
    spi_write(spi_device, MIWRH, data >> 8);

    /* Wait until the PHY write completes. */
    enc28j60_wait_busy(spi_device);
}

static uint32_t enc28j60_interrupt_disable(struct os_spi_device *spi_device)
{
    uint32_t level;

    /* Switch to bank 0 */
    enc28j60_set_bank(spi_device, EIE);

    /* Get last interrupt level */
    level = spi_read(spi_device, EIE);
    /* Disable interrutps */
    spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIE, level);

    return level;
}

static void enc28j60_interrupt_enable(struct os_spi_device *spi_device, uint32_t level)
{
    /* Switch to bank 0 */
    enc28j60_set_bank(spi_device, EIE);
    spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, EIE, level);
}

static os_bool_t enc28j60_check_link_status(struct os_spi_device *spi_device)
{
    uint16_t reg;

    reg = enc28j60_phy_read(spi_device, PHSTAT2);

    if (reg & PHSTAT2_LSTAT)
    {
        /* On */
        return OS_TRUE;
    }
    else
    {
        /* Off */
        return OS_FALSE;
    }
}

void enc28j60_isr(void)
{
    eth_device_ready(&enc28j60_dev.parent);
    NET_DEBUG("enc28j60_isr\r\n");
}

static void _tx_chain_init(void)
{
    enc28j60_tx_list[0].next = &enc28j60_tx_list[1];
    enc28j60_tx_list[1].next = &enc28j60_tx_list[0];

    enc28j60_tx_list[0].prev = &enc28j60_tx_list[1];
    enc28j60_tx_list[1].prev = &enc28j60_tx_list[0];

    enc28j60_tx_list[0].addr = TXSTART_INIT;
    enc28j60_tx_list[1].addr = TXSTART_INIT + MAX_TX_PACKAGE_SIZE;

    enc28j60_tx_list[0].free = OS_TRUE;
    enc28j60_tx_list[1].free = OS_TRUE;

    tx_current = &enc28j60_tx_list[0];
    tx_ack     = tx_current;
}

static os_err_t enc28j60_init(os_device_t *dev)
{
    struct net_device    *enc28j60   = (struct net_device *)dev;
    struct os_spi_device *spi_device = enc28j60->spi_device;
    os_uint8_t reg_val;

    enc28j60_lock(dev);

    _tx_chain_init();

    /* Perform system reset */
    spi_write_op(spi_device, ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    os_task_msleep(OS_TICK_PER_SECOND / 50); /* delay 20ms */

    NextPacketPtr = RXSTART_INIT;

    /* Rx start */
    spi_write(spi_device, ERXSTL, RXSTART_INIT & 0xFF);
    spi_write(spi_device, ERXSTH, RXSTART_INIT >> 8);
    /* Set receive pointer address */
    spi_write(spi_device, ERXRDPTL, RXSTOP_INIT & 0xFF);
    spi_write(spi_device, ERXRDPTH, RXSTOP_INIT >> 8);
    /* RX end */
    spi_write(spi_device, ERXNDL, RXSTOP_INIT & 0xFF);
    spi_write(spi_device, ERXNDH, RXSTOP_INIT >> 8);

    /* TX start */
    spi_write(spi_device, ETXSTL, TXSTART_INIT & 0xFF);
    spi_write(spi_device, ETXSTH, TXSTART_INIT >> 8);
    /* Set transmission pointer address */
    spi_write(spi_device, EWRPTL, TXSTART_INIT & 0xFF);
    spi_write(spi_device, EWRPTH, TXSTART_INIT >> 8);
    /* TX end */
    spi_write(spi_device, ETXNDL, TXSTOP_INIT & 0xFF);
    spi_write(spi_device, ETXNDH, TXSTOP_INIT >> 8);

    /*
     * do bank 1 stuff, packet filter:
     * For broadcast packets we allow only ARP packtets
     * All other packets should be unicast only for our mac (MAADR)
     *
     * The pattern to match on is therefore
     * Type     ETH.DST
     * ARP      BROADCAST
     * 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
     * in binary these poitions are:11 0000 0011 1111
     * This is hex 303F->EPMM0=0x3f,EPMM1=0x30
     */
    reg_val = ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_BCEN;
#if LWIP_IGMP
    reg_val |= ERXFCON_MCEN;
#endif
    spi_write(spi_device, ERXFCON, reg_val);

    /*
     * do bank 2 stuff
     * enable MAC receive
     */
    spi_write(spi_device, MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    /* Enable automatic padding to 60bytes and CRC operations */
    /* spi_write_op(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN); */
    spi_write_op(spi_device,
                 ENC28J60_BIT_FIELD_SET,
                 MACON3,
                 MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);

    /* set inter-frame gap (back-to-back) */
    /* spi_write(MABBIPG, 0x12); */
    spi_write(spi_device, MABBIPG, 0x15);

    spi_write(spi_device, MACON4, MACON4_DEFER);
    spi_write(spi_device, MACLCON2, 63);

    /* set inter-frame gap (non-back-to-back) */
    spi_write(spi_device, MAIPGL, 0x12);
    spi_write(spi_device, MAIPGH, 0x0C);

    /*
     * Set the maximum packet size which the controller will accept
     * Do not send packets longer than MAX_FRAMELEN:
     */
    spi_write(spi_device, MAMXFLL, MAX_FRAMELEN & 0xFF);
    spi_write(spi_device, MAMXFLH, MAX_FRAMELEN >> 8);

    /*
     * do bank 3 stuff
     * write MAC address
     * NOTE: MAC address in ENC28J60 is byte-backward
     */
    spi_write(spi_device, MAADR0, enc28j60->dev_addr[5]);
    spi_write(spi_device, MAADR1, enc28j60->dev_addr[4]);
    spi_write(spi_device, MAADR2, enc28j60->dev_addr[3]);
    spi_write(spi_device, MAADR3, enc28j60->dev_addr[2]);
    spi_write(spi_device, MAADR4, enc28j60->dev_addr[1]);
    spi_write(spi_device, MAADR5, enc28j60->dev_addr[0]);

    /* Output off */
    spi_write(spi_device, ECOCON, 0x00);

    /* enc28j60_phy_write(PHCON1, 0x00); */
    enc28j60_phy_write(spi_device, PHCON1, PHCON1_PDPXMD); /* Full duplex */
    /* No loopback of transmitted frames */
    enc28j60_phy_write(spi_device, PHCON2, PHCON2_HDLDIS);
    /* Enable PHY link changed interrupt. */
    enc28j60_phy_write(spi_device, PHIE, PHIE_PGEIE | PHIE_PLNKIE);

    enc28j60_set_bank(spi_device, ECON2);
    spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON2, ECON2_AUTOINC);

    /* Switch to bank 0 */
    enc28j60_set_bank(spi_device, ECON1);
    /* Enable all interrutps */
    spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, EIE, 0xFF);
    /* Enable packet reception */
    spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    /* Clock out */
    enc28j60_clkout(spi_device, 2);

    enc28j60_phy_write(spi_device, PHLCON, 0xD76); /* 0x476 */
    os_task_msleep(OS_TICK_PER_SECOND / 50);       /* Delay 20ms */

    enc28j60_unlock(dev);
    return OS_EOK;
}

static os_err_t enc28j60_control(os_device_t *dev, int cmd, void *args)
{
    struct net_device *enc28j60 = (struct net_device *)dev;
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* Get mac address */
        if (args)
            memcpy(args, enc28j60->dev_addr, 6);
        else
            return OS_ERROR;
        break;

    default:
        break;
    }

    return OS_EOK;
}

static os_err_t enc28j60_deinit(os_device_t *dev)
{
    return OS_EOK;
}

static os_size_t enc28j60_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_set_errno(OS_ENOSYS);
    return OS_EOK;
}

static os_size_t enc28j60_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_set_errno(OS_ENOSYS);
    return 0;
}

static os_err_t enc28j60_tx(os_device_t *dev, struct pbuf *p)
{
    struct net_device    *enc28j60   = (struct net_device *)dev;
    struct os_spi_device *spi_device = enc28j60->spi_device;
    struct pbuf          *q;
    os_uint32_t           level;
#ifdef ETH_TX_DUMP
    os_size_t   dump_count = 0;
    os_uint8_t *dump_ptr;
    os_size_t   dump_i;
#endif

    os_sem_wait(&tx_sem, OS_WAIT_FOREVER);

    OS_ASSERT(tx_current->free == OS_TRUE);

    enc28j60_lock(dev);

    /* Disable enc28j60 interrupt */
    level = enc28j60_interrupt_disable(spi_device);

    /* Set the write pointer to start of transmit buffer area */
    /*
    spi_write(EWRPTL, TXSTART_INIT&0xFF);
    spi_write(EWRPTH, TXSTART_INIT>>8);
    */
    spi_write(spi_device, EWRPTL, (tx_current->addr) & 0xFF);
    spi_write(spi_device, EWRPTH, (tx_current->addr) >> 8);
    /* Set the TXND pointer to correspond to the packet size given */
    tx_current->len = p->tot_len;
    /*
    spi_write(ETXNDL, (TXSTART_INIT+ p->tot_len + 1)&0xFF);
    spi_write(ETXNDH, (TXSTART_INIT+ p->tot_len + 1)>>8);
    */
    /* write per-packet control byte (0x00 means use macon3 settings) */
    spi_write_op(spi_device, ENC28J60_WRITE_BUF_MEM, 0, 0x00);

#ifdef ETH_TX_DUMP
    NET_DEBUG("tx_dump, size:%d\r\n", p->tot_len);
#endif
    for (q = p; q != NULL; q = q->next)
    {
        uint8_t cmd = ENC28J60_WRITE_BUF_MEM;
        os_spi_send_then_send(enc28j60->spi_device, &cmd, 1, q->payload, q->len);
#ifdef ETH_RX_DUMP
        dump_ptr = q->payload;
        for (dump_i = 0; dump_i < q->len; dump_i++)
        {
            NET_DEBUG("%02x ", *dump_ptr);
            if (((dump_count + 1) % 8) == 0)
            {
                NET_DEBUG("  ");
            }
            if (((dump_count + 1) % 16) == 0)
            {
                NET_DEBUG("\r\n");
            }
            dump_count++;
            dump_ptr++;
        }
#endif
    }
#ifdef ETH_RX_DUMP
    NET_DEBUG("\r\n");
#endif

    /* send the contents of the transmit buffer onto the network */
    if (tx_current == tx_ack)
    {
        NET_DEBUG("[Tx] stop, restart!\r\n");
        /* TX start */
        spi_write(spi_device, ETXSTL, (tx_current->addr) & 0xFF);
        spi_write(spi_device, ETXSTH, (tx_current->addr) >> 8);
        /* TX end */
        spi_write(spi_device, ETXNDL, (tx_current->addr + tx_current->len) & 0xFF);
        spi_write(spi_device, ETXNDH, (tx_current->addr + tx_current->len) >> 8);

        spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    }
    else
    {
        NET_DEBUG("[Tx] busy, add to chain!\r\n");
    }

    tx_current->free = OS_FALSE;
    tx_current       = tx_current->next;

    /* Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12. */
    if ((spi_read(spi_device, EIR) & EIR_TXERIF))
    {
        spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }

    /* Enable enc28j60 interrupt */
    enc28j60_interrupt_enable(spi_device, level);

    enc28j60_unlock(dev);

    return OS_EOK;
}

static struct pbuf *enc28j60_rx(os_device_t *dev)
{
    struct net_device    *enc28j60   = (struct net_device *)dev;
    struct os_spi_device *spi_device = enc28j60->spi_device;
    struct pbuf          *p          = OS_NULL;

    uint8_t     eir, eir_clr;
    uint32_t    pk_counter;
    os_uint32_t level;
    os_uint32_t len;
    os_uint16_t rxstat;

    enc28j60_lock(dev);

    /* Disable enc28j60 interrupt */
    level = enc28j60_interrupt_disable(spi_device);

    /* Get EIR */
    eir = spi_read(spi_device, EIR);

    while (eir & ~EIR_PKTIF)
    {
        eir_clr = 0;

        /* Clear PKTIF */
        if (eir & EIR_PKTIF)
        {
            NET_DEBUG("EIR_PKTIF\r\n");

            /* Switch to bank 0. */
            enc28j60_set_bank(spi_device, EIE);
            /* Disable rx interrutps. */
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIE, EIE_PKTIE);
            eir_clr |= EIR_PKTIF;
            /*
            enc28j60_set_bank(spi_device, EIR);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_PKTIF);
            */
        }

        /* Clear DMAIF */
        if (eir & EIR_DMAIF)
        {
            NET_DEBUG("EIR_DMAIF\r\n");
            eir_clr |= EIR_DMAIF;
            /*
            enc28j60_set_bank(spi_device, EIR);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_DMAIF);
            */
        }

        /* LINK changed handler */
        if (eir & EIR_LINKIF)
        {
            os_bool_t link_status;

            NET_DEBUG("EIR_LINKIF\r\n");
            link_status = enc28j60_check_link_status(spi_device);

            /* Read PHIR to clear the flag */
            enc28j60_phy_read(spi_device, PHIR);
            eir_clr |= EIR_LINKIF;
            /*
            enc28j60_set_bank(spi_device, EIR);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_LINKIF);
            */

            eth_device_linkchange(&(enc28j60->parent), link_status);
        }

        if (eir & EIR_TXIF)
        {
            /* A frame has been transmitted. */
            enc28j60_set_bank(spi_device, EIR);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXIF);

            tx_ack->free = OS_TRUE;
            tx_ack       = tx_ack->next;
            if (tx_ack->free == OS_FALSE)
            {
                NET_DEBUG("[tx isr] Tx chain not empty, continue send the next pkt!\r\n");
                /* TX start */
                spi_write(spi_device, ETXSTL, (tx_ack->addr) & 0xFF);
                spi_write(spi_device, ETXSTH, (tx_ack->addr) >> 8);
                /* TX end */
                spi_write(spi_device, ETXNDL, (tx_ack->addr + tx_ack->len) & 0xFF);
                spi_write(spi_device, ETXNDH, (tx_ack->addr + tx_ack->len) >> 8);

                spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
            }
            else
            {
                NET_DEBUG("[tx isr] Tx chain empty, stop!\r\n");
                os_sem_post(&tx_sem);
            }
        }

        /* Wake up handler */
        if (eir & EIR_WOLIF)
        {
            NET_DEBUG("EIR_WOLIF\r\n");
            eir_clr |= EIR_WOLIF;
            /*
            enc28j60_set_bank(spi_device, EIR);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_WOLIF);
            */
        }

        /* TX Error handler */
        if ((eir & EIR_TXERIF) != 0)
        {
            NET_DEBUG("EIR_TXERIF re-start tx chain!\r\n");
            enc28j60_set_bank(spi_device, ECON1);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
            eir_clr |= EIR_TXERIF;
            /*
            enc28j60_set_bank(spi_device, EIR);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF);
            */

            /* Re-init tx chain */
            _tx_chain_init();
        }

        /* RX Error handler */
        if ((eir & EIR_RXERIF) != 0)
        {
            NET_DEBUG("EIR_RXERIF re-start rx!\r\n");

            NextPacketPtr = RXSTART_INIT;
            enc28j60_set_bank(spi_device, ECON1);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXRST);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXRST);
            /* Switch to bank 0. */
            enc28j60_set_bank(spi_device, ECON1);
            /* Enable packet reception. */
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
            eir_clr |= EIR_RXERIF;
            /*
            enc28j60_set_bank(spi_device, EIR);
            spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_RXERIF);
            */
        }

        enc28j60_set_bank(spi_device, EIR);
        spi_write_op(spi_device, ENC28J60_BIT_FIELD_CLR, EIR, eir_clr);

        eir = spi_read(spi_device, EIR);
    }

    /* Read pkt */
    pk_counter = spi_read(spi_device, EPKTCNT);
    if (pk_counter)
    {
        /* Set the read pointer to the start of the received packet. */
        spi_write(spi_device, ERDPTL, (NextPacketPtr));
        spi_write(spi_device, ERDPTH, (NextPacketPtr) >> 8);

        /* Read the next packet pointer. */
        NextPacketPtr = spi_read_op(spi_device, ENC28J60_READ_BUF_MEM, 0);
        NextPacketPtr |= spi_read_op(spi_device, ENC28J60_READ_BUF_MEM, 0) << 8;

        /* Read the packet length (see datasheet page 43). */
        len = spi_read_op(spi_device, ENC28J60_READ_BUF_MEM, 0);       /* 0x54 */
        len |= spi_read_op(spi_device, ENC28J60_READ_BUF_MEM, 0) << 8; /* 5554 */

        len -= 4; /* Remove the CRC count */

        /* Read the receive status (see datasheet page 43) */
        rxstat = spi_read_op(spi_device, ENC28J60_READ_BUF_MEM, 0);
        rxstat |= ((os_uint16_t)spi_read_op(spi_device, ENC28J60_READ_BUF_MEM, 0)) << 8;

        /*
         * check CRC and symbol errors (see datasheet page 44, table 7-3):
         * The ERXFCON.CRCEN is set by default. Normally we should not
         * need to check this.
         */
        if ((rxstat & 0x80) == 0)
        {
            /* Invalid */
            len = 0;
        }
        else
        {
            /* Allocation pbuf */
            p = pbuf_alloc(PBUF_LINK, len, PBUF_POOL);
            if (p != OS_NULL)
            {
                struct pbuf *q;
#ifdef ETH_RX_DUMP
                os_size_t   dump_count = 0;
                os_uint8_t *dump_ptr;
                os_size_t   dump_i;
                NET_DEBUG("rx_dump, size:%d\r\n", len);
#endif
                for (q = p; q != OS_NULL; q = q->next)
                {
                    uint8_t cmd = ENC28J60_READ_BUF_MEM;
                    os_spi_send_then_recv(spi_device, &cmd, 1, q->payload, q->len);
#ifdef ETH_RX_DUMP
                    dump_ptr = q->payload;
                    for (dump_i = 0; dump_i < q->len; dump_i++)
                    {
                        NET_DEBUG("%02x ", *dump_ptr);
                        if (((dump_count + 1) % 8) == 0)
                        {
                            NET_DEBUG("  ");
                        }
                        if (((dump_count + 1) % 16) == 0)
                        {
                            NET_DEBUG("\r\n");
                        }
                        dump_count++;
                        dump_ptr++;
                    }
#endif
                }
#ifdef ETH_RX_DUMP
                NET_DEBUG("\r\n");
#endif
            }
        }

        /*
         * Move the RX read pointer to the start of the next received packet.
         * This frees the memory we just read out.
         */
        spi_write(spi_device, ERXRDPTL, (NextPacketPtr));
        spi_write(spi_device, ERXRDPTH, (NextPacketPtr) >> 8);

        /* Decrement the packet counter indicate we are done with this packet. */
        spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    }
    else
    {
        /* Switch to bank 0. */
        enc28j60_set_bank(spi_device, ECON1);
        /* Enable packet reception. */
        spi_write_op(spi_device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

        level |= EIE_PKTIE;
    }

    /* Enable enc28j60 interrupt */
    enc28j60_interrupt_enable(spi_device, level);

    enc28j60_unlock(dev);

    return p;
}

const static struct os_device_ops enc28j60_ops = 
{
    .init    = enc28j60_init,
    .deinit  = enc28j60_deinit,
    .read    = enc28j60_read,
    .write   = enc28j60_write,
    .control = enc28j60_control
};

os_err_t enc28j60_attach(const char *spi_device_name)
{
    struct os_spi_device *spi_device;

    spi_device = (struct os_spi_device *)os_device_find(spi_device_name);
    if (spi_device == OS_NULL)
    {
        NET_DEBUG("spi device %s not found!\r\n", spi_device_name);
        return OS_ENOSYS;
    }

    /* Config spi */
    {
        struct os_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode       = OS_SPI_MODE_0 | OS_SPI_MSB; /* SPI Compatible Modes 0 */
        cfg.max_hz     = 20 * 1000 * 1000;           /* SPI Interface with Clock Speeds Up to 20 MHz */
        os_spi_configure(spi_device, &cfg);
    } /* Config spi */

    memset(&enc28j60_dev, 0, sizeof(enc28j60_dev));

    /* Detect device */
    {
        uint16_t value;

        /* Perform system reset. */
        spi_write_op(spi_device, ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
        os_task_msleep(1); /* Delay 20ms */

        enc28j60_dev.emac_rev = spi_read(spi_device, EREVID);

        if (enc28j60_dev.emac_rev == 0xff)
        {
            NET_DEBUG("ENC28J60 HwRevID invalid!\r\n");
            return OS_EIO;
        }

        value                = enc28j60_phy_read(spi_device, PHHID2);
        enc28j60_dev.phy_rev = value & 0x0F;
        enc28j60_dev.phy_pn  = (value >> 4) & 0x3F;
        enc28j60_dev.phy_id  = (enc28j60_phy_read(spi_device, PHHID1) | ((value >> 10) << 16)) << 3;

        if (enc28j60_dev.phy_id != 0x00280418)
        {
            NET_DEBUG("ENC28J60 PHY ID not correct!\r\n");
            NET_DEBUG("emac_rev:%d\r\n", enc28j60_dev.emac_rev);
            NET_DEBUG("phy_rev:%02X\r\n", enc28j60_dev.phy_rev);
            NET_DEBUG("phy_pn:%02X\r\n", enc28j60_dev.phy_pn);
            NET_DEBUG("phy_id:%08X\r\n", enc28j60_dev.phy_id);
            return OS_EIO;
        }
    }

    /* OUI 00-04-A3 (hex): Microchip Technology, Inc. */
    enc28j60_dev.dev_addr[0] = 0x00;
    enc28j60_dev.dev_addr[1] = 0x04;
    enc28j60_dev.dev_addr[2] = 0xA3;
    /* set MAC address, only for test */
    enc28j60_dev.dev_addr[3] = 0x12;
    enc28j60_dev.dev_addr[4] = 0x34;
    enc28j60_dev.dev_addr[5] = 0x56;

    /* init device struct */
    enc28j60_dev.parent.parent.type = OS_DEVICE_TYPE_NETIF;
    enc28j60_dev.parent.parent.ops = &enc28j60_ops;

    /* init ethernet device struct */
    enc28j60_dev.parent.eth_rx = enc28j60_rx;
    enc28j60_dev.parent.eth_tx = enc28j60_tx;

    enc28j60_dev.spi_device = spi_device;
    os_sem_init(&tx_sem, "eth_tx", 1, 1);
    os_mutex_init(&enc28j60_dev.lock, "enc28j60", OS_FALSE);

    eth_device_init(&(enc28j60_dev.parent), "e0");

    return OS_EOK;
}

int os_hw_enc28j60_init(void)
{
    int ret;

    os_hw_spi_device_attach(BSP_ENC28J60_SPI_BUS, BSP_ENC28J60_SPI_DEV, BSP_ENC28J60_SPI_CS);

    os_pin_mode(BSP_ENC28J60_RST, PIN_MODE_OUTPUT);
    os_pin_write(BSP_ENC28J60_RST, PIN_LOW);
    os_task_msleep(10);
    os_pin_write(BSP_ENC28J60_RST, PIN_HIGH);

    ret = enc28j60_attach(BSP_ENC28J60_SPI_DEV);
    if (ret != 0)
    {
        os_kprintf("enc28j60 attach failed %d.\r\n", ret);
        return 0;
    }

    /* Init interrupt pin */
    os_pin_mode(BSP_ENC28J60_IRQ, PIN_MODE_INPUT_PULLUP);
    os_pin_attach_irq(BSP_ENC28J60_IRQ, PIN_IRQ_MODE_FALLING, (void (*)(void *))enc28j60_isr, NULL);
    os_pin_irq_enable(BSP_ENC28J60_IRQ, PIN_IRQ_ENABLE);

    os_kprintf("enc28j60 attach success.\r\n");
    return 0;
}
OS_CMPOENT_INIT(os_hw_enc28j60_init, OS_INIT_SUBLEVEL_LOW);

#ifdef OS_USING_SHELL
#include <shell.h>

static void enc28j60(void)
{
    struct os_spi_device *spi_device = enc28j60_dev.spi_device;

    if (spi_device == OS_NULL)
    {
        os_kprintf("enc28j60 null.\r\n");
        return;
    }

    enc28j60_lock(&enc28j60_dev);

    os_kprintf("-- enc28j60 registers:\r\n");
    os_kprintf("HwRevID: 0x%02X\r\n", spi_read(spi_device, EREVID));

    os_kprintf("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\r\n");
    os_kprintf("       0x%02X  0x%02X  0x%02X  0x%02X  0x%02X\r\n",
               spi_read(spi_device, ECON1),
               spi_read(spi_device, ECON2),
               spi_read(spi_device, ESTAT),
               spi_read(spi_device, EIR),
               spi_read(spi_device, EIE));

    os_kprintf("MAC  : MACON1 MACON3 MACON4\r\n");
    os_kprintf("       0x%02X   0x%02X   0x%02X\r\n",
               spi_read(spi_device, MACON1),
               spi_read(spi_device, MACON3),
               spi_read(spi_device, MACON4));

    os_kprintf("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\r\n");
    os_kprintf("       0x%04X 0x%04X 0x%04X  0x%04X  ",
               (spi_read(spi_device, ERXSTH) << 8) | spi_read(spi_device, ERXSTL),
               (spi_read(spi_device, ERXNDH) << 8) | spi_read(spi_device, ERXNDL),
               (spi_read(spi_device, ERXWRPTH) << 8) | spi_read(spi_device, ERXWRPTL),
               (spi_read(spi_device, ERXRDPTH) << 8) | spi_read(spi_device, ERXRDPTL));

    os_kprintf("0x%02X    0x%02X    0x%04X\r\n",
               spi_read(spi_device, ERXFCON),
               spi_read(spi_device, EPKTCNT),
               (spi_read(spi_device, MAMXFLH) << 8) | spi_read(spi_device, MAMXFLL));

    os_kprintf("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\r\n");
    os_kprintf("       0x%04X 0x%04X 0x%02X     0x%02X     0x%02X\r\n",
               (spi_read(spi_device, ETXSTH) << 8) | spi_read(spi_device, ETXSTL),
               (spi_read(spi_device, ETXNDH) << 8) | spi_read(spi_device, ETXNDL),
               spi_read(spi_device, MACLCON1),
               spi_read(spi_device, MACLCON2),
               spi_read(spi_device, MAPHSUP));

    os_kprintf("PHY  : PHCON1 PHSTAT1\r\n");
    os_kprintf("       0x%04X 0x%04X\r\n",
               enc28j60_phy_read(spi_device, PHCON1),
               enc28j60_phy_read(spi_device, PHSTAT1));

    enc28j60_unlock(&enc28j60_dev);
}
SH_CMD_EXPORT(enc28j60, enc28j60, "dump enc28j60 registers");
#endif

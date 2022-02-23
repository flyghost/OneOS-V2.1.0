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
#include <os_irq.h>
#include <os_memory.h>
#include <string.h>
#include <bus/bus.h>

/*
 * Emac driver uses CubeMX tool to generate emac and phy's configuration,
 * the configuration files can be found in CubeMX_Config folder.
 */

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.eth"
#include <drv_log.h>

#ifdef PHY_USING_RMII

#define RMII_MODE
#else
#define MII_MODE
#endif

#define MAX_ADDR_LEN 6

struct os_gd32_eth
{
    struct eth_device parent;          /* inherit from ethernet device */
#ifndef PHY_USING_INTERRUPT_MODE
    os_timer_t *poll_link_timer;
#endif
    os_uint8_t dev_addr[MAX_ADDR_LEN]; /* interface address info, hw address */
    uint32_t   ETH_Speed;              /* ETH_Speed_Duplex_Mode */
    uint32_t   ETH_Mode;              /* ETH_Speed_Duplex_Mode */
};

/* ENET RxDMA/TxDMA descriptor */
extern enet_descriptors_struct  txdesc_tab[ENET_TXBUF_NUM];
extern enet_descriptors_struct  rxdesc_tab[ENET_RXBUF_NUM];

/*global transmit and receive descriptors pointers */
extern enet_descriptors_struct  *dma_current_txdesc;
extern enet_descriptors_struct  *dma_current_rxdesc;

//static ETH_HandleTypeDef  *EthHandle;
static struct os_gd32_eth gd32_eth_device;

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
        os_kprintf("\n");
    }
}
#endif

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
void enet_system_setup(void);
static void phy_linkchange(void);

os_err_t os_gd32_eth_init(os_device_t *dev)
{
    int i; 
    
    enet_system_setup();
    
    enet_mac_address_set(ENET_MAC_ADDRESS0, &gd32_eth_device.dev_addr[0]);
    
    enet_descriptors_chain_init(ENET_DMA_TX);
    enet_descriptors_chain_init(ENET_DMA_RX);
    
    /* enable ethernet Rx interrrupt */
    for(i=0; i<ENET_RXBUF_NUM; i++)
    { 
        enet_rx_desc_immediate_receive_complete_interrupt(&rxdesc_tab[i]);
    }
#ifdef LWIP_USING_HW_CHECKSUM
    /* enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
    for(i=0; i < ENET_TXBUF_NUM; i++){
        enet_transmit_checksum_config(&txdesc_tab[i], ENET_CHECKSUM_TCPUDPICMP_FULL);
    }
#endif /* LWIP_USING_HW_CHECKSUM */

    /* enable MAC and DMA transmission and reception */
    enet_enable();  
    
    phy_linkchange();
    
    return OS_EOK;
}

static os_err_t os_gd32_eth_open(os_device_t *dev, os_uint16_t oflag)
{
    LOG_EXT_D("emac open");
    return OS_EOK;
}

static os_err_t os_gd32_eth_close(os_device_t *dev)
{
    LOG_EXT_D("emac close");
    return OS_EOK;
}

static os_size_t os_gd32_eth_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    LOG_EXT_D("emac read");
    os_set_errno(OS_ENOSYS);
    return 0;
}

static os_size_t os_gd32_eth_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    LOG_EXT_D("emac write");
    os_set_errno(OS_ENOSYS);
    return 0;
}

static os_err_t os_gd32_eth_control(os_device_t *dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args)
            memcpy(args, gd32_eth_device.dev_addr, 6);
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
os_err_t os_gd32_eth_tx(os_device_t *dev, struct pbuf *p)
{
    os_err_t          reval = OS_ERROR;
    struct pbuf      *q;
    uint8_t *buffer ;
    uint16_t framelength = 0;
    
    /* copy frame from pbufs to driver buffers */
    SYS_ARCH_DECL_PROTECT(sr);
    
    SYS_ARCH_PROTECT(sr);
  
    while((uint32_t)RESET != (dma_current_txdesc->status & ENET_TDES0_DAV)){
    }    
    buffer = (uint8_t *)(enet_desc_information_get(dma_current_txdesc, TXDESC_BUFFER_1_ADDR));

    for(q = p; q != NULL; q = q->next){ 
        memcpy((uint8_t *)&buffer[framelength], q->payload, q->len);
        framelength = framelength + q->len;
    }

   /* transmit descriptors to give to DMA */ 
#ifdef SELECT_DESCRIPTORS_ENHANCED_MODE
    reval = ENET_NOCOPY_PTPFRAME_TRANSMIT_ENHANCED_MODE(framelength, NULL);
#else
    reval = ENET_NOCOPY_FRAME_TRANSMIT(framelength);
#endif /* SELECT_DESCRIPTORS_ENHANCED_MODE */

    SYS_ARCH_UNPROTECT(sr);
    return reval;

}



/* receive data */
struct pbuf *os_gd32_eth_rx(os_device_t *dev)
{
    struct pbuf *p= NULL, *q;
    uint32_t l =0;
    u16_t len;
    uint8_t *buffer;

    /* obtain the size of the packet and put it into the "len" variable. */
    len = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);
    buffer = (uint8_t *)(enet_desc_information_get(dma_current_rxdesc, RXDESC_BUFFER_1_ADDR));
  
    if (len > 0){
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }
  
    if (p != NULL){
        for(q = p; q != NULL; q = q->next){
            memcpy((uint8_t *)q->payload, (u8_t*)&buffer[l], q->len);
            l = l + q->len;
        }
    }
#ifdef SELECT_DESCRIPTORS_ENHANCED_MODE
    ENET_NOCOPY_PTPFRAME_RECEIVE_ENHANCED_MODE(NULL);
  
#else
    
    ENET_NOCOPY_FRAME_RECEIVE();
#endif /* SELECT_DESCRIPTORS_ENHANCED_MODE */

    return p;

}

/*!
    \brief      this function handles ethernet interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ENET_IRQHandler(void)
{
    os_interrupt_enter();
    os_err_t result;
    

    /* frame received */
    if(SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_RS))
    { 
        result = eth_device_ready(&(gd32_eth_device.parent));
        if (result != OS_EOK)
            LOG_EXT_I("RxCpltCallback err = %d", result);
    }

    /* clear the enet DMA Rx interrupt pending bits */
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);

    
    os_interrupt_leave();
}

enum
{
    PHY_LINK        = (1 << 0),
    PHY_100M        = (1 << 1),
    PHY_FULL_DUPLEX = (1 << 2),
};

#define ETH_SPEED_10M        0x00000000U
#define ETH_SPEED_100M       0x00004000U
#define ETH_MODE_FULLDUPLEX       0x00000800U
#define ETH_MODE_HALFDUPLEX       0x00000000U

static void phy_linkchange(void)
{
    static os_uint8_t phy_speed     = 0;
    os_uint8_t        phy_speed_new = 0;
    os_uint16_t       status;
    
    /* read the result of the auto-negotiation */
    enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_BSR, &status);
    LOG_EXT_D("phy basic status reg is 0x%X", status);
    /* configure the duplex mode of MAC following the auto-negotiation result */
    if (status & (PHY_AUTONEGO_COMPLETE_MASK | PHY_LINKED_STATUS_MASK))
    {   
        os_uint16_t SR = 0;

        phy_speed_new |= PHY_LINK;
        
        enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_SR, &SR);
        LOG_EXT_D("phy control status reg is 0x%X", SR);
                
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
            LOG_EXT_D("link up");
            if (phy_speed & PHY_100M)
            {
                LOG_EXT_D("100Mbps");
                gd32_eth_device.ETH_Speed = ETH_SPEED_100M;
            }
            else
            {
                gd32_eth_device.ETH_Speed = ETH_SPEED_10M;
                LOG_EXT_D("10Mbps");
            }

            if (phy_speed & PHY_FULL_DUPLEX)
            {
                LOG_EXT_D("full-duplex");
                gd32_eth_device.ETH_Mode = ETH_MODE_FULLDUPLEX;
            }
            else
            {
                LOG_EXT_D("half-duplex");
                gd32_eth_device.ETH_Mode = ETH_MODE_HALFDUPLEX;
            }

            /* send link up. */
            eth_device_linkchange(&gd32_eth_device.parent, OS_TRUE);
        }
        else
        {
            LOG_EXT_I("link down");
            eth_device_linkchange(&gd32_eth_device.parent, OS_FALSE);
        }
    }
}

#ifdef PHY_USING_INTERRUPT_MODE
static void eth_phy_isr(void *args)
{
    os_uint32_t status = 0;

    HAL_ETH_ReadPHYRegister(EthHandle, PHY_INTERRUPT_FLAG_REG, (uint32_t *)&status);
    LOG_EXT_D("phy interrupt status reg is 0x%X", status);

    phy_linkchange();
}
#endif /* PHY_USING_INTERRUPT_MODE */

#if 0
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
        os_task_mdelay(1000);

        if (detected_count > 10)
        {
            LOG_EXT_E("No PHY device was detected, please check hardware!");
        }
    }

    LOG_EXT_D("Found a phy, address:0x%02X", phy_addr);

    /* RESET PHY */
    LOG_EXT_D("RESET PHY!");
    HAL_ETH_WritePHYRegister(EthHandle, PHY_BASIC_CONTROL_REG, PHY_RESET_MASK);
    os_task_mdelay(2000);
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
    gd32_eth_device.poll_link_timer =
        os_timer_create("phylnk", (void (*)(void *))phy_linkchange, NULL, OS_TICK_PER_SECOND, OS_TIMER_FLAG_PERIODIC);
    if (!gd32_eth_device.poll_link_timer || os_timer_start(gd32_eth_device.poll_link_timer) != OS_EOK)
    {
        LOG_EXT_E("Start link change detection timer failed");
    }
#endif /* PHY_USING_INTERRUPT_MODE */

}
#endif



/*!
    \brief      configures the ethernet interface
    \param[in]  none
    \param[out] none
    \retval     none
*/
static ErrStatus enet_mac_dma_config(void)
{
    ErrStatus reval_state = ERROR;
    
    /* enable ethernet clock  */
    rcu_periph_clock_enable(RCU_ENET);
    rcu_periph_clock_enable(RCU_ENETTX);
    rcu_periph_clock_enable(RCU_ENETRX);
    
    /* reset ethernet on AHB bus */
    enet_deinit();

    reval_state = enet_software_reset();
    if(ERROR == reval_state)
    {
        while(1)
        {
            
        }
    }
/* configure the parameters which are usually less cared for enet initialization */  
//  enet_initpara_config(HALFDUPLEX_OPTION, ENET_CARRIERSENSE_ENABLE|ENET_RECEIVEOWN_ENABLE|ENET_RETRYTRANSMISSION_DISABLE|ENET_BACKOFFLIMIT_10|ENET_DEFERRALCHECK_DISABLE);
//  enet_initpara_config(DMA_OPTION, ENET_FLUSH_RXFRAME_ENABLE|ENET_SECONDFRAME_OPT_ENABLE|ENET_NORMAL_DESCRIPTOR);

#ifdef LWIP_USING_HW_CHECKSUM
    enet_init_status = enet_init(ENET_AUTO_NEGOTIATION, ENET_AUTOCHECKSUM_DROP_FAILFRAMES, ENET_BROADCAST_FRAMES_PASS);
#else  
    reval_state = enet_init(ENET_AUTO_NEGOTIATION, ENET_NO_AUTOCHECKSUM, ENET_BROADCAST_FRAMES_PASS);
#endif /* LWIP_USING_HW_CHECKSUM */
    return reval_state;
}

/*!
    \brief      configures the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void nvic_configuration(void)
{
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x0);
    nvic_irq_enable(ENET_IRQn, 2, 0);
}

/*!
    \brief      configures the different GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void enet_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOG);
    rcu_periph_clock_enable(RCU_GPIOH);
    rcu_periph_clock_enable(RCU_GPIOI);
  
    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_8);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_8);
  
    /* enable SYSCFG clock */
    rcu_periph_clock_enable(RCU_SYSCFG);
  
#ifdef MII_MODE 
  
#ifdef PHY_CLOCK_MCO
    /* output HXTAL clock (25MHz) on CKOUT0 pin(PA8) to clock the PHY */
    rcu_ckout0_config(RCU_CKOUT0SRC_HXTAL, RCU_CKOUT0_DIV1);
#endif /* PHY_CLOCK_MCO */

    syscfg_enet_phy_interface_config(SYSCFG_ENET_PHY_MII);

#elif defined RMII_MODE
    /* choose DIV2 to get 50MHz from 200MHz on CKOUT0 pin (PA8) to clock the PHY */
    rcu_ckout0_config(RCU_CKOUT0SRC_PLLP, RCU_CKOUT0_DIV4);
    syscfg_enet_phy_interface_config(SYSCFG_ENET_PHY_RMII);

#endif /* MII_MODE */

#ifdef MII_MODE

    /* PA1: ETH_MII_RX_CLK */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_1);
    
    /* PA2: ETH_MDIO */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_2);
    
    /* PA7: ETH_MII_RX_DV */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_7);   
    
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);
    
    /* PG11: ETH_MII_TX_EN */
    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_11);
    
    /* PG13: ETH_MII_TXD0 */
    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_13);
    
    /* PG14: ETH_MII_TXD1 */
    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_14);   
    
    gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_11);
    gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_13);
    gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_14);

    /* PC1: ETH_MDC */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_1);
    
    /* PC2: ETH_MII_TXD2 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_2);
    
    /* PC3: ETH_MII_TX_CLK */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_3);  

    /* PC4: ETH_MII_RXD0 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_4);
    
    /* PC5: ETH_MII_RXD1 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_5); 
    
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_2);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_3);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);

    /* PH2: ETH_MII_CRS */
    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_2);
    
    /* PH3: ETH_MII_COL */
    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_3);
    
    /* PH6: ETH_MII_RXD2 */
    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_6);  

    /* PH7: ETH_MII_RXD3 */
    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_7);
    
    gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_2);
    gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_3);
    gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_6);
    gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_7);

    /* PI8: ETH_INT */
    gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_8);  

    /* PI10: ETH_MII_RX_ER */
    gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_10);

    gpio_af_set(GPIOI, GPIO_AF_11, GPIO_PIN_8);
    gpio_af_set(GPIOI, GPIO_AF_11, GPIO_PIN_10);

    /* PB8: ETH_MII_TXD3 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_8);
    
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_8);
      
#elif defined RMII_MODE

    /* PA1: ETH_RMII_REF_CLK */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_1);
    
    /* PA2: ETH_MDIO */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_2);
    
    /* PA7: ETH_RMII_CRS_DV */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_7);   
    
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);
    
    /* PB11: ETH_RMII_TX_EN */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_11);
    
    /* PB12: ETH_RMII_TXD0 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_12);
    
    /* PB13: ETH_RMII_TXD1 */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_13);   
    
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_11);
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_12);
    gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_13);

    /* PC1: ETH_MDC */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_1);

    /* PC4: ETH_RMII_RXD0 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_4);
    
    /* PC5: ETH_RMII_RXD1 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,GPIO_PIN_5); 
    
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
    gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);
#endif /* MII_MODE */
}

/*!
    \brief      setup ethernet system(GPIOs, clocks, MAC, DMA, systick)
    \param[in]  none
    \param[out] none
    \retval     none
*/
void enet_system_setup(void)
{
    ErrStatus enet_init_status = ERROR;
    nvic_configuration();   
  
    /* configure the GPIO ports for ethernet pins */
    enet_gpio_config();
    
    /* configure the ethernet MAC/DMA */
    enet_init_status = enet_mac_dma_config();

    if(ERROR == enet_init_status)
    {
        while(1)
        {
            
        }
    }
  
    enet_interrupt_enable(ENET_DMA_INT_NIE);
    enet_interrupt_enable(ENET_DMA_INT_RIE);
}

/**
 ***********************************************************************************************************************
 * @brief           os_hw_gd32_eth_init:init and register the EMAC device.
 *
 * @param[in]       none
 *
 * @return          Return eth init status.
 * @retval          ERR_OK            init success.
 * @retval          Others            init failed.
 ***********************************************************************************************************************
 */
#define UID_BASE              0x1FFFF7ACUL       /*!< Unique device ID register base address */
const static struct os_device_ops eth_ops = {
    os_gd32_eth_init,
    os_gd32_eth_open,
    os_gd32_eth_close,
    os_gd32_eth_read,
    os_gd32_eth_write,
    os_gd32_eth_control,
};
static int gd32_eth_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t state = OS_EOK;

    gd32_eth_device.ETH_Speed = ETH_SPEED_100M;
    gd32_eth_device.ETH_Mode = ETH_MODE_FULLDUPLEX;

    /* OUI 00-80-E1 STMICROELECTRONICS. */
    gd32_eth_device.dev_addr[0] = 0x00;
    gd32_eth_device.dev_addr[1] = 0x80;
    gd32_eth_device.dev_addr[2] = 0xE1;
    /* generate MAC addr from 96bit unique ID (only for test). */
    gd32_eth_device.dev_addr[3] = *(os_uint8_t *)(UID_BASE + 4);
    gd32_eth_device.dev_addr[4] = *(os_uint8_t *)(UID_BASE + 2);
    gd32_eth_device.dev_addr[5] = *(os_uint8_t *)(UID_BASE + 0);

    gd32_eth_device.parent.parent.ops = &eth_ops;

    gd32_eth_device.parent.eth_rx = os_gd32_eth_rx;
    gd32_eth_device.parent.eth_tx = os_gd32_eth_tx;

    
    /* register eth device */
    state = eth_device_init(&(gd32_eth_device.parent), "e0");
    if (OS_EOK == state)
    {
        LOG_EXT_D("emac device init success");
    }
    else
    {
        LOG_EXT_E("emac device init faild: %d", state);
        state = OS_ERROR;
        goto __exit;
    }

    /* start phy monitor */
    #if 0
    os_task_t *tid;
    tid = os_task_create("phy", phy_monitor_task_entry, OS_NULL, 1024, OS_TASK_PRIORITY_MAX - 2, 2);
    if (tid != OS_NULL)
    {
        os_task_startup(tid);
    }
    else
    {
        state = OS_ERROR;
    }
    #endif
__exit:

    return state;
}

OS_DRIVER_INFO gd32_eth_driver = {
    .name   = "ETH_Type",
    .probe  = gd32_eth_probe,
};

OS_DRIVER_DEFINE(gd32_eth_driver, "4");


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
#include <os_memory.h>
#include <string.h>
#include <bus/bus.h>

#include "fsl_enet.h"
#include "fsl_gpio.h"
#include "fsl_phy.h"
#include "fsl_cache.h"
#include "fsl_iomuxc.h"
#include "fsl_common.h"
#include "fsl_phy.h"
#include "fsl_enet_mdio.h"
#include "fsl_phyksz8081.h"
#include "fsl_ocotp.h"

#include <netif/ethernetif.h>
#include "lwipopts.h"

#include <drv_gpio.h>
#include <drv_eth.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.eth"
#include <dlog.h>

#ifndef PHY_INIT_COUNT
#define PHY_INIT_COUNT (10)
#endif

#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT (100000)
#endif

#ifdef BSP_USING_ETH1
AT_NONCACHEABLE_SECTION_ALIGN(enet_rx_bd_struct_t g_rxBuffDescrip1[ETH1_RXBUFF_NUM], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_tx_bd_struct_t g_txBuffDescrip1[ETH1_TXBUFF_NUM], ENET_BUFF_ALIGNMENT);

AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_rxDataBuff1[ETH1_RXBUFF_NUM][SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT)], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_txDataBuff1[ETH1_TXBUFF_NUM][SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT)], ENET_BUFF_ALIGNMENT);

enet_buffer_config_t buffConfig1 =
{
    ETH1_RXBUFF_NUM,
    ETH1_TXBUFF_NUM,
    SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT),
    SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT),
    &g_rxBuffDescrip1[0],
    &g_txBuffDescrip1[0],
    &g_rxDataBuff1[0][0],
    &g_txDataBuff1[0][0],
    true,
    true,
    NULL,
};

static const struct nxp_eth_info eth1_info = 
{
    .base                   = ENET,
    .phyAddr                = ETH1_PHY_ADDR,
    .reset_pin              = ETH1_RESET_PIN,
    .int_pin                = ETH1_INT_PIN,
    .buffConfig             = &buffConfig1,
    
#ifdef ETH1_USING_KSZ8081RNB
    .ops                    = &phyksz8081_ops
#endif
};
OS_HAL_DEVICE_DEFINE("ETH_Type", "eth1", eth1_info);
#endif

#ifdef BSP_USING_ETH2
AT_NONCACHEABLE_SECTION_ALIGN(enet_rx_bd_struct_t g_rxBuffDescrip2[ETH2_RXBUFF_NUM], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_tx_bd_struct_t g_txBuffDescrip2[ETH2_TXBUFF_NUM], ENET_BUFF_ALIGNMENT);

AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_rxDataBuff2[ETH2_RXBUFF_NUM][SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT)], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_txDataBuff2[ETH2_TXBUFF_NUM][SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT)], ENET_BUFF_ALIGNMENT);

enet_buffer_config_t buffConfig2 =
{
    ETH2_RXBUFF_NUM,
    ETH2_TXBUFF_NUM,
    SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT),
    SDK_SIZEALIGN(ENET_FRAME_MAX_FRAMELEN, ENET_BUFF_ALIGNMENT),
    &g_rxBuffDescrip2[0],
    &g_txBuffDescrip2[0],
    &g_rxDataBuff2[0][0],
    &g_txDataBuff2[0][0],
    true,
    true,
    NULL,
};

static const struct nxp_eth_info eth2_info = 
{
    .base                   = ENET2,
    .phyAddr                = ETH2_PHY_ADDR,
    .reset_pin              = ETH2_RESET_PIN,
    .int_pin                = ETH2_INT_PIN,
    .buffConfig             = &buffConfig2,
    
#ifdef ETH2_USING_KSZ8081RNB
    .ops                    = &phyksz8081_ops
#endif
};
OS_HAL_DEVICE_DEFINE("ETH_Type", "eth2", eth2_info);
#endif

struct os_imxrt_eth
{
    struct eth_device       dev;
    
    struct nxp_eth_info    *eth_info;
    mdio_handle_t           mdioHandle;
    phy_handle_t            phyHandle;
    enet_handle_t           enet_handle;
    
    enet_mii_speed_t        speed;
    enet_mii_duplex_t       duplex;
    os_uint32_t             sysClock;

    os_timer_t             *poll_link_timer;

    os_uint8_t              dev_addr[6];
    struct os_semaphore     tx_sem;
};

void _enet_rx_callback(struct os_imxrt_eth *imxrt_eth)
{
    os_err_t result;
    
    ENET_DisableInterrupts(imxrt_eth->eth_info->base, kENET_RxFrameInterrupt);

    result = eth_device_ready(&imxrt_eth->dev);
    if (result != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "eth_device_ready err %d", result);
    }
}

void _enet_tx_callback(struct os_imxrt_eth *imxrt_eth)
{
    if (imxrt_eth->tx_sem.count == 0)
    {
        os_sem_post(&imxrt_eth->tx_sem);
    }
}

void _enet_callback(ENET_Type *base, enet_handle_t *handle, enet_event_t event, enet_frame_info_t *frameInfo, void *userData)
{
    switch (event)
    {
    case kENET_RxEvent:
        _enet_rx_callback((struct os_imxrt_eth *)userData);
        break;

    case kENET_TxEvent:
        _enet_tx_callback((struct os_imxrt_eth *)userData);
        break;

    case kENET_ErrEvent:
        LOG_D(DRV_EXT_TAG, "kENET_ErrEvent");
        break;

    case kENET_WakeUpEvent:
        LOG_D(DRV_EXT_TAG, "kENET_WakeUpEvent");
        break;

    case kENET_TimeStampEvent:
        LOG_D(DRV_EXT_TAG, "kENET_TimeStampEvent");
        break;

    case kENET_TimeStampAvailEvent:
        LOG_D(DRV_EXT_TAG, "kENET_TimeStampAvailEvent");
        break;

    default:
        LOG_D(DRV_EXT_TAG, "unknow error");
        break;
    }
}

static void _phy_reset_by_gpio(struct os_imxrt_eth *imxrt_eth)
{
    if (imxrt_eth->eth_info->reset_pin >= 0)
    {
        os_pin_mode(imxrt_eth->eth_info->reset_pin, PIN_MODE_OUTPUT);

        if (imxrt_eth->eth_info->int_pin >= 0)
        {
            os_pin_mode(imxrt_eth->eth_info->int_pin, PIN_MODE_OUTPUT);
            /* pull up the ENET_INT before RESET. */
            os_pin_write(imxrt_eth->eth_info->int_pin, 1);
        }
        
        os_pin_write(imxrt_eth->eth_info->reset_pin, 0);
        os_task_msleep(1000);
        os_pin_write(imxrt_eth->eth_info->reset_pin, 1);
    }
}

os_err_t imxrt_eth_init(struct os_imxrt_eth *imxrt_eth)
{
    status_t status         = 0;
    os_uint32_t count_init  = PHY_INIT_COUNT;
    os_uint32_t count       = 0;
    bool link               = false;
    bool autonego           = false;

    enet_config_t config;
    phy_config_t  phyConfig = {0};
    
    clock_enet_pll_config_t pll_config = {.enableClkOutput = true, .loopDivider = 1};
    
#ifdef ENET
    if (imxrt_eth->eth_info->base == ENET)
    {
        CLOCK_InitEnetPll(&pll_config);
        IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);
    }
#endif
#ifdef ENET2
    if (imxrt_eth->eth_info->base == ENET2)
    {
        pll_config.enableClkOutput1 = true;
        pll_config.loopDivider1 = 1;
        CLOCK_InitEnetPll(&pll_config);
        IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET2TxClkOutputDir, true);
    }
#endif
    
    _phy_reset_by_gpio(imxrt_eth);
    
    imxrt_eth->mdioHandle.ops                   = &enet_ops;
    imxrt_eth->mdioHandle.resource.base         = imxrt_eth->eth_info->base;
    imxrt_eth->mdioHandle.resource.csrClock_Hz  = CLOCK_GetFreq(kCLOCK_IpgClk);

    imxrt_eth->phyHandle.phyAddr                = imxrt_eth->eth_info->phyAddr;
    imxrt_eth->phyHandle.mdioHandle             = &imxrt_eth->mdioHandle;
    imxrt_eth->phyHandle.ops                    = imxrt_eth->eth_info->ops;
    
    phyConfig.phyAddr                           = imxrt_eth->eth_info->phyAddr;
    phyConfig.autoNeg                           = true;
    phyConfig.duplex                            = kPHY_FullDuplex;
    phyConfig.speed                             = kPHY_Speed100M;

    do
    {
        status = PHY_Init(&imxrt_eth->phyHandle, &phyConfig);
        if (status == kStatus_Success)
        {
            LOG_D(DRV_EXT_TAG, "Wait for PHY link up...");
            /* Wait for auto-negotiation success and link up */
            count = PHY_AUTONEGO_TIMEOUT_COUNT;
            
            do
            {
                PHY_GetAutoNegotiationStatus(&imxrt_eth->phyHandle, &autonego);
                PHY_GetLinkStatus(&imxrt_eth->phyHandle, &link);
                if (autonego && link)
                {
                    break;
                }
            } while (--count);
            if (!autonego)
            {
                LOG_E(DRV_EXT_TAG, "PHY Auto-negotiation failed. Please check the cable connection and link partner setting!");
            }
        }
        count_init--;
    }while ((!(link && autonego)) && count_init);
    
    if (count_init == 0)
    {
        LOG_E(DRV_EXT_TAG, "PHY_Init failed!");
        return OS_ERROR;
    }
        
    if (imxrt_eth->sysClock != 0)
    {
        ENET_Deinit(imxrt_eth->eth_info->base);
    }
    else
    {
        imxrt_eth->sysClock = CLOCK_GetFreq(kCLOCK_IpgClk);
    }

    ENET_GetDefaultConfig(&config);
    
    PHY_GetLinkSpeedDuplex(&imxrt_eth->phyHandle, (phy_speed_t *)&imxrt_eth->speed, (phy_duplex_t *)&imxrt_eth->duplex);
    
    config.interrupt                    = kENET_TxFrameInterrupt | kENET_RxFrameInterrupt;
    config.miiSpeed                     = imxrt_eth->speed;
    config.miiDuplex                    = imxrt_eth->duplex;
    config.callback                     = _enet_callback;
    config.userData                     = imxrt_eth;
    
    ENET_Init(imxrt_eth->eth_info->base, &imxrt_eth->enet_handle, &config, imxrt_eth->eth_info->buffConfig, &imxrt_eth->dev_addr[0], imxrt_eth->sysClock);
    
    ENET_ActiveRead(imxrt_eth->eth_info->base);

    return OS_EOK;
}

static os_size_t os_imxrt_eth_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    return 0;
}

static os_size_t os_imxrt_eth_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    return 0;
}

static os_err_t os_imxrt_eth_control(os_device_t *dev, int cmd, void *args)
{
    struct os_imxrt_eth *imxrt_eth = (struct os_imxrt_eth *)dev;

    switch (cmd)
    {
    case NIOCTL_GADDR:
        if (args) 
        {
            memcpy(args, imxrt_eth->dev_addr, 6);
        }
        else 
        {
            return OS_ERROR;
        }
    break;
    default :
        break;
    }

    return OS_EOK;
}

os_err_t os_imxrt_eth_tx(os_device_t *dev, struct pbuf *p)
{
    status_t status;
    
    struct pbuf *q;
    os_uint8_t *frame = OS_NULL;

    struct os_imxrt_eth *imxrt_eth = (struct os_imxrt_eth *)dev;
    
    OS_ASSERT(p != NULL);

    if (p->tot_len > ENET_FRAME_MAX_FRAMELEN)
    {
        LOG_E(DRV_EXT_TAG, "pbuf data is too big! max length %d", ENET_FRAME_MAX_FRAMELEN, 0);
        return OS_ERROR;
    }
    
    frame = os_calloc(1, p->tot_len);
    if (frame == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_calloc for buff failed! ");
        return OS_ERROR;
    }
    
    pbuf_copy_partial(p, frame, p->tot_len, 0);

    status = ENET_SendFrame(imxrt_eth->eth_info->base, &imxrt_eth->enet_handle, frame, p->tot_len, 0, false, NULL);
    if (status == kStatus_ENET_TxFrameBusy)
    {
        os_sem_wait(&imxrt_eth->tx_sem, OS_WAIT_FOREVER);
        status = ENET_SendFrame(imxrt_eth->eth_info->base, &imxrt_eth->enet_handle, imxrt_eth->eth_info->buffConfig->txBufferAlign, p->tot_len, 0, false, NULL);
    }

    os_free(frame);
    
    if(status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "ENET_SendFrame failed!");
        return OS_ERROR;
    }

    return OS_EOK;
}

struct pbuf *os_imxrt_eth_rx(os_device_t *dev)
{
    os_uint8_t  i = 0;
    os_uint8_t  rx_buffer_num = 0;
    os_uint32_t length = 0;
    status_t    status;
    os_uint32_t ts;
    enet_data_error_stats_t error;
    
    struct pbuf *p = OS_NULL;

    struct os_imxrt_eth *imxrt_eth = (struct os_imxrt_eth *)dev;

    status = ENET_GetRxFrameSize(&imxrt_eth->enet_handle, &length, 0);
    if ((length == 0) && (status != kStatus_Success))
    {
        LOG_D(DRV_EXT_TAG, "no frame!");
        goto err;
    }
    
    p = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
    if (p == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "pbuf_alloc failed!");
        goto err;
    }
    
    status = ENET_ReadFrame(imxrt_eth->eth_info->base, &imxrt_eth->enet_handle, p->payload, length, 0, &ts);
    if (status != kStatus_Success)
    {
        ENET_GetRxErrBeforeReadFrame(&imxrt_eth->enet_handle, &error, 0);
        ENET_ReadFrame(imxrt_eth->eth_info->base, &imxrt_eth->enet_handle, OS_NULL, 0, 0, OS_NULL);
        
        LOG_E(DRV_EXT_TAG, "A frame read failed!");
        pbuf_free(p);
        p = OS_NULL;
    }
err:
    ENET_EnableInterrupts(imxrt_eth->eth_info->base, kENET_RxFrameInterrupt);
    
    return p;
}

static void phy_linkchange(void *parameter)
{
    status_t status;
    phy_speed_t speed;
    phy_duplex_t duplex;
    bool link     = false;
    bool new_link = false;

    struct os_imxrt_eth *imxrt_eth = (struct os_imxrt_eth *)parameter;

    status = PHY_GetLinkStatus(&imxrt_eth->phyHandle, &new_link);
    if ((status == kStatus_Success) && (link != new_link))
    {
        link = new_link;

        if (link)
        {
            PHY_GetLinkSpeedDuplex(&imxrt_eth->phyHandle, &speed, &duplex);

            if ((imxrt_eth->speed != (enet_mii_speed_t)speed) || (imxrt_eth->duplex != (enet_mii_duplex_t)duplex))
            {
                imxrt_eth->speed    = (enet_mii_speed_t)speed;
                imxrt_eth->duplex   = (enet_mii_duplex_t)duplex;

                imxrt_eth_init(imxrt_eth);
            }

            eth_device_linkchange(&imxrt_eth->dev, OS_TRUE);
        }
        else
        {
            eth_device_linkchange(&imxrt_eth->dev, OS_FALSE);
        }
    }
}

const static struct os_device_ops eth_ops = {
    .init    = OS_NULL,
    .deinit  = OS_NULL,
    .read    = os_imxrt_eth_read,
    .write   = os_imxrt_eth_write,
    .control = os_imxrt_eth_control,
};

static int imxrt_eth_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t state;
    os_uint32_t   unique_id[2];
    
    os_task_t *monitor_task = OS_NULL;

    struct os_imxrt_eth *imxrt_eth = OS_NULL;
    
    imxrt_eth = os_calloc(1, sizeof(struct os_imxrt_eth));
    if (imxrt_eth == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "imxrt_eth memory call failed!");
        return OS_ENOMEM;
    }

    imxrt_eth->eth_info = (struct nxp_eth_info *)dev->info;

    OCOTP_Init(OCOTP, 500000000U);
    unique_id[0]  = OCOTP_ReadFuseShadowRegister(OCOTP, 1);
    unique_id[1]  = OCOTP_ReadFuseShadowRegister(OCOTP, 2);
    OCOTP_Deinit(OCOTP);
    
    /* NXP (Freescale) MAC OUI */
    imxrt_eth->dev_addr[0] = 0x00;
    imxrt_eth->dev_addr[1] = 0x04;
    imxrt_eth->dev_addr[2] = 0x9F;
    /* generate MAC addr from unique ID (only for test). */
    imxrt_eth->dev_addr[3] |= (os_uint8_t)(unique_id[0] & 0xFF);
    imxrt_eth->dev_addr[4] |= (os_uint8_t)((unique_id[0] >> 8) & 0xFF);
    imxrt_eth->dev_addr[5] |= (os_uint8_t)((unique_id[0] >> 16) & 0xFF);
    imxrt_eth->dev_addr[5] += imxrt_eth->eth_info->phyAddr;
    
    if (imxrt_eth_init(imxrt_eth) != OS_EOK)
    {
        return OS_EOK;
    }
        
    imxrt_eth->dev.parent.ops           = &eth_ops;
    imxrt_eth->dev.eth_rx               = os_imxrt_eth_rx;
    imxrt_eth->dev.eth_tx               = os_imxrt_eth_tx;

    /* init tx semaphore */
    os_sem_init(&imxrt_eth->tx_sem, "tx_sem", 0, 1);

    state = eth_device_init(&imxrt_eth->dev, "e0");
    if (OS_EOK != state)
    {
        os_free(imxrt_eth);
        LOG_E(DRV_EXT_TAG, "eth_device_init faild: %d", state);
        return OS_ERROR;
    }

    imxrt_eth->poll_link_timer = os_timer_create("phylnk", (void (*)(void *))phy_linkchange, imxrt_eth, OS_TICK_PER_SECOND, OS_TIMER_FLAG_PERIODIC);
    if (!imxrt_eth->poll_link_timer || os_timer_start(imxrt_eth->poll_link_timer) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG,"Start link change detection timer failed");
    }

    return state;
}

OS_DRIVER_INFO imxrt_eth_driver = {
    .name   = "ETH_Type",
    .probe  = imxrt_eth_probe,
};

OS_DRIVER_DEFINE(imxrt_eth_driver, DEVICE, OS_INIT_SUBLEVEL_LOW);


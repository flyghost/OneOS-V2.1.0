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
 * @file        drv_can.c
 *
 * @brief       This file implements can driver for os_imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <board.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>

#include "fsl_common.h"
#include "fsl_flexcan.h"
#include "drv_can.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.can"
#include <drv_log.h>

struct os_imxrt_can
{
    struct os_can_device        can;

    struct nxp_can_info        *info;
    struct os_can_msg          *rx_msg;
    flexcan_handle_t            canHandle;
    os_uint32_t                 sourceClock_Hz;
    
    flexcan_frame_t            *frame;
    os_uint32_t                 rxmb_num;
    os_uint32_t                 txmb_num;
    
    os_list_node_t              list;
};

static os_list_node_t os_imxrt_can_list = OS_LIST_INIT(os_imxrt_can_list);

static int _can_recvmsg(struct os_imxrt_can *imxrt_can, uint32_t result)
{
    flexcan_mb_transfer_t rxXfer;

    rxXfer.frame = &imxrt_can->frame[result];
    rxXfer.mbIdx = result;

    if (rxXfer.frame->type == kFLEXCAN_FrameTypeData)
    {
        imxrt_can->rx_msg->rtr  = OS_CAN_DTR;
    }
    else
    {
        imxrt_can->rx_msg->rtr  = OS_CAN_RTR;
    }

    if (rxXfer.frame->format == kFLEXCAN_FrameFormatStandard)
    {
        imxrt_can->rx_msg->ide  = OS_CAN_STDID;
        imxrt_can->rx_msg->id   = rxXfer.frame->id >> CAN_ID_STD_SHIFT;
    }
    else
    {
        imxrt_can->rx_msg->ide  = OS_CAN_EXTID;
        imxrt_can->rx_msg->id   = rxXfer.frame->id >> CAN_ID_EXT_SHIFT;
    }
    
    imxrt_can->rx_msg->hdr      = result;
    imxrt_can->rx_msg->len      = rxXfer.frame->length;
    imxrt_can->rx_msg->data[0]  = rxXfer.frame->dataByte0;
    imxrt_can->rx_msg->data[1]  = rxXfer.frame->dataByte1;
    imxrt_can->rx_msg->data[2]  = rxXfer.frame->dataByte2;
    imxrt_can->rx_msg->data[3]  = rxXfer.frame->dataByte3;
    imxrt_can->rx_msg->data[4]  = rxXfer.frame->dataByte4;
    imxrt_can->rx_msg->data[5]  = rxXfer.frame->dataByte5;
    imxrt_can->rx_msg->data[6]  = rxXfer.frame->dataByte6;
    imxrt_can->rx_msg->data[7]  = rxXfer.frame->dataByte7;
    
    return OS_EOK;
}

static void _can_callback(CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData)
{
    struct os_imxrt_can    *imxrt_can;
    flexcan_mb_transfer_t   rxXfer;

    OS_ASSERT(base);
    OS_ASSERT(userData);
   
    imxrt_can = (struct os_imxrt_can *)userData;

    switch (status)
    {
    case kStatus_FLEXCAN_RxIdle:
        if (imxrt_can->rx_msg == OS_NULL)
        {
            return;
        }
        
        _can_recvmsg(imxrt_can, result);
        imxrt_can->rx_msg = OS_NULL;

        os_hw_can_isr_rxdone(&imxrt_can->can);

        rxXfer.frame = &imxrt_can->frame[result];
        rxXfer.mbIdx = result;
        
        FLEXCAN_TransferReceiveNonBlocking(imxrt_can->info->can_base, &imxrt_can->canHandle, &rxXfer);
        
        break;

    case kStatus_FLEXCAN_TxIdle:
        os_hw_can_isr_txdone(&imxrt_can->can, OS_CAN_EVENT_TX_DONE);
    break;
    case kStatus_FLEXCAN_WakeUp:
    case kStatus_FLEXCAN_ErrorStatus:
    break;
    case kStatus_FLEXCAN_TxSwitchToRx:
        break;

    default:
        break;
    }
}

static os_err_t os_imxrt_can_init(struct os_imxrt_can *imxrt_can)
{
    uint32_t i = 0;
    flexcan_mb_transfer_t rxXfer;
    flexcan_rx_mb_config_t mbConfig;
    flexcan_frame_t *frame = OS_NULL;
    
    if (imxrt_can->info->config->maxMbNum < 8)
    {
        LOG_E(DRV_EXT_TAG, "%s mailbox must more than 8!", imxrt_can->can.parent.name, imxrt_can->info->config->maxMbNum);
        return OS_ERROR;
    }
    
    if (imxrt_can->info->config->enableIndividMask != OS_TRUE)
    {
        LOG_E(DRV_EXT_TAG, "%s must set as enableIndividMask!", imxrt_can->can.parent.name);
        return OS_ERROR;
    }
    
    imxrt_can->rxmb_num = imxrt_can->info->config->maxMbNum / 2;
    imxrt_can->txmb_num = imxrt_can->info->config->maxMbNum / 2;
    
    if (imxrt_can->frame == OS_NULL)
    {
        imxrt_can->frame = os_dma_malloc_align(imxrt_can->rxmb_num*sizeof(flexcan_frame_t), 4);
        OS_ASSERT_EX(imxrt_can->frame, "cannot call memory for can receive!");
    }
    
    FLEXCAN_TransferCreateHandle(imxrt_can->info->can_base, &imxrt_can->canHandle, _can_callback, imxrt_can);
    
    mbConfig.format = kFLEXCAN_FrameFormatStandard;
    mbConfig.type   = kFLEXCAN_FrameTypeData;
    mbConfig.id     = FLEXCAN_ID_STD(0);
    
    for (i = 0; i < imxrt_can->rxmb_num; i++)
    {
        FLEXCAN_SetRxIndividualMask(imxrt_can->info->can_base, i, FLEXCAN_RX_MB_STD_MASK(0, 0, 0));
        
        FLEXCAN_SetRxMbConfig(imxrt_can->info->can_base, i, &mbConfig, true);
        
        rxXfer.frame = &imxrt_can->frame[i];
        rxXfer.mbIdx = i;

        FLEXCAN_TransferReceiveNonBlocking(imxrt_can->info->can_base, &imxrt_can->canHandle, &rxXfer);
    }

    for (i = 0; i < imxrt_can->txmb_num; i++)
    {
        FLEXCAN_SetTxMbConfig(imxrt_can->info->can_base, i + imxrt_can->rxmb_num, true);
    }

    FLEXCAN_DisableInterrupts(imxrt_can->info->can_base, (uint32_t)kFLEXCAN_BusOffInterruptEnable | (uint32_t)kFLEXCAN_ErrorInterruptEnable |
              (uint32_t)kFLEXCAN_RxWarningInterruptEnable | (uint32_t)kFLEXCAN_TxWarningInterruptEnable | (uint32_t)kFLEXCAN_WakeUpInterruptEnable);
    
    return OS_EOK;
}

static os_err_t os_imxrt_can_config(struct os_can_device *can, struct can_configure *cfg)
{
    flexcan_config_t can_config;
    struct os_imxrt_can *imxrt_can;

    OS_ASSERT(can);
    
    imxrt_can = (struct os_imxrt_can *)can;

    memcpy(&can_config, imxrt_can->info->config, sizeof(flexcan_config_t));

    switch(cfg->mode)
    {
    case OS_CAN_MODE_NORMAL:
        can_config.enableListenOnlyMode = false;
        can_config.enableLoopBack = false;
    break;
    case OS_CAN_MODE_LISEN:
        can_config.enableListenOnlyMode = true;
        can_config.enableLoopBack = false;
    break;
    case OS_CAN_MODE_LOOPBACK:
        can_config.enableLoopBack = true;
        can_config.enableListenOnlyMode = false;
    break;
    case OS_CAN_MODE_LOOPBACKANLISEN:
        can_config.enableLoopBack = true;
        can_config.enableListenOnlyMode = true;
    break;
    default:
        LOG_D(DRV_EXT_TAG, "not support can current mode!");
    break;
    }
    
    can_config.baudRate = cfg->baud_rate;

    FLEXCAN_CalculateImprovedTimingValues(imxrt_can->info->can_base, can_config.baudRate, imxrt_can->sourceClock_Hz, &can_config.timingConfig);

    FLEXCAN_Init(imxrt_can->info->can_base, &can_config, imxrt_can->sourceClock_Hz);
    
    os_imxrt_can_init(imxrt_can);
    
    return OS_EOK;
}

static os_err_t os_imxrt_can_control(struct os_can_device *can, int cmd, void *arg)
{
    os_err_t err;
    flexcan_rx_mb_config_t mbConfig;
    os_uint32_t     argval, mask;
    os_uint32_t errtype;
    struct os_imxrt_can         *imxrt_can;
    struct os_can_filter_config *filter_cfg;
    struct os_can_filter_item   *item;
    os_uint8_t i, count, index;

    OS_ASSERT(can);
    
    imxrt_can = (struct os_imxrt_can *)can;

    switch (cmd)
    {
    case OS_CAN_CMD_SET_FILTER:
        filter_cfg = (struct os_can_filter_config *)arg;
        if (filter_cfg == OS_NULL)
        {
            for (i = 0; i < imxrt_can->rxmb_num; i++)
            {
                FLEXCAN_SetRxIndividualMask(imxrt_can->info->can_base, i, FLEXCAN_RX_MB_STD_MASK(0, 0, 0));
            }
            return OS_EOK;
        }
        
        item = filter_cfg->items;

        if (filter_cfg->count > imxrt_can->rxmb_num)
        {
            LOG_E(DRV_EXT_TAG, "filter count more than can maxnum!");
            return OS_EINVAL;
        }
        
        for (i = 0; i < filter_cfg->count; i++)
        {
            if (item->hdr >= imxrt_can->rxmb_num)
            {
                continue;
            }
            
            if (item[i].ide)
            {
                mbConfig.format = kFLEXCAN_FrameFormatExtend;
                mbConfig.id = FLEXCAN_ID_EXT(item[i].id);
                mask = FLEXCAN_RX_MB_EXT_MASK(item[i].mask, item[i].rtr, item[i].ide);
            }
            else
            {
                mbConfig.format = kFLEXCAN_FrameFormatStandard;
                mbConfig.id = FLEXCAN_ID_STD(item[i].id);
                mask = FLEXCAN_RX_MB_STD_MASK(item[i].mask, item[i].rtr, item[i].ide);
            }

            if (item[i].rtr)
            {
                mbConfig.type = kFLEXCAN_FrameTypeRemote;
            }
            else
            {
                mbConfig.type = kFLEXCAN_FrameTypeData;
            }

            FLEXCAN_SetRxMbConfig(imxrt_can->info->can_base, item->hdr, OS_NULL, false);
            
            FLEXCAN_SetRxIndividualMask(imxrt_can->info->can_base, item->hdr, mask);
            
            FLEXCAN_SetRxMbConfig(imxrt_can->info->can_base, item->hdr, &mbConfig, true);
        }
        
    break;
    case OS_CAN_CMD_SET_MODE:
        argval = (os_uint32_t)arg;
        if (argval != OS_CAN_MODE_NORMAL &&
            argval != OS_CAN_MODE_LISEN &&
            argval != OS_CAN_MODE_LOOPBACK &&
            argval != OS_CAN_MODE_LOOPBACKANLISEN)
        {
            return OS_ERROR;
        }
        if (argval != can->config.mode)
        {
            can->config.mode = argval;
            return os_imxrt_can_config(can, &can->config);
        }
    break;
    case OS_CAN_CMD_SET_BAUD:
        argval = (os_uint32_t) arg;
        if (argval != can->config.baud_rate)
        {
            can->config.baud_rate = argval;
            return os_imxrt_can_config(can, &can->config);
        }
    break;
    case OS_CAN_CMD_SET_PRIV:
        return OS_EINVAL;
    break;
    case OS_CAN_CMD_GET_STATUS:
        
    break;
    }

    return OS_EOK;
}

static int os_imxrt_can_start_send(struct os_can_device *can, const struct os_can_msg *pmsg)
{
    os_uint8_t i = 0;
    struct os_imxrt_can  *imxrt_can;
    flexcan_mb_transfer_t txXfer;
    
    OS_ASSERT(can);
    
    imxrt_can = (struct os_imxrt_can *)can;
    
    for (i = 0; i < imxrt_can->txmb_num; i++)
    {
        if ((uint8_t)0 == imxrt_can->canHandle.mbState[i+imxrt_can->rxmb_num])
        {
            break;
        }
    }

    txXfer.mbIdx = i+imxrt_can->rxmb_num;
    
    if (OS_CAN_STDID == pmsg->ide)
    {
        txXfer.frame->id = FLEXCAN_ID_STD(pmsg->id);
        txXfer.frame->format = kFLEXCAN_FrameFormatStandard;
    }
    else
    {
        txXfer.frame->id = FLEXCAN_ID_EXT(pmsg->id);
        txXfer.frame->format = kFLEXCAN_FrameFormatExtend;
    }

    if (OS_CAN_DTR == pmsg->rtr)
    {
        txXfer.frame->type = kFLEXCAN_FrameTypeData;
    }
    else
    {
        txXfer.frame->type = kFLEXCAN_FrameTypeRemote;
    }

    txXfer.frame->length    = pmsg->len;
    txXfer.frame->dataByte0 = pmsg->data[0];
    txXfer.frame->dataByte1 = pmsg->data[1];
    txXfer.frame->dataByte2 = pmsg->data[2];
    txXfer.frame->dataByte3 = pmsg->data[3];
    txXfer.frame->dataByte4 = pmsg->data[4];
    txXfer.frame->dataByte5 = pmsg->data[5];
    txXfer.frame->dataByte6 = pmsg->data[6];
    txXfer.frame->dataByte7 = pmsg->data[7];
    
    FLEXCAN_TransferSendNonBlocking(imxrt_can->info->can_base, &imxrt_can->canHandle, &txXfer);

    return 0;
}

static int os_imxrt_can_stop_send(struct os_can_device *can)
{
    struct os_imxrt_can  *imxrt_can;
    flexcan_frame_t     *frame;
    flexcan_mb_transfer_t rxXfer;
    
    OS_ASSERT(can);
    
    imxrt_can = (struct os_imxrt_can *)can;
    
    FLEXCAN_DisableInterrupts(imxrt_can->info->can_base, (uint32_t)kFLEXCAN_BusOffInterruptEnable | (uint32_t)kFLEXCAN_ErrorInterruptEnable |
              (uint32_t)kFLEXCAN_RxWarningInterruptEnable | (uint32_t)kFLEXCAN_TxWarningInterruptEnable | (uint32_t)kFLEXCAN_WakeUpInterruptEnable);

    return 0;
}

static int os_imxrt_can_start_recv(struct os_can_device *can, struct os_can_msg *msg)
{
    struct os_imxrt_can  *imxrt_can;
    
    OS_ASSERT(can);
    
    imxrt_can = (struct os_imxrt_can *)can;

    imxrt_can->rx_msg = msg;

    FLEXCAN_EnableInterrupts(imxrt_can->info->can_base, (uint32_t)kFLEXCAN_BusOffInterruptEnable | (uint32_t)kFLEXCAN_ErrorInterruptEnable |
              (uint32_t)kFLEXCAN_RxWarningInterruptEnable | (uint32_t)kFLEXCAN_TxWarningInterruptEnable | (uint32_t)kFLEXCAN_WakeUpInterruptEnable);

    return 0;
}

static int os_imxrt_can_stop_recv(struct os_can_device *can)
{
    struct os_imxrt_can  *imxrt_can;
    flexcan_frame_t     *frame;
    flexcan_mb_transfer_t rxXfer;
    
    OS_ASSERT(can);
    
    imxrt_can = (struct os_imxrt_can *)can;
    
    FLEXCAN_DisableInterrupts(imxrt_can->info->can_base, (uint32_t)kFLEXCAN_BusOffInterruptEnable | (uint32_t)kFLEXCAN_ErrorInterruptEnable |
              (uint32_t)kFLEXCAN_RxWarningInterruptEnable | (uint32_t)kFLEXCAN_TxWarningInterruptEnable | (uint32_t)kFLEXCAN_WakeUpInterruptEnable);

    os_free(imxrt_can->frame);
    
    return 0;
}

static int os_imxrt_can_recv_state(struct os_can_device *can)
{
    return 0;
}

static const struct os_can_ops os_imxrt_can_ops = {
    .configure  = os_imxrt_can_config,
    .control    = os_imxrt_can_control,
    .start_send = os_imxrt_can_start_send,
    .stop_send  = os_imxrt_can_stop_send,
    .start_recv = os_imxrt_can_start_recv,
    .stop_recv  = os_imxrt_can_stop_recv,
    .recv_state = os_imxrt_can_recv_state,    
};

os_err_t imxrt_can_param_cfg(struct os_imxrt_can *imxrt_can)
{
    switch((os_base_t)imxrt_can->info->can_base)
    {
    case (os_base_t)CAN1:
#ifdef CAN1_CLOCK_SOURCE
        imxrt_can->sourceClock_Hz = CAN1_CLOCK_SOURCE;
#endif
    break;
    case (os_base_t)CAN2:
#ifdef CAN2_CLOCK_SOURCE
        imxrt_can->sourceClock_Hz = CAN2_CLOCK_SOURCE;
#endif
    break;
    default:
    LOG_E(DRV_EXT_TAG, "cannot find can type!");
    return OS_ERROR;
    }
    
    if (imxrt_can->info->config->enableLoopBack)
    {
        if (imxrt_can->info->config->enableListenOnlyMode)
        {
            imxrt_can->can.config.mode = OS_CAN_MODE_LOOPBACKANLISEN;
        }
        else
        {
            imxrt_can->can.config.mode = OS_CAN_MODE_LOOPBACK;
        }
        
    }
    else if (imxrt_can->info->config->enableListenOnlyMode)
    {
        imxrt_can->can.config.mode = OS_CAN_MODE_LISEN;
    }
    else
    {
        imxrt_can->can.config.mode = OS_CAN_MODE_NORMAL;
    }
    
    imxrt_can->can.config.baud_rate = imxrt_can->info->config->baudRate;
    
    return OS_EOK;
}

static int os_imxrt_can_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;

    struct can_configure config = CANDEFAULTCONFIG;
    
    struct os_imxrt_can *imxrt_can = os_calloc(1, sizeof(struct os_imxrt_can));

    OS_ASSERT(imxrt_can);

    imxrt_can->info = (struct nxp_can_info *)dev->info;
    imxrt_can->can.config = config;

    imxrt_can_param_cfg(imxrt_can);
    
    level = os_irq_lock();
    os_list_add_tail(&os_imxrt_can_list, &imxrt_can->list);
    os_irq_unlock(level);
    
    os_hw_can_register(&imxrt_can->can, dev->name, &os_imxrt_can_ops, imxrt_can);

    return OS_EOK;
}

OS_DRIVER_INFO os_imxrt_can_driver = {
    .name   = "CAN_Type",
    .probe  = os_imxrt_can_probe,
};

OS_DRIVER_DEFINE(os_imxrt_can_driver, PREV, OS_INIT_SUBLEVEL_LOW);




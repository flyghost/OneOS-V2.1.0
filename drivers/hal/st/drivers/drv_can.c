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
 * @file        st_can.c
 *
 * @brief       This file implements can driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.can"
#include <drv_log.h>

#define BS1SHIFT    16
#define BS2SHIFT    20
#define RRESCLSHIFT 0
#define SJWSHIFT    24
#define BS1MASK     ((0x0F) << BS1SHIFT)
#define BS2MASK     ((0x07) << BS2SHIFT)
#define RRESCLMASK  (0x3FF << RRESCLSHIFT)
#define SJWMASK     (0x3 << SJWSHIFT)

typedef struct stm32_can_baud {
    os_uint32_t baud;
    
    os_uint16_t prescale;
    os_uint8_t  sjw;
    os_uint8_t  bs1;
    os_uint8_t  bs2;
}stm32_can_baud_t;

struct stm32_can
{
    struct os_can_device can;

    CAN_HandleTypeDef *hcan;
    CAN_FilterTypeDef  filter;
    stm32_can_baud_t   baud;

    struct os_can_msg *rx_msg;

    os_list_node_t list;
};

static os_list_node_t gs_stm32_can_list = OS_LIST_INIT(gs_stm32_can_list);

static struct stm32_can *find_stm32_can(CAN_HandleTypeDef *hcan)
{
    struct stm32_can *st_can;

    os_list_for_each_entry(st_can, &gs_stm32_can_list, struct stm32_can, list)
    {
        if (st_can->hcan == hcan)
        {
            return st_can;
        }
    }

    return OS_NULL;
}

static const os_uint32_t sjw_table[] = 
{
    CAN_SJW_1TQ,
    CAN_SJW_2TQ,
    CAN_SJW_3TQ,
    CAN_SJW_4TQ,
};

static os_uint32_t stm32_can_sjw(os_uint8_t sjw)
{
    return sjw_table[sjw - 1];
}

static const os_uint32_t bs1_table[] = 
{
    CAN_BS1_1TQ ,
    CAN_BS1_2TQ ,
    CAN_BS1_3TQ ,
    CAN_BS1_4TQ ,
    CAN_BS1_5TQ ,
    CAN_BS1_6TQ ,
    CAN_BS1_7TQ ,
    CAN_BS1_8TQ ,
    CAN_BS1_9TQ ,
    CAN_BS1_10TQ,
    CAN_BS1_11TQ,
    CAN_BS1_12TQ,
    CAN_BS1_13TQ,
    CAN_BS1_14TQ,
    CAN_BS1_15TQ,
    CAN_BS1_16TQ,
};

static os_uint32_t stm32_can_bs1(os_uint8_t bs1)
{
    return bs1_table[bs1 - 1];
}

const os_uint32_t bs2_table[] = 
{
    CAN_BS2_1TQ,
    CAN_BS2_2TQ,
    CAN_BS2_3TQ,
    CAN_BS2_4TQ,
    CAN_BS2_5TQ,
    CAN_BS2_6TQ,
    CAN_BS2_7TQ,
    CAN_BS2_8TQ,
};

static os_uint32_t stm32_can_bs2(os_uint8_t bs2)
{
    return bs2_table[bs2 - 1];
}

static int stm32_can_calc_baud(stm32_can_baud_t *baud)
{
    os_uint32_t pclk1;
    os_uint16_t prescale;
    os_uint8_t  sjw;
    os_uint8_t  bs1;
    os_uint8_t  bs2;

    os_uint64_t tmp;
    os_uint32_t min_delta = (os_uint32_t)-1;

    os_uint16_t best_prescale = (os_uint16_t)-1;
    os_uint8_t  best_sjw = (os_uint8_t)-1;
    os_uint8_t  best_bs1 = (os_uint8_t)-1;
    os_uint8_t  best_bs2 = (os_uint8_t)-1;

    pclk1 = HAL_RCC_GetPCLK1Freq();

    for (sjw = 1; sjw < 3; sjw++)
    {
        for (bs2 = 2; bs2 < 6; bs2++)
        {
            for (bs1 = 4; bs1 < 12; bs1++)
            {
                for (prescale = 1; prescale < 0x3FF; prescale++)
                {
                    tmp = (os_uint64_t)(sjw + bs1 + bs2) * prescale * baud->baud;
                    
                    if (tmp == pclk1)
                    {
                        best_prescale = prescale;
                        best_sjw = sjw;
                        best_bs1 = bs1;
                        best_bs2 = bs2;
                        min_delta = 0;
                        goto end;
                    }

                    if (tmp - pclk1 < min_delta || pclk1 - tmp < min_delta)
                    {
                        min_delta = min(tmp - pclk1, pclk1 - tmp);
                        best_prescale = prescale;
                        best_sjw = sjw;
                        best_bs1 = bs1;
                        best_bs2 = bs2;
                    }
                }
            }
        }
    }
    
end:
    if (best_prescale == (os_uint16_t)-1)
        return -1;
    
    baud->prescale = best_prescale;
    baud->sjw = best_sjw;
    baud->bs1 = best_bs1;
    baud->bs2 = best_bs2;

    os_kprintf("can baud:%d, clk:%d, rate:0.%03d\r\n", baud->baud, pclk1, (int)((os_uint64_t)min_delta * 1000 / pclk1));
    
    return 0;
}

static os_err_t stm32_can_config(struct os_can_device *can, struct can_configure *cfg)
{
    struct stm32_can  *st_can;
    CAN_HandleTypeDef *hcan;

    OS_ASSERT(can);
    OS_ASSERT(cfg);
    
    st_can = (struct stm32_can *)can;

    hcan   = st_can->hcan;

    hcan->Init.TimeTriggeredMode    = DISABLE;
    hcan->Init.AutoBusOff           = ENABLE;
    hcan->Init.AutoWakeUp           = DISABLE;
    hcan->Init.AutoRetransmission   = DISABLE;
    hcan->Init.ReceiveFifoLocked    = DISABLE;
    hcan->Init.TransmitFifoPriority = ENABLE;

    switch (cfg->mode)
    {
    case OS_CAN_MODE_NORMAL:
        hcan->Init.Mode = CAN_MODE_NORMAL;
        break;
    case OS_CAN_MODE_LISEN:
        hcan->Init.Mode = CAN_MODE_SILENT;
        break;
    case OS_CAN_MODE_LOOPBACK:
        hcan->Init.Mode = CAN_MODE_LOOPBACK;
        break;
    case OS_CAN_MODE_LOOPBACKANLISEN:
        hcan->Init.Mode = CAN_MODE_SILENT_LOOPBACK;
        break;
    }

    stm32_can_baud_t baud;
    baud.baud = cfg->baud_rate;

    if (stm32_can_calc_baud(&baud) != 0)
    {
        os_kprintf("%s calc baud failed %u.\r\n", device_name(&can->parent), baud.baud);
        return OS_ERROR;
    }

    st_can->baud = baud;

    hcan->Init.SyncJumpWidth = stm32_can_sjw(baud.sjw);
    hcan->Init.TimeSeg1      = stm32_can_bs1(baud.bs1);
    hcan->Init.TimeSeg2      = stm32_can_bs2(baud.bs2);
    hcan->Init.Prescaler     = baud.prescale;

    if (HAL_CAN_Init(hcan) != HAL_OK)
    {
        os_kprintf("%s init failed.\r\n", device_name(&can->parent));
        return OS_ERROR;
    }

    HAL_CAN_ConfigFilter(hcan, &st_can->filter);

    HAL_CAN_Start(hcan);

    return OS_EOK;
}

static os_err_t stm32_can_control(struct os_can_device *can, int cmd, void *arg)
{
    os_uint32_t                  argval;
    struct stm32_can *           st_can;
    struct os_can_filter_config *filter_cfg;

    OS_ASSERT(can != OS_NULL);
    st_can = (struct stm32_can *)can;

    switch (cmd)
    {
    case OS_CAN_CMD_SET_FILTER:
        if (OS_NULL == arg)
        {
            HAL_CAN_ConfigFilter(st_can->hcan, &st_can->filter);
        }
        else
        {
            filter_cfg = (struct os_can_filter_config *)arg;

            for (int i = 0; i < filter_cfg->count; i++)
            {
                st_can->filter.FilterBank   = filter_cfg->items[i].hdr;
                st_can->filter.FilterIdHigh = (filter_cfg->items[i].id >> 13) & 0xFFFF;
                st_can->filter.FilterIdLow  = ((filter_cfg->items[i].id << 3) | 
                                               (filter_cfg->items[i].ide << 2) | 
                                               (filter_cfg->items[i].rtr << 1)) & 0xFFFF;
                st_can->filter.FilterMaskIdHigh = (filter_cfg->items[i].mask >> 16) & 0xFFFF;
                st_can->filter.FilterMaskIdLow  = filter_cfg->items[i].mask & 0xFFFF;
                st_can->filter.FilterMode       = filter_cfg->items[i].mode;

                HAL_CAN_ConfigFilter(st_can->hcan, &st_can->filter);
            }
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
        if (argval != st_can->can.config.mode)
        {
            can->config.mode = argval;
            return stm32_can_config(can, &can->config);
        }
        break;
    case OS_CAN_CMD_SET_BAUD:
        argval = (os_uint32_t) arg;
        if (argval != can->config.baud_rate)
        {
            can->config.baud_rate = argval;
            return stm32_can_config(can, &can->config);
        }
        break;
    case OS_CAN_CMD_SET_PRIV:
        argval = (os_uint32_t)arg;
        if (argval != OS_CAN_MODE_PRIV && argval != OS_CAN_MODE_NOPRIV)
        {
            return OS_ERROR;
        }
        if (argval != can->config.privmode)
        {
            can->config.privmode = argval;
            return stm32_can_config(can, &can->config);
        }
        break;
    case OS_CAN_CMD_GET_STATUS:
    {
        os_uint32_t errtype;
        errtype                 = st_can->hcan->Instance->ESR;
        can->status.rcverrcnt   = errtype >> 24;
        can->status.snderrcnt   = (errtype >> 16) & 0xFF;
        can->status.lasterrtype = errtype & 0x70;
        can->status.errcode     = errtype & 0x07;

        memcpy(arg, &can->status, sizeof(can->status));
    }
    break;
    }

    return OS_EOK;
}

static int stm32_can_recvmsg(struct os_can_device *can, struct os_can_msg *pmsg, os_uint32_t fifo)
{
    struct stm32_can   *st_can;
    HAL_StatusTypeDef   status;
    CAN_HandleTypeDef  *hcan;
    CAN_RxHeaderTypeDef rxheader = {0};

    OS_ASSERT(can);
    st_can = (struct stm32_can *)can;

    hcan = st_can->hcan;

    /* get data */
    status = HAL_CAN_GetRxMessage(hcan, fifo, &rxheader, pmsg->data);
    if (HAL_OK != status)
        return OS_ERROR;
    /* get id */
    if (CAN_ID_STD == rxheader.IDE)
    {
        pmsg->ide = OS_CAN_STDID;
        pmsg->id  = rxheader.StdId;
    }
    else
    {
        pmsg->ide = OS_CAN_EXTID;
        pmsg->id  = rxheader.ExtId;
    }
    /* get type */
    if (CAN_RTR_DATA == rxheader.RTR)
    {
        pmsg->rtr = OS_CAN_DTR;
    }
    else
    {
        pmsg->rtr = OS_CAN_RTR;
    }
    /* get len */
    pmsg->len = rxheader.DLC;
    /* get hdr */
    if (hcan->Instance == CAN1)
    {
        pmsg->hdr = (rxheader.FilterMatchIndex + 1) >> 1;
    }
#ifdef CAN2
    else if (hcan->Instance == CAN2)
    {
        pmsg->hdr = (rxheader.FilterMatchIndex >> 1) + 14;
    }
#endif

    return OS_EOK;
}

static int stm32_can_start_send(struct os_can_device *can, const struct os_can_msg *pmsg)
{
    struct stm32_can  *st_can;
    CAN_HandleTypeDef *hcan;
    
    CAN_TxHeaderTypeDef  txheader = {0};

    OS_ASSERT(can);
    st_can = (struct stm32_can *)can;
    
    hcan  = st_can->hcan;

    if (OS_CAN_STDID == pmsg->ide)
    {
        txheader.IDE = CAN_ID_STD;
        txheader.StdId = pmsg->id;
    }
    else
    {
        txheader.IDE = CAN_ID_EXT;
        txheader.ExtId = pmsg->id;
    }

    if (OS_CAN_DTR == pmsg->rtr)
    {
        txheader.RTR = CAN_RTR_DATA;
    }
    else
    {
        txheader.RTR = CAN_RTR_REMOTE;
    }

    txheader.DLC = pmsg->len;

    uint32_t TxMailbox;
    HAL_StatusTypeDef ret = HAL_CAN_AddTxMessage(hcan, &txheader, (uint8_t *)pmsg->data, &TxMailbox);
    if (ret != HAL_OK)
    {
        os_kprintf("%s hal send failed.\r\n", device_name(&can->parent), ret);
        return -1;
    }

    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_TX_MAILBOX_EMPTY);
    
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_ERROR_WARNING);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_ERROR_PASSIVE);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_BUSOFF);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_LAST_ERROR_CODE);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_ERROR);

    return 0;
}

static int stm32_can_stop_send(struct os_can_device *can)
{
    struct stm32_can  *st_can;
    CAN_HandleTypeDef *hcan;

    OS_ASSERT(can);
    st_can = (struct stm32_can *)can;
    hcan   = st_can->hcan;
    OS_ASSERT(hcan);

    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_TX_MAILBOX_EMPTY);

    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_ERROR_WARNING);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_ERROR_PASSIVE);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_BUSOFF);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_LAST_ERROR_CODE);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_ERROR);
    
    return 0;
}

static int stm32_can_start_recv(struct os_can_device *can, struct os_can_msg *msg)
{
    struct stm32_can  *st_can;
    CAN_HandleTypeDef *hcan;

    OS_ASSERT(can);
    st_can = (struct stm32_can *)can;
    hcan   = st_can->hcan;
    OS_ASSERT(hcan);

    st_can->rx_msg = msg;

    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO0_FULL);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO0_OVERRUN);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO1_FULL);
    __HAL_CAN_ENABLE_IT(hcan, CAN_IT_RX_FIFO1_OVERRUN);
    
    return 0;
}

static int stm32_can_stop_recv(struct os_can_device *can)
{
    struct stm32_can  *st_can;
    CAN_HandleTypeDef *hcan;

    OS_ASSERT(can);
    st_can = (struct stm32_can *)can;
    hcan   = st_can->hcan;
    OS_ASSERT(hcan);

    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_FULL);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_OVERRUN);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_FULL);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_OVERRUN);
    
    return 0;
}

static int stm32_can_recv_state(struct os_can_device *can)
{
    return 0;
}

static const struct os_can_ops stm32_can_ops = {
    .configure  = stm32_can_config,
    .control    = stm32_can_control,
    .start_send = stm32_can_start_send,
    .stop_send  = stm32_can_stop_send,
    .start_recv = stm32_can_start_recv,
    .stop_recv  = stm32_can_stop_recv,
    .recv_state = stm32_can_recv_state,    
};

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_TX_MAILBOX_EMPTY);

    os_hw_can_isr_txdone((struct os_can_device *)find_stm32_can(hcan), OS_CAN_EVENT_TX_DONE);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_TX_MAILBOX_EMPTY);
    
    os_hw_can_isr_txdone((struct os_can_device *)find_stm32_can(hcan), OS_CAN_EVENT_TX_DONE);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_TX_MAILBOX_EMPTY);
    
    os_hw_can_isr_txdone((struct os_can_device *)find_stm32_can(hcan), OS_CAN_EVENT_TX_DONE);
}

void HAL_CAN_TxMailbox0AbortCallback(CAN_HandleTypeDef *hcan)
{
    os_hw_can_isr_txdone((struct os_can_device *)find_stm32_can(hcan), OS_CAN_EVENT_TX_FAIL);
}

void HAL_CAN_TxMailbox1AbortCallback(CAN_HandleTypeDef *hcan)
{
    os_hw_can_isr_txdone((struct os_can_device *)find_stm32_can(hcan), OS_CAN_EVENT_TX_FAIL);
}

void HAL_CAN_TxMailbox2AbortCallback(CAN_HandleTypeDef *hcan)
{
    os_hw_can_isr_txdone((struct os_can_device *)find_stm32_can(hcan), OS_CAN_EVENT_TX_FAIL);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    struct stm32_can *st_can = find_stm32_can(hcan);

    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_FULL);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_OVERRUN);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_FULL);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_OVERRUN);

    if (st_can->rx_msg == OS_NULL)
        return;

    stm32_can_recvmsg(&st_can->can, st_can->rx_msg, CAN_RX_FIFO0);
    st_can->rx_msg = OS_NULL;

    os_hw_can_isr_rxdone((struct os_can_device *)st_can);        
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    struct stm32_can *st_can = find_stm32_can(hcan);

    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_FULL);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO0_OVERRUN);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_FULL);
    __HAL_CAN_DISABLE_IT(hcan, CAN_IT_RX_FIFO1_OVERRUN);

    if (st_can->rx_msg == OS_NULL)
        return;

    stm32_can_recvmsg(&st_can->can, st_can->rx_msg, CAN_RX_FIFO1);
    st_can->rx_msg = OS_NULL;

    os_hw_can_isr_rxdone((struct os_can_device *)st_can);
}

void HAL_CAN_SleepCallback(CAN_HandleTypeDef *hcan)
{
        
}

void HAL_CAN_WakeUpFromRxMsgCallback(CAN_HandleTypeDef *hcan)
{
        
}

#define STM32_HAL_CAN_TX_ERRCODE (HAL_CAN_ERROR_TX_ALST0 | HAL_CAN_ERROR_TX_TERR0 | HAL_CAN_ERROR_TX_ALST1\
                                | HAL_CAN_ERROR_TX_TERR1 | HAL_CAN_ERROR_TX_ALST2 | HAL_CAN_ERROR_TX_TERR2)

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    __HAL_CAN_ENABLE_IT(hcan, 
                        CAN_IT_ERROR_WARNING         |
                        CAN_IT_ERROR_PASSIVE         |
                        CAN_IT_BUSOFF                |
                        CAN_IT_LAST_ERROR_CODE       |
                        CAN_IT_ERROR                 |
                        CAN_IT_RX_FIFO0_MSG_PENDING  |
                        CAN_IT_RX_FIFO0_OVERRUN      |
                        CAN_IT_RX_FIFO0_FULL         |
                        CAN_IT_RX_FIFO1_MSG_PENDING  |
                        CAN_IT_RX_FIFO1_OVERRUN      |
                        CAN_IT_RX_FIFO1_FULL         |
                        CAN_IT_TX_MAILBOX_EMPTY);

    if (hcan->ErrorCode & STM32_HAL_CAN_TX_ERRCODE)
    {
        hcan->ErrorCode &= ~STM32_HAL_CAN_TX_ERRCODE;
    
        os_hw_can_isr_txdone((struct os_can_device *)find_stm32_can(hcan), OS_CAN_EVENT_TX_FAIL);
    }
}

static int stm32_can_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct can_configure config = CANDEFAULTCONFIG;
    
    os_base_t   level;

    struct stm32_can *st_can = os_calloc(1, sizeof(struct stm32_can));

    OS_ASSERT(st_can);

    st_can->hcan = (CAN_HandleTypeDef *)dev->info;

    config.privmode             = OS_CAN_MODE_NOPRIV;
    config.ticks                = 50;
#ifdef OS_CAN_USING_HDR
    config.maxhdr = 14;
#ifdef CAN2
    config.maxhdr = 28;
#endif
#endif

    st_can->can.config = config;

    /* config default filter */
    st_can->filter.FilterIdHigh         = 0x0000;
    st_can->filter.FilterIdLow          = 0x0000;
    st_can->filter.FilterMaskIdHigh     = 0x0000;
    st_can->filter.FilterMaskIdLow      = 0x0000;
    st_can->filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    st_can->filter.FilterBank           = 0;
    st_can->filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    st_can->filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    st_can->filter.FilterActivation     = ENABLE;
    st_can->filter.SlaveStartFilterBank = 14;

    st_can->filter.FilterBank = 0;

    level = os_irq_lock();
    os_list_add_tail(&gs_stm32_can_list, &st_can->list);
    os_irq_unlock(level);
    
    os_hw_can_register(&st_can->can, dev->name, &stm32_can_ops, st_can);

    return OS_EOK;
}

OS_DRIVER_INFO stm32_can_driver = {
    .name   = "CAN_HandleTypeDef",
    .probe  = stm32_can_probe,
};

OS_DRIVER_DEFINE(stm32_can_driver,PREV,OS_INIT_SUBLEVEL_LOW);


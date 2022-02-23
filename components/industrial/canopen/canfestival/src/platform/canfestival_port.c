/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        canfestival_port.c
 *
 * @brief       This file implement canfestival interface function
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#ifdef OS_CAN_USING_HDR
#warning "nonsupport"
#endif

#if defined(OS_USING_CANFESTIVAL) && !defined(OS_CAN_USING_HDR)
#include "cf_canfestival.h"
#include <drv_cfg.h>
#include <hrtimer.h>
#include <os_event.h>
#include <os_memory.h>
#include <os_sem.h>
#include <timer/clocksource.h>

#include "dlog.h"
#define TAG "CF"

#ifndef CANFESTIVAL_MAX_NODE_NUM
#define CANFESTIVAL_MAX_NODE_NUM 10
#endif

//#define CANFESTIVAL_SEND_DUMP_ENABLE
//#define CANFESTIVAL_RECV_DUMP_ENABLE

//#define CAN_TEST_WRITE_NONBLOCK
//#define CAN_TEST_READ_NONBLOCK

static CO_Data *   sg_nodelist[CANFESTIVAL_MAX_NODE_NUM];
static UNS8        sg_nodenum;
static os_mutex_t *sg_cf_mutex;

void canopen_init(void)
{
    UNS8 index;
    sg_nodenum = 0;
    for (index = 0; index < sizeof(sg_nodelist) / sizeof(sg_nodelist[0]); index++)
    {
        sg_nodelist[index] = NULL;
    }
    sg_cf_mutex = os_mutex_create("cf_mutex", FALSE);
    OS_ASSERT(sg_cf_mutex);
}

CO_Data **get_canopen_nodelist(void)
{
    return sg_nodelist;
}

UNS8 get_canopen_nodenum(void)
{
    return sg_nodenum;
}

void CanOpen_EnterMutex(void)
{
    os_mutex_lock(sg_cf_mutex, OS_WAIT_FOREVER);
}

void CanOpen_LeaveMutex(void)
{
    os_mutex_unlock(sg_cf_mutex);
}

UNS8 canSend(CAN_PORT notused, Message *m)
{
    os_size_t         size;
    struct os_can_msg msg;
    os_device_t *     device;
    device = (os_device_t *)notused;
    OS_ASSERT(device);
    msg.hdr = 0;
    msg.id  = m->cob_id;
    msg.ide = OS_CAN_STDID;
    msg.rtr = m->rtr;
    msg.len = m->len;
    memcpy(msg.data, m->data, m->len);
#ifdef CANFESTIVAL_SEND_DUMP_ENABLE
    os_kprintf("%s send id:%04x ", device->name, msg.id);
    hex_dump(msg.data, 8);
#endif
#ifdef CAN_TEST_WRITE_NONBLOCK
    size = os_device_write_nonblock(device, 0, &msg, sizeof(msg));
#else
    size = os_device_write_block(device, 0, &msg, sizeof(msg));
#endif
    return size;
}

OS_USED static os_err_t can_rx_done(os_device_t *dev, struct os_device_cb_info *info)
{
//    LOG_I(TAG,"%s %s",dev->name,__FUNCTION__);
    return OS_EOK;
}

OS_USED static os_err_t can_tx_done(os_device_t *dev, struct os_device_cb_info *info)
{
//    LOG_I(TAG,"%s %s",dev->name,__FUNCTION__);
    return OS_EOK;
}

static void canopen_recv_entry(void *param)
{
    UNS8               index, node_num;
    os_size_t          rxsize;
    struct os_can_msg  msg;
    Message            co_msg;
    CO_Data **         d;
    os_device_t *const device = (os_device_t *)param;
    OS_ASSERT(device);
    
    while (1)
    {
#ifdef CAN_TEST_READ_NONBLOCK
        rxsize = os_device_read_nonblock(device, 0, &msg, sizeof(msg));
#else
        device->rx_size = sizeof(msg);
        rxsize = os_device_read_block(device, 0, &msg, sizeof(msg));
#endif
        if (rxsize <= 0)
        {
            os_task_msleep(10);
            continue;
        }

        co_msg.cob_id = msg.id;
        co_msg.len    = msg.len;
        co_msg.rtr    = msg.rtr;
        memcpy(co_msg.data, msg.data, msg.len);

#ifdef CANFESTIVAL_RECV_DUMP_ENABLE
        os_kprintf("%s recv id:%04x ", device->name, msg.id);
        hex_dump(msg.data, 8);
#endif
        d        = get_canopen_nodelist();
        node_num = get_canopen_nodenum();
        for (index = 0; index < node_num; index++)
        {
            if ((d[index]->canHandle != device) || ((msg.id & 0x7F) != getNodeId(d[index])))
            {
                continue;
            }
            LOG_D(TAG, "%s Node: %2.2x recv pack.", device, getNodeId(d[index]));
            CanOpen_EnterMutex();
            canDispatch(d[index], &co_msg);
            CanOpen_LeaveMutex();
            break;
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           open a can port
 *
 * @param[in]       can device name & baud
 *
 * @return          The operation result.
 * @retval          >0          If the operation successful.
 * @retval          else        Error .
 ***********************************************************************************************************************
 */
CAN_PORT canopen_portopen(s_BOARD *board)
{
    os_err_t     res;
    os_uint32_t  baud;
    os_task_t *  task;
    os_device_t *device;
    char         name[OS_NAME_MAX];
    device = os_device_find(board->busname);
    if (!device)
    {
        LOG_W(TAG, "canopen %s open error.", board->busname);
        OS_ASSERT(device);
        return NULL;
    }

#ifndef CAN_TEST_READ_NONBLOCK
    struct os_device_cb_info cb_info_rx = {
        .type = OS_DEVICE_CB_TYPE_RX,
        .cb   = can_rx_done,
    };
    os_device_control(device, OS_DEVICE_CTRL_SET_CB, &cb_info_rx);
#endif

#ifndef CAN_TEST_WRITE_NONBLOCK
    struct os_device_cb_info cb_info_tx = {
        .type = OS_DEVICE_CB_TYPE_TX,
        .cb   = can_tx_done,
    };
    os_device_control(device, OS_DEVICE_CTRL_SET_CB, &cb_info_tx);
#endif

    res = os_device_open(device);
    OS_ASSERT(res == OS_EOK);
    baud = atoi(board->baudrate);
    OS_ASSERT((baud >= 5000) && (baud <= 1000000));
    res = os_device_control(device, OS_CAN_CMD_SET_BAUD, (void *)baud);
    OS_ASSERT(res == OS_EOK);

    snprintf(name, sizeof(name) - 1, "cf_task_%s", device->name);
    task = os_task_create(name, canopen_recv_entry, device, 1024 * 1, 11);
    OS_ASSERT(task);
    os_task_startup(task);
    LOG_I(TAG, "canopen %s open sucessful.", device->name);
    return (CAN_PORT)device;
}

/**
 ***********************************************************************************************************************
 * @brief           init a canopen node
 *
 * @param[in]
 *
 * @return          The operation result.
 * @retval          =0          If the operation successful.
 * @retval          else        Error .
 ***********************************************************************************************************************
 */
int canopen_node_init(CAN_PORT const canport, CO_Data *const d, const UNS8 node_id)
{
    uint8_t      index;
    CO_Data *    ptr;
    os_device_t *device = (os_device_t *)canport;
    if (sg_nodenum >= sizeof(sg_nodelist) / sizeof(sg_nodelist[0]))
    {
        LOG_W(TAG, "%s Node: %2.2x init error.", device->name, node_id);
        OS_ASSERT(0);
    }

    for (index = 0; index < sg_nodenum; index++)
    {
        ptr = sg_nodelist[index];
        if ((getNodeId(ptr) == node_id) && (ptr->canHandle == canport))
        {
            LOG_W(TAG, "%s Node: %2.2x already init.", device->name, node_id);
            return -1;
        }
    }

    sg_nodelist[sg_nodenum++] = d;
    d->canHandle              = device;
    setNodeId(d, node_id);
    setState(d, Initialisation);
    LOG_I(TAG, "%s Node: %2.2x init sucessful", device->name, node_id);
    return 0;
}

/*
 * @[weak] callback function
 */
OS_WEAK void canfestival_heartbeatError(CO_Data *d, UNS8 heartbeatID)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x heartbeatError %d.", nodeId, heartbeatID);
}

OS_WEAK void canfestival_initialisation(CO_Data *d)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x enter initialisation state.", nodeId);
}

OS_WEAK void canfestival_preOperational(CO_Data *d)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x enter preOperational state.", nodeId);
}

OS_WEAK void canfestival_operational(CO_Data *d)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x enter operational state.", nodeId);
}

OS_WEAK void canfestival_stopped(CO_Data *d)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x enter stop state.", nodeId);
}

OS_WEAK void canfestival_post_sync(CO_Data *d)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x post sync.", nodeId);
}

OS_WEAK void canfestival_post_TPDO(CO_Data *d)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x post TPDO.", nodeId);
}

OS_WEAK void canfestival_storeODSubIndex(CO_Data *d, UNS16 wIndex, UNS8 bSubindex)
{
    UNS8 nodeId = getNodeId(d);
    LOG_I(TAG, "Node: %2.2x storeODSubIndex : %4.4x %2.2x", wIndex, bSubindex);
}

OS_WEAK void canfestival_post_emcy(CO_Data *d, UNS8 nodeId, UNS16 errCode, UNS8 errReg, const UNS8 errSpec[5])
{
    LOG_I(TAG, "Node: %2.2x received EMCY message.ErrorCode: %4.4x  ErrorRegister: %2.2x", nodeId, errCode, errReg);
}

/*
 * @endl callback function
 */
#endif

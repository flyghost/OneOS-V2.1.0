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
 * @file        can.h
 *
 * @brief       This file provides struct/enum definition and can functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef CAN_H_
#define CAN_H_

#include <os_task.h>
#include <device.h>
#include <os_mutex.h>
#include <os_sem.h>

#ifndef OS_CANMSG_BOX_SZ
#define OS_CANMSG_BOX_SZ 16
#endif
#ifndef OS_CANSND_BOX_NUM
#define OS_CANSND_BOX_NUM 1
#endif

/**
 ***********************************************************************************************************************
 * @enum        CANBAUD
 *
 * @brief       can baud rate
 ***********************************************************************************************************************
 */
enum CANBAUD
{
    CAN1MBaud   = 1000UL * 1000, /* 1 MBit/sec   */
    CAN800kBaud = 1000UL * 800,  /* 800 kBit/sec */
    CAN500kBaud = 1000UL * 500,  /* 500 kBit/sec */
    CAN250kBaud = 1000UL * 250,  /* 250 kBit/sec */
    CAN125kBaud = 1000UL * 125,  /* 125 kBit/sec */
    CAN100kBaud = 1000UL * 100,  /* 100 kBit/sec */
    CAN50kBaud  = 1000UL * 50,   /* 50 kBit/sec  */
    CAN20kBaud  = 1000UL * 20,   /* 20 kBit/sec  */
    CAN10kBaud  = 1000UL * 10    /* 10 kBit/sec  */
};

#define OS_CAN_MODE_NORMAL          0
#define OS_CAN_MODE_LISEN           1
#define OS_CAN_MODE_LOOPBACK        2
#define OS_CAN_MODE_LOOPBACKANLISEN 3

#define OS_CAN_MODE_PRIV   0x01
#define OS_CAN_MODE_NOPRIV 0x00

/**
 ***********************************************************************************************************************
 * @struct      os_can_filter_item
 *
 * @brief       os_can_filter_item content.
 ***********************************************************************************************************************
 */
struct os_can_filter_item
{
    os_uint32_t id   : 29;  /* can message id */
    os_uint32_t ide  : 1;   /* Extended frame identification bit */
    os_uint32_t rtr  : 1;   /* Remote frame identification bit */
    os_uint32_t mode : 1;   /* Filter table mode */
    os_uint32_t mask;       /* ID mask, 1 means the corresponding bits must matched */
    os_int32_t  hdr;        /* The corresponding filter table control block will be initialized */
#ifdef OS_CAN_USING_HDR
    /* Filter table callback function */
    os_err_t (*ind)(os_device_t dev, void *args, os_int32_t hdr, os_size_t size);
    void *args; /* Callback function parameters */
#endif          /*OS_CAN_USING_HDR*/
};

#ifdef OS_CAN_USING_HDR
/**
 ***********************************************************************************************************************
 * @def         OS_CAN_FILTER_ITEM_INIT
 *
 * @brief       corresponding to can filter item
 *
 * @param       id              os_can_filter_item.id
 * @param       ide             os_can_filter_item.ide
 * @param       rtr             os_can_filter_item.rtr
 * @param       mode            os_can_filter_item.mode
 * @param       mask            os_can_filter_item.mask
 * @param       hdr             os_can_filter_item.hdr
 * @param       ind             os_err_t (*ind)()
 * @param       args            void *args
 ***********************************************************************************************************************
 */
#define OS_CAN_FILTER_ITEM_INIT(id, ide, rtr, mode, mask, ind, args)                                                   \
    {                                                                                                                  \
        (id), (ide), (rtr), (mode), (mask), -1, (ind), (args)                                                          \
    }
#define OS_CAN_FILTER_STD_INIT(id, ind, args)          OS_CAN_FILTER_ITEM_INIT(id, 0, 0, 0, 0xFFFFFFFF, ind, args)
#define OS_CAN_FILTER_EXT_INIT(id, ind, args)          OS_CAN_FILTER_ITEM_INIT(id, 1, 0, 0, 0xFFFFFFFF, ind, args)
#define OS_CAN_STD_RMT_FILTER_INIT(id, ind, args)      OS_CAN_FILTER_ITEM_INIT(id, 0, 1, 0, 0xFFFFFFFF, ind, args)
#define OS_CAN_EXT_RMT_FILTER_INIT(id, ind, args)      OS_CAN_FILTER_ITEM_INIT(id, 1, 1, 0, 0xFFFFFFFF, ind, args)
#define OS_CAN_STD_RMT_DATA_FILTER_INIT(id, ind, args) OS_CAN_FILTER_ITEM_INIT(id, 0, 0, 1, 0xFFFFFFFF, ind, args)
#define OS_CAN_EXT_RMT_DATA_FILTER_INIT(id, ind, args) OS_CAN_FILTER_ITEM_INIT(id, 1, 0, 1, 0xFFFFFFFF, ind, args)
#else

#define OS_CAN_FILTER_ITEM_INIT(id, ide, rtr, mode, mask)                                                              \
    {                                                                                                                  \
        (id), (ide), (rtr), (mode), (mask), -1,                                                                        \
    }
#define OS_CAN_FILTER_STD_INIT(id)          OS_CAN_FILTER_ITEM_INIT(id, 0, 0, 0, 0xFFFFFFFF)
#define OS_CAN_FILTER_EXT_INIT(id)          OS_CAN_FILTER_ITEM_INIT(id, 1, 0, 0, 0xFFFFFFFF)
#define OS_CAN_STD_RMT_FILTER_INIT(id)      OS_CAN_FILTER_ITEM_INIT(id, 0, 1, 0, 0xFFFFFFFF)
#define OS_CAN_EXT_RMT_FILTER_INIT(id)      OS_CAN_FILTER_ITEM_INIT(id, 1, 1, 0, 0xFFFFFFFF)
#define OS_CAN_STD_RMT_DATA_FILTER_INIT(id) OS_CAN_FILTER_ITEM_INIT(id, 0, 0, 1, 0xFFFFFFFF)
#define OS_CAN_EXT_RMT_DATA_FILTER_INIT(id) OS_CAN_FILTER_ITEM_INIT(id, 1, 0, 1, 0xFFFFFFFF)
#endif

/**
 ***********************************************************************************************************************
 * @struct      os_can_filter_config
 *
 * @brief       structure of can filter
 ***********************************************************************************************************************
 */
struct os_can_filter_config
{
    os_uint32_t                count;
    os_uint32_t                actived;
    struct os_can_filter_item *items;
};

/**
 ***********************************************************************************************************************
 * @struct      can_configure
 *
 * @brief       structure of can configure parameter
 ***********************************************************************************************************************
 */
struct can_configure
{
    os_uint32_t baud_rate;
    os_uint32_t msgboxsz;
    os_uint32_t sndboxnumber;
    os_uint32_t mode : 8;
    os_uint32_t privmode : 8;
    os_uint32_t reserved : 16;
    os_uint32_t ticks;
#ifdef OS_CAN_USING_HDR
    os_uint32_t maxhdr;
#endif
};

#define CANDEFAULTCONFIG                                                                                               \
    {                                                                                                                  \
        CAN1MBaud,                                                                                                     \
        OS_CANMSG_BOX_SZ,                                                                                              \
        OS_CANSND_BOX_NUM,                                                                                             \
        OS_CAN_MODE_NORMAL,                                                                                            \
    };

struct os_can_ops;
#define OS_CAN_CMD_SET_FILTER     IOC_CAN(1)
#define OS_CAN_CMD_SET_BAUD       IOC_CAN(2)
#define OS_CAN_CMD_SET_MODE       IOC_CAN(3)
#define OS_CAN_CMD_SET_PRIV       IOC_CAN(4)
#define OS_CAN_CMD_GET_STATUS     IOC_CAN(5)
#define OS_CAN_CMD_SET_STATUS_IND IOC_CAN(6)
#define OS_CAN_CMD_SET_BUS_HOOK   IOC_CAN(7)

#define OS_DEVICE_CAN_INT_ERR 0x1000

enum OS_CAN_STATUS_MODE
{
    NORMAL     = 0,
    ERRWARNING = 1,
    ERRPASSIVE = 2,
    BUSOFF     = 4,
};
enum OS_CAN_BUS_ERR
{
    OS_CAN_BUS_NO_ERR           = 0,
    OS_CAN_BUS_BIT_PAD_ERR      = 1,
    OS_CAN_BUS_FORMAT_ERR       = 2,
    OS_CAN_BUS_ACK_ERR          = 3,
    OS_CAN_BUS_IMPLICIT_BIT_ERR = 4,
    OS_CAN_BUS_EXPLICIT_BIT_ERR = 5,
    OS_CAN_BUS_CRC_ERR          = 6,
};

struct os_can_status
{
    os_uint32_t rcverrcnt;
    os_uint32_t snderrcnt;
    os_uint32_t errcode;
    os_uint32_t rcvpkg;
    os_uint32_t dropedrcvpkg;
    os_uint32_t sndpkg;
    os_uint32_t dropedsndpkg;
    os_uint32_t bitpaderrcnt;
    os_uint32_t formaterrcnt;
    os_uint32_t ackerrcnt;
    os_uint32_t biterrcnt;
    os_uint32_t crcerrcnt;
    os_uint32_t rcvchange;
    os_uint32_t sndchange;
    os_uint32_t lasterrtype;
};

#ifdef OS_CAN_USING_HDR
struct os_can_hdr
{
    os_uint32_t               connected;
    os_uint32_t               msgs;
    struct os_can_filter_item filter;
    struct os_list_node       list;
};
#endif
struct os_can_device;
typedef os_err_t (*os_canstatus_ind)(struct os_can_device *, void *);
typedef struct os_can_status_ind_type
{
    os_canstatus_ind ind;
    void *           args;
} * os_can_status_ind_type_t;
typedef void (*os_can_bus_hook)(struct os_can_device *);

/**
 ***********************************************************************************************************************
 * @struct      os_can_device
 *
 * @brief       structure of can device
 ***********************************************************************************************************************
 */
struct os_can_device
{
    struct os_device parent;

    const struct os_can_ops *ops;
    struct can_configure     config;
    struct os_can_status     status;

    struct os_can_status_ind_type status_indicate;
#ifdef OS_CAN_USING_HDR
    struct os_can_hdr *hdr;
#endif
#ifdef OS_CAN_USING_BUS_HOOK
    os_can_bus_hook bus_hook;
#endif /*OS_CAN_USING_BUS_HOOK*/
    struct os_mutex lock;
    struct os_can_rx_fifo *can_rx;
    struct os_can_tx_fifo *can_tx;
};
typedef struct os_can_device *os_can_t;

#define OS_CAN_STDID 0
#define OS_CAN_EXTID 1
#define OS_CAN_DTR   0
#define OS_CAN_RTR   1

typedef struct os_can_status *os_can_status_t;

/**
 ***********************************************************************************************************************
 * @struct      os_can_msg
 *
 * @brief       structure of os_can_msg
 ***********************************************************************************************************************
 */
struct os_can_msg
{
    os_uint32_t id  : 29;     /* CAN ID */
    os_uint32_t ide : 1;      /* Extended frame identification bit */
    os_uint32_t rtr : 1;      /* Remote frame identification bit */
    os_uint32_t rsv : 1;      /* Reserved bit */
    os_uint32_t len : 8;      /* data length */
    os_uint32_t priv: 8;      /* Message sending priority */
    os_int32_t  hdr : 8;      /* Hardware filter table number */
    os_uint32_t reserved : 8; /* Reserved bit */
    os_uint8_t  data[8];      /* Data segment */
};
typedef struct os_can_msg *os_can_msg_t;

struct os_can_msg_list
{
    struct os_list_node list;
#ifdef OS_CAN_USING_HDR
    struct os_list_node hdrlist;
    struct os_can_hdr  *owner;
#endif
    struct os_can_msg data;
};

struct os_can_rx_fifo
{
    struct os_can_msg_list *buffer;
    os_uint32_t             freenumbers;
    struct os_list_node     freelist;
    struct os_list_node     datalist;
    int rx_available;
};

#define OS_CAN_SND_RESULT_OK   0
#define OS_CAN_SND_RESULT_ERR  1
#define OS_CAN_SND_RESULT_WAIT 2

#define OS_CAN_EVENT_RX_IND     0x01 /* Rx indication */
#define OS_CAN_EVENT_TX_DONE    0x02 /* Tx complete   */
#define OS_CAN_EVENT_TX_FAIL    0x03 /* Tx fail   */
#define OS_CAN_EVENT_RX_TIMEOUT 0x05 /* Rx timeout    */
#define OS_CAN_EVENT_RXOF_IND   0x06 /* Rx overflow */

struct os_can_sndbxinx_list
{
    struct os_list_node list;
    struct os_can_msg   data;
    os_uint32_t         result;
};

struct os_can_tx_fifo
{
    struct os_can_sndbxinx_list *buffer;
    struct os_list_node          freelist;
    struct os_list_node          datalist;
};

/**
 ***********************************************************************************************************************
 * @struct      os_can_ops
 *
 * @brief       can operation function set.
 ***********************************************************************************************************************
 */
struct os_can_ops
{
    os_err_t (*configure)(struct os_can_device *can, struct can_configure *cfg);
    os_err_t (*control)(struct os_can_device *can, int cmd, void *arg);
    
    int (*start_send)(struct os_can_device *can, const struct os_can_msg *msg);
    int (*stop_send)(struct os_can_device *can);
    
    int (*start_recv)(struct os_can_device *can, struct os_can_msg *msg);
    int (*stop_recv)(struct os_can_device *can);
    int (*recv_state)(struct os_can_device *can);    
};

os_err_t os_hw_can_register(struct os_can_device *can, const char *name, const struct os_can_ops *ops, void *data);
void os_hw_can_isr_rxdone(struct os_can_device *can);
void os_hw_can_isr_txdone(struct os_can_device *can, int event);

#endif /*_CAN_H*/

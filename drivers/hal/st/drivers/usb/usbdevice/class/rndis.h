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
 * @file        rndis.h
 *
 * @brief       This file provides rndis struct/Macro definition.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __RNDIS_H__
#define __RNDIS_H__

#include <os_task.h>

#define USB_ETH_MTU               1500 + 14
#define RNDIS_MESSAGE_BUFFER_SIZE 128

#define RESPONSE_AVAILABLE 0x00000001

/* Remote NDIS version numbers */
#define RNDIS_MAJOR_VERSION 1
#define RNDIS_MINOR_VERSION 0

/* Common status values */
#define RNDIS_STATUS_SUCCESS          0X00000000
#define RNDIS_STATUS_FAILURE          0XC0000001
#define RNDIS_STATUS_INVALID_DATA     0XC0010015
#define RNDIS_STATUS_NOT_SUPPORTED    0XC00000BB
#define RNDIS_STATUS_MEDIA_CONNECT    0X4001000B
#define RNDIS_STATUS_MEDIA_DISCONNECT 0X4001000C

/* Remote NDIS message types */
#define REMOTE_NDIS_PACKET_MSG          0x00000001
#define REMOTE_NDIS_INITIALIZE_MSG      0X00000002
#define REMOTE_NDIS_HALT_MSG            0X00000003
#define REMOTE_NDIS_QUERY_MSG           0X00000004
#define REMOTE_NDIS_SET_MSG             0X00000005
#define REMOTE_NDIS_RESET_MSG           0X00000006
#define REMOTE_NDIS_INDICATE_STATUS_MSG 0X00000007
#define REMOTE_NDIS_KEEPALIVE_MSG       0X00000008
#define REMOTE_NDIS_INITIALIZE_CMPLT    0X80000002
#define REMOTE_NDIS_QUERY_CMPLT         0X80000004
#define REMOTE_NDIS_SET_CMPLT           0X80000005
#define REMOTE_NDIS_RESET_CMPLT         0X80000006
#define REMOTE_NDIS_KEEPALIVE_CMPLT     0X80000008

/* Device flags */
#define RNDIS_DF_CONNECTIONLESS      0x00000001
#define RNDIS_DF_CONNECTION_ORIENTED 0x00000002
/* Mediums */
#define RNDIS_MEDIUM_802_3 0x00000000

struct ucls_rndis
{
    uep_t       notify;
    os_uint32_t filter;
    os_bool_t   header;
    os_uint8_t  rndis_state;
    os_uint8_t  media_state;
    os_uint8_t  ethaddr[6];
};

/**
 ***********************************************************************************************************************
 * @struct      rndis_gen_msg
 *
 * @brief       Remote NDIS generic message type
 ***********************************************************************************************************************
 */
struct rndis_gen_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
};
typedef struct rndis_gen_msg *rndis_gen_msg_t;

struct rndis_packet_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t DataOffset;
    os_uint32_t DataLength;
    os_uint32_t OOBDataOffset;
    os_uint32_t OOBDataLength;
    os_uint32_t NumOOBDataElements;
    os_uint32_t PerPacketInfoOffset;
    os_uint32_t PerPacketInfoLength;
    os_uint32_t VcHandle;
    os_uint32_t Reserved;
};
typedef struct rndis_packet_msg *rndis_packet_msg_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_init_msg
 *
 * @brief       Remote NDIS Initialize Message
 ***********************************************************************************************************************
 */
struct rndis_init_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
    os_uint32_t MajorVersion;
    os_uint32_t MinorVersion;
    os_uint32_t MaxTransferSize;
};
typedef struct rndis_init_msg *rndis_init_msg_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_init_cmplt
 *
 * @brief       Response
 ***********************************************************************************************************************
 */
struct rndis_init_cmplt
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
    os_uint32_t Status;
    os_uint32_t MajorVersion;
    os_uint32_t MinorVersion;
    os_uint32_t DeviceFlags;
    os_uint32_t Medium;
    os_uint32_t MaxPacketsPerTransfer;
    os_uint32_t MaxTransferSize;
    os_uint32_t PacketAlignmentFactor;
    os_uint32_t AfListOffset;
    os_uint32_t AfListSize;
};
typedef struct rndis_init_cmplt *rndis_init_cmplt_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_halt_msg
 *
 * @brief       Remote NDIS Halt Message
 ***********************************************************************************************************************
 */
struct rndis_halt_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
};

/**
 ***********************************************************************************************************************
 * @struct      rndis_query_msg
 *
 * @brief       Remote NDIS Query Message
 ***********************************************************************************************************************
 */
struct rndis_query_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
    os_uint32_t Oid;
    os_uint32_t InformationBufferLength;
    os_uint32_t InformationBufferOffset;
    os_uint32_t DeviceVcHandle;
};
typedef struct rndis_query_msg *rndis_query_msg_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_query_cmplt
 *
 * @brief       Response
 ***********************************************************************************************************************
 */
struct rndis_query_cmplt
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
    os_uint32_t Status;
    os_uint32_t InformationBufferLength;
    os_uint32_t InformationBufferOffset;
};
typedef struct rndis_query_cmplt *rndis_query_cmplt_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_set_msg
 *
 * @brief       Remote NDIS Set Message
 ***********************************************************************************************************************
 */
struct rndis_set_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
    os_uint32_t Oid;
    os_uint32_t InformationBufferLength;
    os_uint32_t InformationBufferOffset;
    os_uint32_t DeviceVcHandle;
};
typedef struct rndis_set_msg *rndis_set_msg_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_set_cmplt
 *
 * @brief       Response
 ***********************************************************************************************************************
 */
struct rndis_set_cmplt
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
    os_uint32_t Status;
};
typedef struct rndis_set_cmplt *rndis_set_cmplt_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_reset_msg
 *
 * @brief       Remote NDIS Soft Reset Message
 ***********************************************************************************************************************
 */
struct rndis_reset_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t Reserved;
};

/**
 ***********************************************************************************************************************
 * @struct      rndis_reset_cmplt
 *
 * @brief       Remote NDIS Soft Reset Response
 ***********************************************************************************************************************
 */
struct rndis_reset_cmplt
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t Status;
    os_uint32_t AddressingReset;
};

/**
 ***********************************************************************************************************************
 * @struct      rndis_indicate_status_msg
 *
 * @brief       Remote NDIS Indicate Status Message
 ***********************************************************************************************************************
 */
struct rndis_indicate_status_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t Status;
    os_uint32_t StatusBufferLength;
    os_uint32_t StatusBufferOffset;
};
typedef struct rndis_indicate_status_msg *rndis_indicate_status_msg_t;

struct rndis_keepalive_msg
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestID;
};
typedef struct rndis_keepalive_msg *rndis_keepalive_msg_t;

/**
 ***********************************************************************************************************************
 * @struct      rndis_keepalive_cmplt
 *
 * @brief       Response
 ***********************************************************************************************************************
 */
struct rndis_keepalive_cmplt
{
    os_uint32_t MessageType;
    os_uint32_t MessageLength;
    os_uint32_t RequestId;
    os_uint32_t Status;
};
typedef struct rndis_keepalive_cmplt *rndis_keepalive_cmplt_t;

#endif
